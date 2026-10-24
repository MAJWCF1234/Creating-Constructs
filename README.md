# Creating Constructs

Offline sandbox for pouring concrete volumes, planting nature, and running pipes — build monuments, then walk inside them.

## Play

- **Web:** open `Creating Constructs.html` in a browser (fully self-contained).
- **Native:** run `Play/CreatingConstructs.exe` (Vulkan). The main menu offers Play / continue, Saved worlds, Settings, Controls, and Save and quit. Rebuild with `build.ps1`.

## Controls

- Click / drag to place, Shift+click or X to erase, R to rotate
- Escape opens the main / pause menu; arrows + Enter navigate it. Gamepad Start opens/resumes, D-pad + A select, B goes back.
- Tab for walk mode · WASD · Space jump · H hide UI · M mute
- Service doors (Detail): F or left click while walking to open/close; gamepad X. Doors reopen if the player obstructs closing.

## Build

```powershell
.\build.ps1
```

Requires Visual Studio Build Tools (cl) and the Vulkan SDK (`glslc`).

## World API

[World generation and native runtime contract](docs/World-API.md): 104 parts, Share JSON, MON4/MNS4 saves, materials, viewpoints, animated doors, menu controls, and isolated QA launch flags.

[Native source layout](docs/Source-Layout.md) explains the consolidated modules, generated data, shader stages, and build workflow.
