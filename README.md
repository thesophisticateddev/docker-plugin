# Docker Plugin for Kate

A KTextEditor plugin that lets you manage Docker containers directly from the Kate editor.

## Features

- View running and stopped Docker containers in a sidebar panel
- Start, stop, and remove containers via right-click context menu
- Inspect containers and open the full JSON output in the editor
- Automatic Docker engine availability detection with clear error messages

## Requirements

- **Kate** with KDE Frameworks 6 (KF6) support
- **Docker CLI** installed and available in `PATH`
- **Docker daemon** running

## Download

Pre-built releases are available on the [GitHub Releases](../../releases) page.

Each release includes:
- `docker-plugin-vX.Y.Z-linux-x86_64.tar.gz` — Linux binary
- `docker-plugin-vX.Y.Z-windows-x86_64.zip` — Windows binary (experimental)
- SHA256 checksums

## Installation

### Linux / Unix

1. Download and extract the release archive:

```bash
tar xzf docker-plugin-v*-linux-x86_64.tar.gz
```

2. Copy the plugin to a local directory:

```bash
export KATE_DOCKER_PLUGIN_DIR="$HOME/.local/share/kate-docker-plugin"
mkdir -p "$KATE_DOCKER_PLUGIN_DIR"
cp -r kf6/ "$KATE_DOCKER_PLUGIN_DIR/"
```

3. Launch Kate with the plugin path:

```bash
export QT_PLUGIN_PATH="$KATE_DOCKER_PLUGIN_DIR${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"
kate
```

4. Enable the plugin:
   **Settings > Configure Kate > Plugins** > enable **Docker Plugin**

> Do not set `QT_PLUGIN_PATH` globally in your shell profile. Use it per-session or in a launcher script.

### Windows (Experimental)

Windows builds may only work with the specific Kate distribution they were built against. ABI compatibility between the plugin DLL and your Kate installation is required.

**Option A: Session-local plugin path**

1. Extract the archive to a directory, e.g. `C:\Users\<user>\kate-docker-plugin\`
2. Launch Kate from PowerShell:

```powershell
$env:QT_PLUGIN_PATH="C:\Users\<user>\kate-docker-plugin;$env:QT_PLUGIN_PATH"
kate.exe
```

3. Enable the plugin in Kate's settings.

**Option B: Same-prefix install**

Copy `docker-plugin.dll` into your Kate installation's plugin directory:
```
<Kate install dir>\lib\qt6\plugins\kf6\ktexteditor\
```

## Troubleshooting

### Plugin does not appear in Kate

1. Verify the file exists in the correct path:
```bash
ls "$KATE_DOCKER_PLUGIN_DIR/kf6/ktexteditor/docker-plugin.so"
```

2. Enable plugin debug output:
```bash
QT_DEBUG_PLUGINS=1 kate 2>&1 | grep -i docker
```

3. Confirm `QT_PLUGIN_PATH` is set correctly for the Kate session.

### Plugin loads but shows "Docker not found"

Docker CLI is not in `PATH`. Verify:
```bash
docker --version
```

### Plugin shows "Docker daemon is not running"

Start the Docker daemon:
```bash
sudo systemctl start docker
```

### Plugin shows "Permission denied"

Your user is not in the `docker` group:
```bash
sudo usermod -aG docker $USER
```
Log out and back in for the change to take effect.

### Docker commands work in terminal but not in Kate

Kate may have been launched with a different `PATH` than your terminal. Launch Kate from a terminal where `docker` is available.

### Windows: Plugin does not load

The most common cause is an ABI mismatch. The plugin must be built with the same Qt6/KF6 versions as your Kate installation.

## Building from Source

### Dependencies

- CMake 3.16+
- Extra CMake Modules (ECM) 6.0.0+
- Qt 6 (Widgets)
- KDE Frameworks 6: KF6::TextEditor, KF6::I18n

### Build

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

### Install to a custom prefix

```bash
cmake --install . --prefix /path/to/install
```

### Test locally

```bash
export QT_PLUGIN_PATH=/path/to/install:$QT_PLUGIN_PATH
kate --startanon
```

## Release Process

1. Update the version in `CMakeLists.txt`
2. Push a tag: `git tag v0.1.0 && git push --tags`
3. GitHub Actions builds platform artifacts automatically
4. Release page gets downloadable files with checksums

## License

Apache-2.0-or-later. See [LICENSES/](LICENSES/) for details.
