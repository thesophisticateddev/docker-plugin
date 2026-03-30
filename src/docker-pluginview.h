/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DOCKER_PLUGINVIEW_H
#define DOCKER_PLUGINVIEW_H

#include "dockerclient.h"

#include <QObject>

class QWidget;
class QLabel;
class QPushButton;
class QToolButton;
class QTreeWidget;

namespace KTextEditor {
class MainWindow;
}

class DockerPlugin;

class DockerPluginView : public QObject
{
    Q_OBJECT

public:
    DockerPluginView(DockerPlugin *plugin, KTextEditor::MainWindow *mainwindow);
    ~DockerPluginView() override;

private Q_SLOTS:
    void onRefreshClicked();
    void onShowAllToggled(bool checked);
    void onAvailabilityChanged(DockerAvailability availability, const QString &errorMessage);
    void onContainersUpdated(const QList<ContainerInfo> &containers);

private:
    void setupToolView(DockerPlugin *plugin, KTextEditor::MainWindow *mainwindow);
    void setStatusText(const QString &text, bool isError = false);

    QWidget *m_toolView = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_refreshBtn = nullptr;
    QToolButton *m_showAllBtn = nullptr;
    QTreeWidget *m_containerTree = nullptr;
    DockerClient *m_client = nullptr;
    bool m_showAll = false;
};

#endif // DOCKER_PLUGINVIEW_H
