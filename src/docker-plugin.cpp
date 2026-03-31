/*
    SPDX-FileCopyrightText: 2026 Salman Ahmed <salman.sc829@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "docker-plugin.h"

#include "docker-pluginview.h"

// KF headers
#include <KTextEditor/MainWindow>

#include <KPluginFactory>
#include <KLocalizedString>

K_PLUGIN_CLASS_WITH_JSON(DockerPlugin, "docker-plugin.json")


DockerPlugin::DockerPlugin(QObject* parent, const QVariantList& /*args*/)
    : KTextEditor::Plugin(parent)
{
}

DockerPlugin::~DockerPlugin()
{
}

QObject* DockerPlugin::createView(KTextEditor::MainWindow* mainwindow)
{
    return new DockerPluginView(this, mainwindow);
}


// needed for K_PLUGIN_CLASS_WITH_JSON
#include <docker-plugin.moc>

#include "moc_docker-plugin.cpp"
