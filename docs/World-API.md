# Creating Constructs — World Generation API

This document is the full contract for writing worlds from code: Share JSON, named saves (`.mns`), autosave (`world.bin`), cell keys, piece IDs, cables, and face-mounted pipes.

Use it to build savers, procedural generators, level converters, and batch tools that the HTML sandbox and the native Vulkan app can load.

---

## 1. Coordinate system

| Axis | Meaning | Range |
|------|---------|--------|
| **X** | East | `0 … N-1` |
| **Y** | Up | `0 … 19` (always 20 layers) |
| **Z** | North | `0 … N-1` |

- One **cell** is a 1×1×1 cube. World origin is the corner of cell `(0,0,0)`.
- Cell center in world units: `(x + 0.5, y + 0.5, z + 0.5)`.
- **Plot size `N`** is the square extent in X and Z. Allowed UI sizes: `4, 6, 10, 12, 16, 24, 32`.
- Absolute max plot: **`MaxPlot = 32`**. Generators must keep `4 ≤ N ≤ 32`.
- Empty cells are not stored in Share JSON. In binary formats they are `t = -1`.

**Rotation `r`** is an integer `0…3`. Each step is +90° about **+Y** (counter-clockwise when looking down):

| `r` | Facing (local −Z of the mesh becomes…) |
|-----|----------------------------------------|
| 0 | −Z (south-ish default) |
| 1 | −X |
| 2 | +Z |
| 3 | +X |

Many pieces are rotation-sensitive (doors, cantilevers, ivy, sconces, chase, pipes’ host wall). Materials that fill the whole cube (Block, Boardform, Moss, …) usually ignore `r`.

---

## 2. Cell key encoding (Share JSON)

Every occupied cell is addressed by a single integer key:

```
key = x * 1024 + z * 32 + y
```

Decode:

```
x = key >> 10
z = (key >> 5) & 31
y = key & 31
```

Constraints implied by the bit layout:

- `x, z ∈ [0, 31]` (5 bits each after packing)
- `y ∈ [0, 31]` but only `0…19` are valid in-game

This key is shared by **HTML** (`key(x,y,z)`) and **native** (`exportWorldJson` / `loadWorldJson`).

---

## 3. Share JSON (portable interchange)

### 3.1 Shape

```json
{
  "N": 12,
  "c": [
    [key, {"t": pieceId, "r": rotation}],
    ...
  ],
  "cab": [
    [ax, ay, az, bx, by, bz],
    ...
  ],
  "pipes": [
    {
      "cx": 1, "cy": 2, "cz": 4,
      "face": 0, "u": 1, "v": 1,
      "kind": "pipe",
      "rot": 0,
      "auto": false
    }
  ]
}
```

| Field | Required | Description |
|-------|----------|-------------|
| `N` | yes | Plot size (int). |
| `c` | yes | Sparse cell list. Each entry is `[key, {t, r}]`. |
| `cab` | no | Cable endpoints; default `[]`. |
| `pipes` | no | Face-mounted pipe nodes; default `[]`. |

Rules:

- Omit empty cells entirely.
- `t` must be `0 … PieceCount-1` (currently **0…55**).
- `r` must be `0…3`.
- Duplicate keys: last write wins (avoid duplicates).
- Unknown top-level keys are ignored by the native JSON loader (skipped). Prefer the schema above.

### 3.2 How players load it

**HTML** (`Creating Constructs.html`):

1. Click **Share**.
2. Paste the full JSON into the text box.
3. Click **Load**.

Also persisted in `localStorage` key `creating-constructs` with the same shape (`N`, `c`, `cab`, `pipes`).

**Native** (`CreatingConstructs.exe`):

1. **Share** opens a dialog with exported JSON.
2. Paste and **Load**.

> **Parity note:** Native `exportWorldJson` currently emits `N` / `c` / `cab` only (no `pipes`). Native `loadWorldJson` also does not yet apply a `pipes` array (unknown keys are skipped). The HTML path round-trips pipes. For generators that need pipes in both runtimes, prefer writing **autosave `world.bin` (MON3)** or loading Share JSON in the **HTML** build until native Share gains pipe import/export. Binary autosave already stores pipes.

### 3.3 Minimal valid world

```json
{"N":10,"c":[[0,{"t":0,"r":0}]],"cab":[],"pipes":[]}
```

That places a Block at `(0,0,0)` on a 10×10 plot.

### 3.4 Encoding / decoding helpers (Python)

```python
def cell_key(x: int, y: int, z: int) -> int:
    return x * 1024 + z * 32 + y

def decode_key(k: int) -> tuple[int, int, int]:
    return k >> 10, k & 31, (k >> 5) & 31  # x, y, z

def put(cells: dict, x, y, z, t, r=0):
    cells[(x, y, z)] = (t, r)

def to_share(N: int, cells: dict, cables=(), pipes=()) -> dict:
    return {
        "N": N,
        "c": [[cell_key(x, y, z), {"t": t, "r": r}]
              for (x, y, z), (t, r) in sorted(cells.items())],
        "cab": [list(c) for c in cables],
        "pipes": list(pipes),
    }
```

---

## 4. Piece catalog (`t`)

Canonical names live in `src/catalog.inc` / HTML `TN` (keep them in sync via `tools/sync_catalog.py`). Current **`PieceCount = 56`**.

| ID | Name | Notes |
|----|------|--------|
| 0 | Block | Generic concrete cube |
| 1 | Slab | Half-height |
| 2 | Pillar | Slim column |
| 3 | Glass | Transparent panel |
| 4 | Slope | Ramp / wedge |
| 5 | Moss | Soft floor / wall carpet |
| 6 | Stairs | 4-step run |
| 7 | Column | Round column |
| 8 | Window | Framed opening + glass |
| 9 | Panel | Thin wall panel |
| 10 | Arch | Arch opening |
| 11 | Railing | Guard rail |
| 12 | Beam | Horizontal beam |
| 13 | Light | Strip light |
| 14 | Tree | Nature tree |
| 15 | Ivy | Climbing vines (attach to wall behind) |
| 16 | Cable | *Tool* — see cables section; do not place as a cell |
| 17 | Planter | Box planter |
| 18 | Brick | Brick material cube |
| 19 | Timber | Wood material cube |
| 20 | Steel | Metal material cube |
| 21 | Fins | Vertical fins |
| 22 | Cobogó | Lattice screen |
| 23 | Doorway | Flat doorway |
| 24 | Pool | Basin / pool rim |
| 25 | Ladder | Climbable |
| 26 | Bollard | Path light |
| 27 | Sconce | Wall lamp (faces with `r`) |
| 28 | Dome | Hemisphere |
| 29 | Silo | Cylinder |
| 30 | Vault | Barrel vault |
| 31 | Fence | Thin fence |
| 32 | Slit | Narrow opening |
| 33 | Ribbon | Horizontal ribbon window |
| 34 | Floor | Thin floor plate |
| 35 | Pergola | Open roof frame |
| 36 | Fine stairs | 8-step stair |
| 37 | Pendant | Hanging lamp |
| 38 | Obelisk | Tall monument |
| 39 | Bench | Seat |
| 40 | Water | Traversable water surface |
| 41 | Birch | Birch tree |
| 42 | Pine | Pine tree |
| 43 | Pipe | *Pipe tool* — use `pipes[]`, not cells |
| 44 | Valve | *Pipe tool* |
| 45 | Gauge | *Pipe tool* |
| 46 | Outlet | *Pipe tool* |
| 47 | Boardform | Board-marked concrete material |
| 48 | Cantilever | Overhanging slab |
| 49 | Batter | Sloped retaining mass |
| 50 | Waffle | Coffered underside |
| 51 | Portal | Deep recessed doorway |
| 52 | Clerestory | High horizontal slot |
| 53 | Plinth | Low podium / seating edge |
| 54 | Chase | Vertical service recess |
| 55 | Spout | Rain scupper |

**Do not** put types `16` or `43–46` in `c` for generators. Cables and pipes have their own arrays.

### Categories (palette only)

Structure / Shapes / Openings / Detail / Nature / Material / Pipes — see `catTypes` in `catalog.inc`. Every piece ID must appear in some category for native self-tests; generators need not care about categories.

---

## 5. Cables (`cab`)

Cables are free-space polylines between two 3D points (face anchors), not cells.

```json
"cab": [[ax, ay, az, bx, by, bz]]
```

- Coordinates are **world units** (floats), not cell indices.
- Typical anchors sit slightly outside a face, e.g. `(x+0.5, y+0.5, z)` ± offset.
- Limits: length must be finite; endpoints should stay within a reasonable bound (`|coord| ≲ 100` in native validation). Max count **2048**.

---

## 6. Pipes (`pipes`)

Pipes are a **3×3 grid per cell face**, SurfaceWorks-style. They do not occupy the voxel `c` grid.

### 6.1 Node fields

| Field | Type | Meaning |
|-------|------|---------|
| `cx, cy, cz` | int | Host cell (must contain a solid piece) |
| `face` | int `0…5` | Which face of that cell |
| `u, v` | int `0…2` | Slot on the 3×3 face grid |
| `kind` | string | `"pipe"` \| `"valve"` \| `"gauge"` \| `"outlet"` |
| `rot` | int `0…3` | Orientation for valve/gauge (and pipe run bias) |
| `auto` | bool | Auto-orient valve/gauge to neighbors |

Native binary stores the same as packed `int8` fields with `kind` as:

| kind string | binary `kind` |
|-------------|----------------|
| pipe | 0 |
| valve | 1 |
| gauge | 2 |
| outlet | 3 |

(`pipeKindFromType`: piece `43+kind`.)

### 6.2 Faces

Outward normals (`PIPE_N`):

| face | Normal | Side of cell |
|------|--------|--------------|
| 0 | +X | East face |
| 1 | −X | West face |
| 2 | +Y | Top |
| 3 | −Y | Bottom |
| 4 | +Z | North face |
| 5 | −Z | South face |

Tangent bases (`PIPE_U`, `PIPE_V`) match HTML and `src/pipes.inc`. Slot center:

```
origin = (cx+0.5, cy+0.5, cz+0.5)
          + N * 0.5
          + U * (u - 1) * (1/3)
          + V * (v - 1) * (1/3)
          + N * 0.055   // standoff
```

### 6.3 Connectivity rules (practical)

- A `"pipe"` node connects to orthogonal neighbors on the same face grid (and can step to adjacent cells when `u`/`v` overflow).
- `"valve"` / `"gauge"`: `auto: true` lets the game rotate them to fit runs; set `rot` explicitly for fixed orientation.
- `"outlet"`: only valid on non-top/bottom faces with `cy == 0` and `v == 0` (floor-edge outlet) in HTML validation.
- Host cell should be a solid architectural piece (walls/chase work best). Empty air hosts will not render usefully.

Example vertical run on the east face of `(1, *, 4)`:

```json
"pipes": [
  {"cx":1,"cy":1,"cz":4,"face":0,"u":1,"v":1,"kind":"pipe","rot":0,"auto":false},
  {"cx":1,"cy":2,"cz":4,"face":0,"u":1,"v":1,"kind":"pipe","rot":0,"auto":false},
  {"cx":1,"cy":2,"cz":4,"face":0,"u":1,"v":2,"kind":"valve","rot":1,"auto":true}
]
```

---

## 7. Binary autosave — `world.bin` (MON3)

Path (Windows):

```
%LOCALAPPDATA%\CreatingConstructs\world.bin
```

(Smoke tests use `test-world.bin` in the same folder.)

### 7.1 Layout (little-endian)

| Offset | Type | Field |
|--------|------|--------|
| 0 | `u32` | Magic **`0x334F4E4D`** (`MON3`) |
| 4 | `i32` | `N` (plot size) |
| 8 | `Cell[20480]` | Full grid, always **32×32×20** |
| … | `u32` | Cable count |
| … | `Cable[count]` | Each cable = 6×`f32` (`ax…bz`) |
| … | `u32` | Pipe count |
| … | `PipeNode[count]` | 9×`i8` per node |

Legacy magic **`0x324F4E4D` (`MON2`)** used a 16×16×20 cell blob and no pipes; the loader migrates cells into the 32³ grid.

### 7.2 Cell record

```c
struct Cell { int8_t t; int8_t r; };  // 2 bytes, empty: t = -1
```

Linear index (native):

```
idx = x * (32 * 20) + z * 20 + y
    = x * 640 + z * 20 + y
```

Always write **20480** cells even if `N < 32`. Cells outside `N` should be empty (`t = -1`).

### 7.3 PipeNode record

```c
struct PipeNode {
  int8_t cx, cy, cz, face, u, v, kind, rot, autoAlign;
};  // 9 bytes
```

`autoAlign` is `0` or `1` (JSON `auto`).

### 7.4 Python writer sketch

```python
import struct

MAX, H = 32, 20
CELLS = MAX * MAX * H  # 20480

def write_world_bin(path, N, cells, cables=(), pipes=()):
    raw = bytearray()
    for i in range(CELLS):
        raw += struct.pack("<bb", -1, 0)
    for (x, y, z), (t, r) in cells.items():
        off = (x * 640 + z * 20 + y) * 2
        struct.pack_into("<bb", raw, off, t, r)

    blob = struct.pack("<Ii", 0x334F4E4D, N) + raw
    blob += struct.pack("<I", len(cables))
    for a in cables:
        blob += struct.pack("<6f", *a)
    blob += struct.pack("<I", len(pipes))
    KIND = {"pipe": 0, "valve": 1, "gauge": 2, "outlet": 3}
    for p in pipes:
        blob += struct.pack(
            "<9b",
            p["cx"], p["cy"], p["cz"], p["face"], p["u"], p["v"],
            KIND[p["kind"]], p.get("rot", 0), 1 if p.get("auto") else 0,
        )
    path.write_bytes(blob)
```

---

## 8. Named saves — `.mns` (MNS2)

Directory:

```
%LOCALAPPDATA%\CreatingConstructs\Saves\*.mns
```

### 8.1 Layout (little-endian)

| Field | Size | Notes |
|-------|------|--------|
| Magic | `u32` **`0x32534E4D`** (`MNS2`) | Legacy `MNS1` = `0x31534E4D` (16³ cells) |
| Title | `wchar_t[64]` = **128 bytes** UTF-16LE, NUL-padded | Must be non-empty; `title[63] == 0` |
| SavedView | **76 bytes** | Camera + atmosphere (below) |
| `N` | `i32` | Plot size |
| Cells | `Cell[20480]` | Same as autosave |
| Cable count | `u32` | |
| Cables | `6×f32` each | |

**Named saves do not currently store pipes.** Pipe runs are lost if you only write `.mns`. Prefer Share JSON (HTML) or `world.bin` when pipes matter; or regenerate pipes after load.

Legacy **MNS1** used 16×16×20 cells and max `N ≤ 16`.

### 8.2 SavedView (76 bytes)

Packed as little-endian:

```
float target[3]
float player[3]
float theta, phi, distance
float yaw, pitch, fov
float sunHeight, sunAngle, exposure, fog, sat
int32  mood          // 0..4
uint32 flags         // bit0 night, bit1 rain, bit2 ao, bit3 walk
```

Validation (native):

- All floats finite, `|f| ≤ 10000`
- `mood ∈ [0, 4]`
- `distance ∈ [4, 70]`, `fov ∈ [45, 110]`, `phi ∈ [0.1, 1.6]`
- `exposure ∈ [0.5, 1.8]`, `fog ∈ [0, 1]`
- `flags` only lower 4 bits set

Mood indices:

| 0 | Overcast |
| 1 | Noon |
| 2 | Golden hour |
| 3 | Blue hour |
| 4 | Concrete overcast |

### 8.3 Python writer (matches `tools/make_overgrown_construct.py`)

```python
def pack_view(target, player, theta, phi, dist, yaw, pitch, fov,
              sun_h, sun_a, exp, fog, sat, mood, flags) -> bytes:
    v = struct.pack("<17fiI", *target, *player, theta, phi, dist,
                    yaw, pitch, fov, sun_h, sun_a, exp, fog, sat, mood, flags)
    assert len(v) == 76
    return v

def write_mns(path, name: str, N, cells_raw: bytes, cables, view: bytes):
    title = name.encode("utf-16le").ljust(128, b"\0")
    blob = (struct.pack("<I", 0x32534E4D) + title + view
            + struct.pack("<i", N) + cells_raw
            + struct.pack("<I", len(cables))
            + b"".join(struct.pack("<6f", *a, *b) for a, b in cables))
    path.write_bytes(blob)
```

---

## 9. End-to-end: generate a world in code

### 9.1 Recommended pipeline for tools

1. Build a `dict[(x,y,z)] -> (t,r)` for solid architecture.
2. Optionally add `cab` endpoints and `pipes` nodes.
3. Emit **Share JSON** for HTML / paste interchange.
4. Optionally emit **`world.bin`** for native autosave (includes pipes).
5. Optionally emit **`.mns`** for the Saves browser (camera + title; no pipes today).

Repo examples:

- `tools/make_overgrown_construct.py` — large 32² yard → `.json` + `.mns`
- `worlds/Brutal Overgrown.json` — compact brutalist Share sample

### 9.2 Full generator example

```python
"""Write a tiny brutalist courtyard as Share JSON."""
from pathlib import Path
import json

N = 12
cells = {}

def put(x, y, z, t, r=0):
    assert 0 <= x < N and 0 <= z < N and 0 <= y < 20
    cells[x, y, z] = (t, r)

def key(x, y, z):
    return x * 1024 + z * 32 + y

# Podium
for x in range(N):
    for z in range(N):
        put(x, 0, z, 47)  # Boardform

# West wall
for z in range(1, 8):
    for y in range(1, 4):
        put(1, y, z, 47)

# Portal + waffle ceiling
put(1, 1, 1, 51, 2)
put(1, 3, 3, 50)
put(1, 3, 4, 50)

# Overgrowth
put(3, 0, 3, 5)   # Moss
put(3, 1, 3, 14)  # Tree
put(0, 1, 3, 15)  # Ivy
put(0, 2, 3, 15)

pipes = [{
    "cx": 1, "cy": 2, "cz": 4, "face": 0,
    "u": 1, "v": 1, "kind": "pipe", "rot": 0, "auto": False,
}]

world = {
    "N": N,
    "c": [[key(x, y, z), {"t": t, "r": r}]
          for (x, y, z), (t, r) in sorted(cells.items())],
    "cab": [],
    "pipes": pipes,
}

Path("my-world.json").write_text(json.dumps(world, separators=(",", ":")), encoding="utf-8")
print("pieces", len(cells))
```

Load `my-world.json` via Share → paste → Load in the HTML app.

### 9.3 Design tips for generators

- **Floor first** — walk mode needs continuous solids under the player; leave air for corridors.
- Prefer **Boardform (47)** over Block (0) for brutalist mass so materials read poured.
- Use **Chase (54)** behind pipe runs so plumbing sits *in* the wall language.
- Cap piece count for performance (thousands are fine; tens of thousands of nature+ivy on 32² can get heavy).
- Keep ivy (`15`) on cells that have a supporting wall behind their facing.
- Water (`40`) is traversable; don’t surround the player with unbroken solids at eye height without openings.

---

## 10. Runtime limits (native validation)

| Resource | Limit |
|----------|--------|
| Plot `N` | 4…32 |
| Height | 20 |
| Piece type | 0…`PieceCount-1` |
| Rotation | 0…3 |
| Cables | ≤ 2048 |
| Pipes (autosave) | ≤ 8000 |
| Named save title | ≤ 63 UTF-16 code units |

Invalid `.mns` / `world.bin` files are rejected without clobbering the open world (named load also writes `recovery.mns` first).

---

## 11. Format cheat sheet

| Format | Magic | Cells | Cables | Pipes | Camera | Best for |
|--------|-------|-------|--------|-------|--------|----------|
| Share JSON | — | sparse keys | yes | yes (HTML) | no | Generators, sharing, itch drops |
| `world.bin` | `MON3` | dense 32×32×20 | yes | yes | no | Native autosave / full fidelity |
| `.mns` | `MNS2` | dense 32×32×20 | yes | **no** | yes | Saves UI presets / titled slots |

---

## 12. Related source of truth

| Topic | File |
|-------|------|
| Piece names / counts | `src/catalog.inc`, HTML `TN` |
| Sync catalog from HTML | `tools/sync_catalog.py` |
| JSON export/import (native) | `src/platform.inc` (`exportWorldJson`, `loadWorldJson`) |
| Autosave R/W | `src/main.cpp` (`save`, `load`) |
| Named saves | `src/saves.inc` |
| Pipe math | `src/pipes.inc`, HTML pipe block |
| Example generator | `tools/make_overgrown_construct.py` |
| Sample worlds | `worlds/*.json`, `worlds/*.mns` |

When piece IDs change, bump generators and re-run `tools/sync_catalog.py` so native and HTML stay aligned.
