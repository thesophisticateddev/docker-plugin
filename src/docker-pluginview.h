/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DOCKER_PLUGINVIEW_H
#define DOCKER_PLUGINVIEW_H

// Qt headers
#include <QObject>

namespace KTextEditor {
class MainWindow;
}

class docker-pluginPlugin;

class docker-pluginView: public QObject
{
    Q_OBJECT

public:
    docker-pluginView(docker-pluginPlugin* plugin, KTextEditor::MainWindow *view);
    ~docker-pluginView() override;
};

#endif // DOCKER_PLUGINVIEW_H
