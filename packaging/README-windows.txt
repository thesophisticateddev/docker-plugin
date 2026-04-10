Docker Plugin for Kate
======================

A KTextEditor plugin for managing Docker containers from the Kate editor.

IMPORTANT: This Windows build is EXPERIMENTAL. The plugin DLL must be
ABI-compatible with your Kate installation's Qt6/KF6 build. It has been
built against a specific toolchain and may not work with all Kate distributions.

Requirements
------------
- Kate with KDE Frameworks 6 (KF6) support
- Docker CLI installed and available in PATH
- Docker Desktop running (or Docker daemon accessible)

Installation
------------
1. Extract the archive to a directory, for example:

   C:\Users\<user>\kate-docker-plugin\

2. Verify the archive contains:

   kf6\ktexteditor\docker-plugin.dll

3. Launch Kate from PowerShell with the plugin path:

   $env:QT_PLUGIN_PATH="C:\Users\<user>\kate-docker-plugin;$env:QT_PLUGIN_PATH"
   kate.exe

4. Enable the plugin:
   Settings > Configure Kate > Plugins > enable "Docker Plugin"

Alternative: Same-prefix install
---------------------------------
If you know where Kate keeps its plugins, copy docker-plugin.dll directly into:

   <Kate install dir>\lib\qt6\plugins\kf6\ktexteditor\

Troubleshooting
---------------
If the plugin does not load, the most likely cause is an ABI mismatch between
the plugin DLL and your Kate installation. The plugin must be built with the
same Qt6/KF6 versions as your Kate.

Verify Docker is working:

   docker info
   docker ps

License: Apache-2.0-or-later
