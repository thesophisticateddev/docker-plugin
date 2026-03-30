/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "docker-pluginplugin.h"

#include "docker-pluginview.h"

// KF headers
#include <KTextEditor/MainWindow>

#include <KPluginFactory>
#include <KLocalizedString>

K_PLUGIN_CLASS_WITH_JSON(docker-pluginPlugin, "docker-plugin.json")


docker-pluginPlugin::docker-pluginPlugin(QObject* parent, const QVariantList& /*args*/)
    : KTextEditor::Plugin(parent)
{
}

docker-pluginPlugin::~docker-pluginPlugin()
{
}

QObject* docker-pluginPlugin::createView(KTextEditor::MainWindow* mainwindow)
{
    return new docker-pluginView(this, mainwindow);
}


// needed for K_PLUGIN_CLASS_WITH_JSON
#include <docker-pluginplugin.moc>

#include "moc_docker-pluginplugin.cpp"
