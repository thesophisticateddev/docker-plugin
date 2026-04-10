Docker Plugin for Kate
======================

A KTextEditor plugin for managing Docker containers from the Kate editor.

Requirements
------------
- Kate with KDE Frameworks 6 (KF6) support
- Docker CLI installed and available in PATH
- Docker daemon running

Installation
------------
1. Extract the archive:

   tar xzf docker-plugin-*-linux-*.tar.gz

2. Choose an install location (your home directory is recommended):

   export KATE_DOCKER_PLUGIN_DIR="$HOME/.local/share/kate-docker-plugin"
   mkdir -p "$KATE_DOCKER_PLUGIN_DIR"
   cp -r kf6/ "$KATE_DOCKER_PLUGIN_DIR/"

3. Launch Kate with the plugin path:

   export QT_PLUGIN_PATH="$KATE_DOCKER_PLUGIN_DIR${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
   kate

4. Enable the plugin:
   Settings > Configure Kate > Plugins > enable "Docker Plugin"

Troubleshooting
---------------
If the plugin does not appear in Kate, check plugin loading with:

   QT_DEBUG_PLUGINS=1 kate 2>&1 | grep -i docker

If Docker errors appear in the plugin panel, verify Docker is working:

   docker info
   docker ps

License: Apache-2.0-or-later
