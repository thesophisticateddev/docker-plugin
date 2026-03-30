/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "docker-pluginview.h"
#include "docker-plugin.h"

#include <KTextEditor/MainWindow>
#include <KLocalizedString>

#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QIcon>

DockerPluginView::DockerPluginView(DockerPlugin *plugin, KTextEditor::MainWindow *mainwindow)
    : QObject(mainwindow)
{
    setupToolView(plugin, mainwindow);

    m_client = new DockerClient(this);
    connect(m_client, &DockerClient::availabilityChanged,
            this, &DockerPluginView::onAvailabilityChanged);
    connect(m_client, &DockerClient::containersUpdated,
            this, &DockerPluginView::onContainersUpdated);

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

#include "moc_docker-pluginview.cpp"
