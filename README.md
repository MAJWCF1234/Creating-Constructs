# Creating Constructs

Offline sandbox for pouring concrete volumes, planting nature, and running pipes — build monuments, then walk inside them.

## Play

- **Web:** open `Creating Constructs.html` in a browser (fully self-contained).
- **Native:** run `Play/CreatingConstructs.exe` (Vulkan). The main menu offers Play / continue, Saved worlds, Settings, Controls, and Save and quit. Rebuild with `build.ps1`.

## Controls

- Click / drag to place, Shift+click or X to erase, R to rotate
- Escape opens the main / pause menu; arrows + Enter navigate it. Gamepad Start opens/resumes, D-pad + A select, B goes back.
- Build tools (native): B cycles Brush / Box / Select — two clicks fill, erase or copy a volume; P paints finishes onto existing pieces; Ctrl+V stamps the copied region (R turns it); N mirrors placements across X, Z or both; Page Up / Down raise a build plane, Home clears it
- Parts: 124 pieces in nine tabs, including the Cyber tab (neon violet, holo sign, server rack, vending, grate floor, barrier, duct, fan vent, antenna, light strip, grid floor) and the Liminal tab (carpet, yellow wall, ceiling tile, tube light, pool tile, door blank, partition, water cooler, painted pillar). Native and web builds share the catalog.
- Sharing (native): Export part / Import part exchange selected regions as `.part.json` files that stamp into any world; the Share dialog saves and opens whole worlds as files. OBJ / FBX mesh export is in World.
- Generate (native): G opens five seeded construct generators — tower, cloister, terraces, ruins, megastructure deck; Undo restores the previous plot
- Atmosphere (native): K runs a four-minute day cycle, O slowly turns the build camera for showcases, T lights a torch while walking at night
- Tab for walk mode · WASD · Space jump · H hide UI · M mute
- Service doors (Detail): F or left click while walking to open/close; gamepad X. Doors reopen if the player obstructs closing.

## Build

```powershell
.\build.ps1
```

Requires Visual Studio Build Tools (cl) and the Vulkan SDK (`glslc`).

## World API

[World generation and native runtime contract](docs/World-API.md): 124 parts, Share JSON, MON4/MNS4 saves, materials, viewpoints, animated doors, menu controls, and isolated QA launch flags.

[Native source layout](docs/Source-Layout.md) explains the consolidated modules, generated data, shader stages, and build workflow.
