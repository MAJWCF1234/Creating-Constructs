# Reusable project tools

`sync_catalog.py` regenerates `src/generated/catalog.inc` from HTML `TN` / `CATS` (`PieceCount` must stay aligned). Run it before native builds (`build.ps1` already invokes it). `verify_world_contract.py` checks catalog parity, HTML footprints vs native, and `docs/World-API.md` markers.

`audit_catalog.py` runs the native Vulkan catalog checker, verifies its CSV/JSON results, and creates a lossless preview gallery. Build the game first, then run `py -3 tools/audit_catalog.py`. Its native fixtures never save over the player world. Use `--summarize-only --output <folder>` to regenerate an existing gallery.

`prepare_nature.py` imports the local Retro Official nature models and records their sources. `import_topography.py` imports the topography textures. `import_brick.py` imports lime-mortar red clay brick (side/top/mossy/stairs/arch/tudor) from `O:\Assets\Textures\…` into `src/generated/brick_textures.inc` + HTML. The embed tools support the standalone web build.

`prepare_audio.py` prepares the soundtrack/ambience bank. `remaster_movement.py` packs the approved mastered movement WAVs under `assets/audio/movement`, preserving the first 17 clips byte for byte. `--verify` checks the existing 41-clip bank without changing it.

World generators: `tinyworld_to_mns.py`, `write_concept_save.py`, `make_reference_atrium.py`, `make_overgrown_construct.py`. Format contract: `docs/World-API.md` (MON4 / MNS4 / Share `stride:64` / pieces `0…103`).

Retired one-shot scripts (HTML patch/probe fragments, itch cover/background generators, GDI probe) are recoverable from `output/maintenance/retired-one-off-tools.zip`; its adjacent JSON records SHA-256 hashes. They are historical tools, not build dependencies.

`make_test_garden.py` authors ordinary editable saves for every catalog part/rotation, all 24 surface finishes, neighboring adaptations, and the liminal service court. See `output/test-garden/README.md`.

`make_door_workshop.py` authors the normal editable service-door test save. `Check-Game.cmd` now checks door animation, sprint blocking, four rotations, obstruction reversal, save/Share state and live GPU poses. Pass `--door-review-images` alongside `--smoke-test` for captured door phases.

Native menu checks run as part of `--smoke-test`. Add `--menu-review-images` for app-rendered menu previews, or combine it with `--review-save="path.mns"` for an isolated 15-frame preview of an authored world. See World-API sections 14–17 for material/viewpoint/door schemas and launch behavior.
