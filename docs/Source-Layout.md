# Native source layout

The native game uses one C++ translation unit, `src/main.cpp`. Related include
fragments are grouped by subsystem and included in dependency order. The cleanup
consolidated **67 source files into 41** (42 with the later `world/tools.inc`); `main.cpp` is the only file directly in
`src`. The editor configuration lives at the project root in `.vscode`.

| Location | Contents |
|----------|----------|
| `src/main.cpp` | Core world, geometry, physics, Vulkan setup, and frame loop |
| `src/audio/mixer.inc` | Mixer, audio events, and mood soundtrack definitions |
| `src/world/` | State/material recipes, saves, pieces, vegetation, landscape, pipes, doors, export, shared solidity, imported props (`props.inc`), and build tools (`tools.inc`: box / select / stamp / paint, symmetry, build plane, day cycle, torch, construct generators) |
| `src/render/` | Texture baking, foliage tracing, postprocessing, frame/readback support, thumbnails, and placement previews |
| `src/platform/windows.inc` | Win32 windows, input, gamepad handling, launch flags, and application lifecycle |
| `src/platform/ui.inc` | Save-browser helpers and the native editor/menu composition |
| `src/tests/regression.inc` | Smoke/regression tests and catalog audit, preserving their original dependency order |
| `src/generated/` | Generated catalog, asset, texture, mesh, prop (`prop_models.inc`, `prop_textures.inc`, `prop_shader.glsl`) and SPIR-V data files |
| `src/shaders/` | Seven Vulkan stage entry points and six shared GLSL files |

## What was combined

- Door state and surface material/chunk definitions → `world/state.inc`.
- Ivy, shape helpers, nature, and special nature → `world/vegetation.inc`.
- Moss nets and basins → `world/landscape.inc`.
- Save-browser UI helpers and glass/menu panels → `platform/ui.inc`.
- Gamepad handling → `platform/windows.inc`.
- Mood soundtrack definitions → `audio/mixer.inc`.
- Performance counters, captures, glass readback, and region blur → `render/frame_support.inc`.
- All test fragments plus catalog audit → `tests/regression.inc`.

The unused old `panels.inc` implementation was retired. A local recovery copy and
source migration map are in `output/source-consolidation/`; that folder is build
output and is excluded from Git. Original files remain recoverable in Git history.

Generated multi-megabyte arrays stay separate from hand-written logic. The Vulkan
stage entry points stay separate because `glslc` compiles them individually. Shared
GLSL still includes `world/solidity.inc` for the CPU/GPU solidity contract.

## Build and generation

Run `build.ps1` from the project root. It synchronizes
`src/generated/catalog.inc`, compiles `src/shaders/*`, emits
`src/generated/shaders.h`, and builds `Play/CreatingConstructs.exe`.

The existing asset importers and HTML embedding tools now read/write
`src/generated/`. Generated files are committed so a normal native build does not
require the external asset-library drives. Existing import commands are unchanged.

Run `py -3 tools/verify_world_contract.py` for catalog/documentation checks and
`Play/Check-Game.cmd` for the native smoke run followed by catalog auditing.

This reorganization preserves the expanded C++ content, shader data, save formats,
and behavior. Validation of the rebuilt executable included matching its native
`.text` section to the previous build and a 250-frame Vulkan smoke run with no
validation errors.
