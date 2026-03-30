/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DOCKER_PLUGINPLUGIN_H
#define DOCKER_PLUGINPLUGIN_H

// KF headers
#include <KTextEditor/Plugin>

class docker-pluginPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    /**
     * Default constructor, with arguments as expected by KPluginFactory
     */
    docker-pluginPlugin(QObject* parent, const QVariantList& args);

    ~docker-pluginPlugin() override;

public: // KTextEditor::Plugin API
    QObject* createView(KTextEditor::MainWindow* mainWindow) override;
};

#endif // DOCKER_PLUGINPLUGIN_H
