/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: Apache-2.0-or-later
*/

#include "docker-pluginview.h"
#include "docker-plugin.h"

#include <KTextEditor/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLoggingCategory>
#include <QTemporaryFile>

Q_LOGGING_CATEGORY(DOCKER_PLUGIN, "docker-plugin")

static const int ContainerIdRole = Qt::UserRole;

DockerPluginView::DockerPluginView(DockerPlugin *plugin, KTextEditor::MainWindow *mainwindow)
    : QObject(mainwindow)
    , m_mainWindow(mainwindow)
{
    setupToolView(plugin, mainwindow);

    m_client = new DockerClient(this);
    connect(m_client, &DockerClient::availabilityChanged,
            this, &DockerPluginView::onAvailabilityChanged);
    connect(m_client, &DockerClient::containersUpdated,
            this, &DockerPluginView::onContainersUpdated);
    connect(m_client, &DockerClient::commandFinished,
            this, &DockerPluginView::onCommandFinished);
    connect(m_client, &DockerClient::inspectResult,
            this, &DockerPluginView::onInspectResult);

    setStatusText(i18n("Checking Docker..."));
    m_client->checkAvailability();
}

DockerPluginView::~DockerPluginView()
{
    delete m_toolView;
}

void DockerPluginView::setupToolView(DockerPlugin *plugin, KTextEditor::MainWindow *mainwindow)
{
    m_toolView = mainwindow->createToolView(
        plugin,
        QStringLiteral("docker_plugin_toolview"),
        KTextEditor::MainWindow::Left,
        QIcon::fromTheme(QStringLiteral("run-build")),
        i18n("Docker"));

    auto *container = new QWidget(m_toolView);
    container->setMinimumWidth(250);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(4, 4, 4, 4);

    m_statusLabel = new QLabel(container);
    m_statusLabel->setWordWrap(true);
    layout->addWidget(m_statusLabel);

    auto *btnLayout = new QHBoxLayout();
    m_refreshBtn = new QPushButton(i18n("Refresh"), container);
    m_refreshBtn->setEnabled(false);
    connect(m_refreshBtn, &QPushButton::clicked, this, &DockerPluginView::onRefreshClicked);
    btnLayout->addWidget(m_refreshBtn);

    m_showAllBtn = new QToolButton(container);
    m_showAllBtn->setText(i18n("Show All"));
    m_showAllBtn->setCheckable(true);
    m_showAllBtn->setChecked(false);
    m_showAllBtn->setEnabled(false);
    connect(m_showAllBtn, &QToolButton::toggled, this, &DockerPluginView::onShowAllToggled);
    btnLayout->addWidget(m_showAllBtn);

    layout->addLayout(btnLayout);

    m_containerTree = new QTreeWidget(container);
    m_containerTree->setHeaderLabels({i18n("Name"), i18n("Image"), i18n("Status"), i18n("Ports")});
    m_containerTree->setRootIsDecorated(false);
    m_containerTree->header()->setStretchLastSection(true);
    m_containerTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_containerTree, &QTreeWidget::customContextMenuRequested,
            this, &DockerPluginView::onContextMenu);
    layout->addWidget(m_containerTree);
}

void DockerPluginView::setStatusText(const QString &text, bool isError)
{
    m_statusLabel->setText(text);
    if (isError) {
        m_statusLabel->setStyleSheet(QStringLiteral("QLabel { color: red; }"));
    } else {
        m_statusLabel->setStyleSheet(QString());
    }
}

void DockerPluginView::onRefreshClicked()
{
    m_refreshBtn->setEnabled(false);
    m_containerTree->clear();
    setStatusText(i18n("Checking Docker..."));
    m_client->checkAvailability();
}

void DockerPluginView::onShowAllToggled(bool checked)
{
    m_showAll = checked;
    m_refreshBtn->setEnabled(false);
    m_showAllBtn->setEnabled(false);
    m_containerTree->clear();
    setStatusText(i18n("Fetching containers..."));
    m_client->listContainers(m_showAll);
}

void DockerPluginView::onAvailabilityChanged(DockerAvailability availability,
                                              const QString &errorMessage)
{
    switch (availability) {
    case DockerAvailability::Available:
        setStatusText(i18n("Docker available. Fetching containers..."));
        m_refreshBtn->setEnabled(true);
        m_showAllBtn->setEnabled(true);
        m_client->listContainers(m_showAll);
        return;
    case DockerAvailability::NotFound:
        setStatusText(i18n("Docker not found. Install Docker and ensure it is in PATH."), true);
        m_refreshBtn->setEnabled(false);
        m_showAllBtn->setEnabled(false);
        break;
    case DockerAvailability::DaemonDown:
        setStatusText(i18n("Docker daemon is not running. Start it and click Refresh."), true);
        m_refreshBtn->setEnabled(true);
        m_showAllBtn->setEnabled(false);
        break;
    case DockerAvailability::PermissionDenied:
        setStatusText(i18n("Permission denied. Add your user to the 'docker' group."), true);
        m_refreshBtn->setEnabled(false);
        m_showAllBtn->setEnabled(false);
        break;
    default:
        setStatusText(i18n("Docker error: %1", errorMessage), true);
        m_refreshBtn->setEnabled(true);
        m_showAllBtn->setEnabled(false);
        break;
    }
    m_containerTree->clear();
}

void DockerPluginView::onContainersUpdated(const QList<ContainerInfo> &containers)
{
    m_containerTree->clear();

    if (containers.isEmpty()) {
        setStatusText(m_showAll ? i18n("Docker available. No containers.")
                                : i18n("Docker available. No running containers."));
        m_refreshBtn->setEnabled(true);
        m_showAllBtn->setEnabled(true);
        return;
    }

    setStatusText(i18np("Docker available. %1 container.",
                        "Docker available. %1 containers.",
                        containers.size()));

    for (const ContainerInfo &c : containers) {
        auto *item = new QTreeWidgetItem(m_containerTree);
        item->setData(0, ContainerIdRole, c.id);
        item->setText(0, c.name);
        item->setText(1, c.image);
        item->setText(2, c.status);
        item->setText(3, c.ports);
    }

    m_containerTree->resizeColumnToContents(0);
    m_containerTree->resizeColumnToContents(1);
    m_refreshBtn->setEnabled(true);
    m_showAllBtn->setEnabled(true);
}

void DockerPluginView::onContextMenu(const QPoint &pos)
{
    auto *item = m_containerTree->itemAt(pos);
    if (!item) {
        return;
    }

    const QString containerId = containerIdFromItem(item);
    const QString containerName = item->text(0);

    QMenu menu;
    auto *startAction = menu.addAction(QIcon::fromTheme(QStringLiteral("media-playback-start")),
                                       i18n("Start"));
    auto *stopAction = menu.addAction(QIcon::fromTheme(QStringLiteral("media-playback-stop")),
                                      i18n("Stop"));
    menu.addSeparator();
    auto *inspectAction = menu.addAction(i18n("Inspect"));
    auto *inspectJsonAction = menu.addAction(i18n("Inspect (Open as JSON)"));
    menu.addSeparator();
    auto *removeAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-delete")),
                                        i18n("Remove"));

    auto *chosen = menu.exec(m_containerTree->viewport()->mapToGlobal(pos));
    if (!chosen) {
        return;
    }

    if (chosen == startAction) {
        setStatusText(i18n("Starting %1...", containerName));
        m_client->startContainer(containerId);
    } else if (chosen == stopAction) {
        const auto result = QMessageBox::question(
            m_toolView, i18n("Stop Container"),
            i18n("Stop container '%1'?", containerName));
        if (result == QMessageBox::Yes) {
            setStatusText(i18n("Stopping %1...", containerName));
            m_client->stopContainer(containerId);
        }
    } else if (chosen == inspectAction) {
        m_inspectOpenAsJson = false;
        setStatusText(i18n("Inspecting %1...", containerName));
        m_client->inspectContainer(containerId);
    } else if (chosen == inspectJsonAction) {
        m_inspectOpenAsJson = true;
        setStatusText(i18n("Inspecting %1...", containerName));
        m_client->inspectContainer(containerId);
    } else if (chosen == removeAction) {
        const auto result = QMessageBox::warning(
            m_toolView, i18n("Remove Container"),
            i18n("Remove container '%1'? This cannot be undone.", containerName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (result == QMessageBox::Yes) {
            setStatusText(i18n("Removing %1...", containerName));
            m_client->removeContainer(containerId);
        }
    }
}

void DockerPluginView::onCommandFinished(const QString &action, const QString &containerId,
                                          bool success, const QString &output)
{
    Q_UNUSED(containerId)
    if (success) {
        setStatusText(i18n("'docker %1' succeeded.", action));
        m_client->listContainers(m_showAll);
    } else {
        setStatusText(i18n("'docker %1' failed: %2", action, output), true);
    }
}

void DockerPluginView::onInspectResult(const QString &containerId, const QString &jsonText)
{
    if (!m_inspectOpenAsJson) {
        setStatusText(i18n("Inspect complete."));
        QMessageBox::information(m_toolView, i18n("Container Inspect"), jsonText);
        return;
    }

    // Write to a temp file and open via Kate's openUrl().
    // QTemporaryFile with setAutoRemove(false) so Kate can read it;
    // Kate will prompt "Save As" on close since it's a real file the user can manage.
    auto *tmpFile = new QTemporaryFile(
        QDir::tempPath() + QStringLiteral("/docker-inspect-%1-XXXXXX.json").arg(containerId.left(12)),
        this);
    tmpFile->setAutoRemove(false);

    if (!tmpFile->open()) {
        qCWarning(DOCKER_PLUGIN) << "Failed to create temporary file for inspect result:"
                                  << tmpFile->errorString();
        setStatusText(i18n("Failed to create temporary file: %1", tmpFile->errorString()), true);
        delete tmpFile;
        return;
    }

    const QByteArray data = jsonText.toUtf8();
    if (tmpFile->write(data) != data.size()) {
        qCWarning(DOCKER_PLUGIN) << "Failed to write inspect data to temporary file:"
                                  << tmpFile->errorString();
        setStatusText(i18n("Failed to write inspect data: %1", tmpFile->errorString()), true);
        tmpFile->remove();
        delete tmpFile;
        return;
    }

    const QString filePath = tmpFile->fileName();
    tmpFile->close();
    delete tmpFile;

    qCDebug(DOCKER_PLUGIN) << "Opening inspect JSON from temp file:" << filePath;

    if (!m_mainWindow) {
        qCWarning(DOCKER_PLUGIN) << "MainWindow is null, cannot open inspect result";
        setStatusText(i18n("Cannot open file: editor window unavailable."), true);
        QFile::remove(filePath);
        return;
    }

    auto *view = m_mainWindow->openUrl(QUrl::fromLocalFile(filePath));
    if (!view) {
        qCWarning(DOCKER_PLUGIN) << "openUrl returned null for:" << filePath;
        setStatusText(i18n("Failed to open inspect result in editor."), true);
        QFile::remove(filePath);
        return;
    }

    // Clean up the temp file now that Kate has loaded it into memory.
    // The document becomes untitled/modified so Kate will prompt Save As on close.
    auto *doc = view->document();
    if (doc) {
        doc->setHighlightingMode(QStringLiteral("JSON"));
    }
    QFile::remove(filePath);

    setStatusText(i18n("Inspect result opened in editor. Use Save As to keep it."));
}

QString DockerPluginView::containerIdFromItem(QTreeWidgetItem *item) const
{
    return item->data(0, ContainerIdRole).toString();
}

#include "moc_docker-pluginview.cpp"
