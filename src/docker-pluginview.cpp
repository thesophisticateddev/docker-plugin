/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "docker-pluginview.h"

#include "docker-pluginplugin.h"

// KF headers
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/MainWindow>

#include <KLocalizedString>


docker-pluginView::docker-pluginView(docker-pluginPlugin* plugin, KTextEditor::MainWindow* mainwindow)
    : QObject(mainwindow)
{
    Q_UNUSED(plugin);
}

docker-pluginView::~docker-pluginView()
{
}

#include "moc_docker-pluginview.cpp"
