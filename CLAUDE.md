# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

KTextEditor/Kate plugin for Docker integration. Built as a KDE Framework 6 plugin using C++/Qt. Licensed under LGPL-2.0-or-later.

## Build Commands

```bash
# Set custom install path (optional, for testing)
export MYKATEPLUGINPATH=$HOME/mykateplugins

# Configure and build
mkdir -p build && cd build
cmake .. -DKDE_INSTALL_PLUGINDIR=$MYKATEPLUGINPATH
make

# Install
make install
```

## Testing the Plugin

```bash
export QT_PLUGIN_PATH=$MYKATEPLUGINPATH:$QT_PLUGIN_PATH
kate --startanon
# Then enable in: Settings > Configure Kate > Application > Plugins
```

## Dependencies

- CMake 3.16+
- KDE Frameworks 6 (KF6 >= 6.0.0): `KF6::TextEditor`, `KF6::I18n`
- Extra CMake Modules (ECM)

## Architecture

- **`DockerPlugin`** (`docker-plugin.cpp/h`): Main plugin class, subclasses `KTextEditor::Plugin`. Registered via `K_PLUGIN_CLASS_WITH_JSON` macro with metadata from `docker-plugin.json`.
- **`DockerPluginView`** (`docker-pluginview.cpp/h`): Per-window view created by `DockerPlugin::createView()`. This is where UI and per-window logic goes.
- **`docker-plugin.json`**: KPlugin metadata (name, author, service type).
- **`Messages.sh`**: i18n string extraction script for KDE translation system.

## Notes

- The codebase was scaffolded from a KDE template and still has template placeholders (`%{CURRENT_YEAR}`, `%{AUTHOR}`, `%{EMAIL}`) in license headers that need replacing.
- Some files still reference old names (`docker-pluginplugin.h`, `docker-pluginplugin.moc`) from a rename — these includes need updating to match the current filenames (`docker-plugin.h`, `docker-plugin.cpp`).
- The plugin installs to `${KDE_INSTALL_PLUGINDIR}/kf6/ktexteditor`.


## Project Overview

This repository contains a **Kate plugin** for managing **Docker on the host machine**.

Current technology stack:

- **C++**
- **Qt 6**
- **KF6 / KDE Frameworks 6**
- **CMake**
- **Kate plugin architecture**

The plugin depends on the **Docker CLI** being installed and available on the host system.

The project is being developed with assistance from **Claude Code**. This file defines the expectations, constraints, and coding approach Claude Code should follow when making changes.

---

## Repository Layout

Current repository structure:

- `CMakeLists.txt` — top-level CMake configuration
- `src/` — plugin source code
- `README.md` — project documentation
- `CLAUDE.md` — Claude-specific project notes, if any
- `LICENSES/` — licensing files
- `build/` — generated build artifacts, not source of truth

### Source layout

The main implementation currently lives in `src/`:

- `docker-plugin.cpp`
- `docker-plugin.h`
- `docker-pluginview.cpp`
- `docker-pluginview.h`
- `docker-plugin.json`
- `Messages.sh`
- `src/CMakeLists.txt`

### Important guidance about `build/`
The `build/` directory contains generated artifacts and local build output. Agents must treat it as **derived output**, not primary source.

Do not make intentional source edits inside:

- `build/`
- `build/CMakeFiles/`
- generated autogen directories
- generated Ninja/CMake cache files

If a change is needed, modify the real source under the repository root or `src/`, then let the build system regenerate artifacts.

---

## Current Architecture Expectations

At the moment, the codebase appears to be centered around:

- a plugin class in:
  - `src/docker-plugin.h`
  - `src/docker-plugin.cpp`

- a plugin view / UI class in:
  - `src/docker-pluginview.h`
  - `src/docker-pluginview.cpp`

Claude Code should preserve and extend this structure unless there is a strong reason to refactor.

### Current likely responsibility split

#### Plugin class
The plugin class should remain responsible for:

- Kate plugin integration
- plugin lifecycle wiring
- creation/registration of views and actions
- high-level integration with Kate/KTextEditor APIs

#### Plugin view class
The plugin view class should remain responsible for:

- UI widgets and toolview behavior
- user-triggered actions
- displaying Docker-related state/results
- delegating actual Docker execution logic elsewhere when complexity grows

### When adding more functionality
Do not overload `docker-pluginview.cpp` with too much process and parsing logic. As Docker functionality grows, prefer extracting focused classes under `src/`, for example:

- `dockerclient.*`
- `dockerprocessrunner.*`
- `dockerparser.*`
- `dockermodels.*`

These are examples, not mandatory names.

---

## Primary Goals

When working in this repository, optimize for:

1. **Reliability**
   - The plugin must behave predictably when Docker is installed, missing, misconfigured, or inaccessible.

2. **Safety**
   - The plugin manages Docker on the host machine.
   - Avoid surprising or destructive actions.

3. **Good KDE / Qt integration**
   - Follow Kate, Qt, and KDE conventions.
   - Keep the UI responsive.

4. **Maintainability**
   - Keep responsibilities separated.
   - Write clear, reviewable code.

5. **Incremental evolution**
   - This codebase is still small.
   - Prefer minimal, coherent extensions over premature architecture expansion.

---

## Build System Rules

### CMake is the source of truth
Use:

- top-level `CMakeLists.txt`
- `src/CMakeLists.txt`

for build configuration changes.

Do not edit generated files such as:

- `build.ninja`
- `CMakeCache.txt`
- `cmake_install.cmake`
- `compile_commands.json`
- autogen files under `build/`

### Dependency changes
Do not introduce new dependencies unless clearly necessary and justified.

Prefer using existing:

- Qt6 APIs
- KF6 APIs
- Kate/KTextEditor APIs

---

## Kate Plugin Rules

This is a Kate plugin, so all changes should respect Kate/KTextEditor plugin conventions.

### Preserve plugin metadata
Be careful when editing:

- `src/docker-plugin.json`

Do not remove or casually rename plugin metadata keys unless required.

### Respect plugin lifecycle
Ensure plugin/view initialization and teardown remain clean and predictable.

### Integrate with Kate idioms
Prefer established Kate/KDE patterns instead of inventing custom lifecycle handling.

---

## Docker Integration Rules

### Docker CLI is required
This plugin currently depends on the host system having the `docker` executable available.

Claude Code should assume Docker is invoked through the CLI, not through a direct Engine API client.

### Always handle missing Docker
The plugin must gracefully handle:

- `docker` not installed
- `docker` not in `PATH`
- Docker daemon unavailable
- insufficient user permissions
- non-zero exit codes
- malformed or unexpected output

### Prefer structured output
When possible, use Docker commands with stable machine-readable output, such as:

- `docker ps --format ...`
- `docker images --format ...`
- `docker inspect`

Avoid fragile parsing of human-formatted tables.

### Do not use shell wrapping unnecessarily
Prefer `QProcess` with explicit program + arguments.

Avoid:

- `sh -c`
- string-concatenated shell commands
- any pattern that risks quoting bugs or command injection

---

## UI and Responsiveness Rules

### Never block the UI thread
Docker operations may be slow. Do not freeze Kate while waiting for commands to finish.

Use async process handling where appropriate.

### Show clear states
The UI should clearly communicate:

- Docker available / unavailable
- command running
- empty result sets
- command failure
- permission or daemon issues

### Surface errors helpfully
When an action fails, show useful information without overwhelming the user.

Good error reporting includes:

- what action failed
- a concise explanation
- relevant stderr when helpful

### Confirm destructive actions
For actions like:

- stop container
- restart container
- remove container
- prune resources
- remove image

prefer explicit confirmation or another clear user-intent mechanism.

---

## C++ / Qt / KF6 Coding Standards

### General
- Follow existing project style first.
- Keep classes focused.
- Prefer readability over cleverness.
- Make small, reviewable changes.

### C++
- Use modern C++ supported by the project toolchain.
- Prefer RAII.
- Be clear about ownership.
- Avoid unnecessary raw owning pointers.

### Qt
- Use signals/slots and Qt idioms consistently.
- Use `QProcess` for Docker execution.
- Keep UI logic and process logic separated as the project grows.

### KF6 / KDE
- Use KDE/KF APIs where they fit naturally.
- Maintain consistency with Kate plugin development patterns.

---

## File-Specific Guidance

### `src/docker-plugin.cpp` / `src/docker-plugin.h`
Use for:

- plugin integration
- setup/registration
- creation of plugin views/actions
- lightweight coordination

Do not let these files become a dumping ground for Docker business logic.

### `src/docker-pluginview.cpp` / `src/docker-pluginview.h`
Use for:

- toolview UI
- event wiring
- presenting results to the user
- invoking Docker-related services

Keep heavy parsing and process handling out of the UI class once complexity increases.

### `src/docker-plugin.json`
Use for plugin metadata only. Edit carefully.

### `src/Messages.sh`
Respect KDE translation/localization workflows when user-visible strings are added or changed.

### `README.md`
Update when:

- new features are added
- setup requirements change
- Docker CLI assumptions change
- user-visible behavior changes

### `CLAUDE.md`
If this file contains repo-specific instructions for Claude Code, follow both files together:

1. repository conventions and actual code
2. `CLAUDE.md`
3. `AGENTS.md`

If they conflict, prefer the more repository-specific instruction unless it is clearly outdated.

---

## Security Expectations

Because this plugin controls Docker on the host system:

- never interpolate unchecked user input into shell commands
- always pass process arguments explicitly
- avoid hidden destructive behavior
- do not add privilege escalation automatically
- do not introduce `sudo` or pkexec flows unless explicitly requested and designed

Host-impacting actions must remain deliberate and visible.

---

## Testing Expectations

As the codebase grows, prioritize tests for:

- Docker command construction
- output parsing
- error handling
- missing-Docker behavior

If there is not yet a dedicated test structure, do not invent an oversized one immediately. Prefer extracting logic into testable units first.

If automated tests are not added for a change, explain why in the change summary.

---

## Refactoring Guidance

This codebase is still small. Refactoring should be conservative.

### Good refactors
- extracting Docker process handling into a small helper/service
- extracting parsing logic from UI classes
- reducing duplicated code
- improving naming and ownership clarity

### Avoid unless necessary
- large directory reorganizations
- broad framework rewrites
- replacing established Kate plugin patterns
- introducing abstraction layers with no immediate benefit

---

## Preferred Change Style for Claude Code

Claude Code should:

1. Read the current code first and extend existing patterns.
2. Keep edits localized when possible.
3. Avoid changing generated files.
4. Prefer source edits in `src/` and CMake files only.
5. Preserve plugin metadata and lifecycle correctness.
6. Keep Docker execution safe and explicit.
7. Make failure cases visible and user-friendly.
8. Avoid blocking Kate’s UI thread.
9. Explain meaningful design choices briefly in summaries.
10. Leave the repository in a buildable, coherent state.

---

## Non-Goals Unless Explicitly Requested

Do not assume support for:

- Podman
- Docker Engine API client libraries
- Kubernetes
- Docker Swarm
- remote Docker hosts
- privilege escalation workflows
- advanced Compose management

Stick to the current project scope unless asked otherwise.
