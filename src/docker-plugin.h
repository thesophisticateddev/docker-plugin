/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: Apache-2.0-or-later
*/

#ifndef DOCKER_PLUGIN_H
#define DOCKER_PLUGIN_H

// KF headers
#include <KTextEditor/Plugin>

class DockerPlugin : public KTextEditor::Plugin
{
    Q_OBJECT

public:
    /**
     * Default constructor, with arguments as expected by KPluginFactory
     */
    DockerPlugin(QObject* parent, const QVariantList& args);

    ~DockerPlugin() override;

public: // KTextEditor::Plugin API
    QObject* createView(KTextEditor::MainWindow* mainWindow) override;
};

#endif // DOCKER_PLUGIN_H
