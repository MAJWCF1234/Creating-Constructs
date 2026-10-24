# Creating Constructs — World Generation API

**Core world contract (native + HTML):** plots support **64×64** with Share JSON `stride:64`; autosave writes **MON4** (`0x344F4E4D`); named snapshots write **MNS4** (`0x34534E4D`). MNS4 keeps the MNS3 64×64×20 cell layout and appends `uint32_t pipeCount` plus packed 9-byte `PipeNode` records after the cables. MNS1 / MNS2 / MNS3 remain readable. Native Share JSON round-trips `pipes` (including `rot` and `auto`). **`PieceCount = 128`** (IDs `0…127`).

**Updated: October 24, 2026.** Native extensions include material recipes, named viewpoints, animated service-door targets, the main / pause menu, and the build tools / construct generators in section 16.4. The shared piece catalog does not imply feature parity between native and HTML.

This document is the full contract for writing worlds from code: Share JSON, named saves (`.mns`), autosave (`world.bin`), cell keys, piece IDs, cables, and face-mounted pipes.

Use it to build savers, procedural generators, level converters, and batch tools that the HTML sandbox and the native Vulkan app can load. Keep piece names in sync with `tools/sync_catalog.py` (HTML `TN` → `src/generated/catalog.inc`).

---

## 1. Coordinate system

| Axis | Meaning | Range |
|------|---------|--------|
| **X** | East | `0 … N-1` |
| **Y** | Up | `0 … 19` (always 20 layers) |
| **Z** | North | `0 … N-1` |

- One **cell** is a 1×1×1 cube. World origin is the corner of cell `(0,0,0)`.
- Cell center in world units: `(x + 0.5, y + 0.5, z + 0.5)`.
- **Plot size `N`** is the square extent in X and Z. Allowed UI sizes: `4, 6, 10, 12, 16, 24, 32, 48, 64`.
- Absolute max plot: **`MaxPlot = 64`**. Generators must keep `4 ≤ N ≤ 64`.
- Dense grid size: **`WorldCells = 64 × 64 × 20 = 81920`** cells.
- Empty cells are not stored in Share JSON. In binary formats they are `t = -1`.

**Rotation `r`** is an integer `0…3`. Each step is +90° about **+Y** (counter-clockwise when looking down):

| `r` | Facing (local −Z of the mesh becomes…) |
|-----|----------------------------------------|
| 0 | −Z (default) |
| 1 | −X |
| 2 | +Z |
| 3 | +X |

Many pieces are rotation-sensitive (doors, cantilevers, ivy, sconces, chase, wall lamps). Full-cube materials (Block, Boardform, Moss, …) usually ignore `r`.

---

## 2. Cell key encoding (Share JSON)

### 2.1 Current (preferred) — dense stride

When `stride` is present (native export always writes `"stride":64`; HTML uses `MAX=64`):

```
key = x * (stride * 20) + z * 20 + y
    = x * 1280 + z * 20 + y     // when stride == 64
```

Decode:

```
pitch = stride * 20
x = key / pitch
rem = key % pitch
z = rem / 20
y = rem % 20
```

HTML `key(x,y,z)` is identical: `x*(MAX*H)+z*H+y` with `MAX=64`, `H=20`.

### 2.2 Legacy keys (no `stride`)

Older Share JSON used a packed 5-bit layout (max plot 32):

```
key = x * 1024 + z * 32 + y
x = key >> 10
z = (key >> 5) & 31
y = key & 31
```

Native `loadWorldJson` still accepts this when `stride` is omitted. Prefer emitting `stride` for any new tool.

---

## 3. Share JSON (portable interchange)

### 3.1 Shape

```json
{
  "N": 12,
  "stride": 64,
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
| `N` | yes | Plot size (int), `4…64`. |
| `stride` | recommended | Dense key stride (native writes `64`). Omit only for legacy packed keys. |
| `c` | yes | Sparse cell list. Each entry is `[key, {t, r}]`; native also exports `m` and `d`. |
| `cab` | no | Cable endpoints; default `[]`. |
| `pipes` | no | Face-mounted pipe nodes; default `[]`. |
| `views` | no | Native named viewpoint arrays; default `[]`. See section 14. |

Rules:

- Omit empty cells entirely.
- `t` must be `0 … PieceCount-1` (currently **0…127**).
- `r` must be `0…3`.
- Duplicate keys: last write wins (avoid duplicates).
- Native `m` defaults to `0` (original material); `d` defaults to `0` (closed). See sections 14–15 for validation and encoding.
- Unknown top-level keys are ignored by the native JSON loader. Prefer the schema above.

### 3.2 How players load it

**HTML** (`Creating Constructs.html`):

1. Click **Share**.
2. Paste the full JSON into the text box.
3. Click **Load**.

Also persisted in `localStorage` key `creating-constructs` with the same shape (`N`, `c`, `cab`, `pipes`; HTML also keeps `stride:MAX` on Share export).

**Native** (`CreatingConstructs.exe`):

1. **Share** opens a dialog with exported JSON (`N`, `stride`, `c`, `cab`, `pipes`, `views`), including per-cell `m` / `d`.
2. Paste and **Load** (clipboard load also supported).

### 3.3 Minimal valid world

```json
{"N":10,"stride":64,"c":[[0,{"t":0,"r":0}]],"cab":[],"pipes":[]}
```

That places a Block at `(0,0,0)` on a 10×10 plot.

### 3.4 Encoding / decoding helpers (Python)

```python
STRIDE, H = 64, 20

def cell_key(x: int, y: int, z: int, stride: int = STRIDE) -> int:
    return x * (stride * H) + z * H + y

def decode_key(k: int, stride: int = STRIDE) -> tuple[int, int, int]:
    pitch = stride * H
    x, rem = divmod(k, pitch)
    z, y = divmod(rem, H)
    return x, y, z

def to_share(N: int, cells: dict, cables=(), pipes=(), stride: int = STRIDE) -> dict:
    return {
        "N": N,
        "stride": stride,
        "c": [[cell_key(x, y, z, stride), {"t": t, "r": r}]
              for (x, y, z), (t, r) in sorted(cells.items())],
        "cab": [list(c) for c in cables],
        "pipes": list(pipes),
    }
```

---

## 4. Piece catalog (`t`)

Canonical names live in `src/generated/catalog.inc` / HTML `TN` (keep them in sync via `tools/sync_catalog.py`). Current **`PieceCount = 128`**. GPU cell encoding uses **`PieceEncodingStride = 128`** (`1 + t + stride * r`).

| ID | Name | Notes |
|----|------|--------|
| 0 | Block | Generic concrete cube |
| 1 | Slab | Half-height |
| 2 | Pillar | Slim column |
| 3 | Glass | Transparent panel |
| 4 | Slope | Ramp / wedge (high at −Z) |
| 5 | Moss | Adaptive surface-nets moss |
| 6 | Stairs | 4-step run + side handrails when open |
| 7 | Column | Round column |
| 8 | Window | Framed opening + glass |
| 9 | Panel | Thin wall panel |
| 10 | Arch | Arch opening |
| 11 | Railing | Guard rail |
| 12 | Beam | Horizontal beam |
| 13 | Light | Strip light |
| 14 | Tree | Nature tree |
| 15 | Ivy | Climbing vines (wall-flush auto-face) |
| 16 | Cable | *Tool* — use `cab[]`, not cells |
| 17 | Planter | Box planter |
| 18 | Brick | Brick material cube |
| 19 | Timber | Wood material cube |
| 20 | Steel | Metal material cube |
| 21 | Fins | Vertical fins (wall-flush) |
| 22 | Cobogó | Inset lattice screen; diamonds extrude outward; tiled edge merge |
| 23 | Doorway | Flat doorway — **2×2** footprint (W×H) |
| 24 | Pool | Basin / pool rim |
| 25 | Ladder | Climbable (wall-flush) |
| 26 | Bollard | Path light |
| 27 | Sconce | Wall lamp (wall-flush) |
| 28 | Dome | Hemisphere |
| 29 | Silo | Cylinder |
| 30 | Vault | Barrel vault |
| 31 | Fence | Thin fence |
| 32 | Slit | Narrow opening (wall-flush) |
| 33 | Ribbon | Horizontal ribbon window (wall-flush) |
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
| 51 | Portal | Deep recessed doorway — **2×2** footprint |
| 52 | Clerestory | High horizontal slot (wall-flush) |
| 53 | Plinth | Low podium / seating edge |
| 54 | Chase | Vertical service recess |
| 55 | Spout | Rain scupper |
| 56 | Pilotis | Raised column capital — **1×2** tall |
| 57 | Megastep | Oversized tread — **2×1** wide |
| 58 | Lightwell | Open shaft rim — **2×2** plan × **2** tall |
| 59 | Bridge | Inhabited span — **2×2** walk tunnel |
| 60 | Core | Tower shaft — **1×2** tall walk-in |
| 61 | Aggregate | Exposed-aggregate concrete |
| 62 | Corbel | Stepped bracket |
| 63 | Stone | Procedural rock (surface nets) |
| 64 | Boulder | Larger procedural rock |
| 65 | Log | Fallen timber mesh |
| 66 | Stump | Tree stump mesh |
| 67 | Mushroom | Soft prop (nonblocking) |
| 68 | Strut | Diagonal mega brace — **1×2** tall |
| 69 | Girder | Thick span — **2×1** wide |
| 70 | Niche | Deep square alcove (open local −Z; flush auto-face) |
| 71 | Hood | Hooded niche projecting past local −Z |
| 72 | Roundel | Circular facade opening |
| 73 | Stripe | Horizontal stripe / band |
| 74 | Half | Half-block mass |
| 75 | Lilypad | Water-surface pad (nature) |
| 76 | Grass | Ground grass cover |
| 77 | Cascade | Vertical nature cascade |
| 78 | Flight | Wide stair flight — **2×1** wide |
| 79 | Amber | Warm accent light |
| 80 | Cool | Cool accent light |
| 81 | Ribbed | Ribbed material cube |
| 82 | Green wall | Living wall panel (wall-flush) |
| 83 | Lattice | Open lattice screen (wall-flush) |
| 84 | Glassblock | Glass-block panel (wall-flush) |
| 85 | Embrasure | Deep punched opening (wall-flush) |
| 86 | Shrub | Nature shrub |
| 87 | Hang | Hanging nature accent |
| 88 | Slopegrass | Grassy slope cover |
| 89 | Captop | Cap / coping vegetation |
| 90 | Soffitveg | Underside vegetation |
| 91 | Deckgrass | Deck / surface grass |
| 92 | Dropveg | Dropping vegetation |
| 93 | Downslope | Opposite slope (high at +Z) |
| 94 | Quadpier | Four slender pillars in **one** cell |
| 95 | Angle | Wall-flush cube lamp with downward aperture |
| 96 | Service entry | Interactive hinged 2 m door — **1×2** tall; `d` stores the open target |
| 97 | Air vent | Wall louvre grille (wall-flush) |
| 98 | Cable tray | Ladder tray along X, merges across cells |
| 99 | Ceiling light | Recessed fluorescent panel (fixture) |
| 100 | Exit sign | Green pictogram housing (wall-flush fixture) |
| 101 | Neon cyan | Twin cyan tubes on a dark backing (wall-flush fixture) |
| 102 | Neon rose | Twin rose tubes (wall-flush fixture) |
| 103 | Terminal | Industrial terminal with lit screen (wall-flush) |
| 104 | Neon violet | Twin violet tubes (wall-flush fixture) — *Cyber* |
| 105 | Wall screen | Retro TV set against the wall (wall-flush fixture; Retro Official OFFICE) |
| 106 | Server cabinet | Metal cabinet with LED rows; solid (OFFICE) |
| 107 | Trash can | Street trash can (URBAN) |
| 108 | Grate floor | Steel grating plate at cell floor (tileable grating texture) |
| 109 | Barrier | Jersey barrier — **2×1** footprint (URBAN) |
| 110 | Duct | Square ventilation duct filling the cell (URBAN) |
| 111 | Fan vent | Vent cover on the wall (wall-flush; URBAN) |
| 112 | Street light | Street lamp — **1×2** tall fixture (URBAN) |
| 113 | Light strip | Floor-level cyan strip on a wall base (wall-flush fixture) |
| 114 | Grid floor | Metal plate with glowing grid lines |
| 115 | Bollard light | Concrete bollard with light (URBAN fixture) |
| 116 | Carpet | Office carpet cube — *Liminal* |
| 117 | Yellow wall | Yellow plaster cube with dado stripe |
| 118 | Ceiling tile | Cream plaster drop-ceiling tile with T-bars at cell top |
| 119 | Tube light | Bare twin fluorescent tubes (fixture) |
| 120 | Pool tile | Ceramic pool tile cube |
| 121 | Office door | Blue office door in a dark frame (wall-flush, non-interactive) |
| 122 | Partition | Office divider panel (OFFICE) |
| 123 | Water cooler | Water cooler with bottle (OFFICE) |
| 124 | Desk | Office desk — **2×1** footprint (OFFICE) |
| 125 | Office chair | Office chair (OFFICE) |
| 126 | File cabinet | Metal file cabinet (OFFICE) |
| 127 | Painted pillar | Square yellow plaster pillar |

**Do not** put types `16` or `43–46` in `c` for generators. Cables and pipes have their own arrays.

### Multi-cell footprints

Stored at the min corner; leave covered neighbors empty. Width/depth follow rotation (`r` spins local +X / +Z).

| Piece | Footprint (local W×D×H) |
|-------|-------------------------|
| Doorway (23), Portal (51) | 2×1×2 |
| Pilotis (56), Core (60), Strut (68), Street light (112) | 1×1×2 |
| Megastep (57), Girder (69), Flight (78), Barrier (109), Desk (124) | 2×1×1 |
| Lightwell (58) | 2×2×2 |
| Bridge (59) | 2×1×2 |
| Quadpier (94) | **1×1×1** (four pillars packed in-cell) |

Native and HTML `footprintW` / `footprintD` / `footprintH` must match.

### Categories (palette only)

Structure / Shapes / Openings / Detail / Nature / Material / Pipes / Cyber / Liminal — see `catTypes` in `catalog.inc`. Every piece ID must appear in some category for native self-tests; generators need not care about categories.

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
| `cx, cy, cz` | int | Host cell (should contain a solid piece) |
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

Tangent bases (`PIPE_U`, `PIPE_V`) match HTML and `src/world/pipes.inc`. Slot center:

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
- Host cell should be a solid architectural piece (walls/chase work best).

Example vertical run on the east face of `(1, *, 4)`:

```json
"pipes": [
  {"cx":1,"cy":1,"cz":4,"face":0,"u":1,"v":1,"kind":"pipe","rot":0,"auto":false},
  {"cx":1,"cy":2,"cz":4,"face":0,"u":1,"v":1,"kind":"pipe","rot":0,"auto":false},
  {"cx":1,"cy":2,"cz":4,"face":0,"u":1,"v":2,"kind":"valve","rot":1,"auto":true}
]
```

---

## 7. Binary autosave — `world.bin` (MON4)

Path (Windows):

```
%LOCALAPPDATA%\CreatingConstructs\world.bin
```

(Smoke tests use `test-world.bin` in the same folder.)

### 7.1 Layout (little-endian)

| Offset | Type | Field |
|--------|------|--------|
| 0 | `u32` | Magic **`0x344F4E4D`** (`MON4`) |
| 4 | `i32` | `N` (plot size) |
| 8 | `Cell[81920]` | Full grid, always **64×64×20** |
| … | `u32` | Cable count |
| … | `Cable[count]` | Each cable = 6×`f32` (`ax…bz`) |
| … | `u32` | Pipe count |
| … | `PipeNode[count]` | 9×`i8` per node |
| … | Optional extension chunks | `MAT1` → `VEW1` → `DOR1`; section 14 |

Legacy:

| Magic | Grid | Notes |
|-------|------|--------|
| `MON2` `0x324F4E4D` | 16×16×20 | Migrated into 64×64×20 on load; no pipes |
| `MON3` `0x334F4E4D` | 32×32×20 | Migrated into 64×64×20 on load; includes pipes |

### 7.2 Cell record

```c
struct Cell { int8_t t; int8_t r; };  // 2 bytes, empty: t = -1
```

Linear index (native):

```
idx = x * (64 * 20) + z * 20 + y
    = x * 1280 + z * 20 + y
```

Always write **81920** cells even if `N < 64`. Cells outside `N` should be empty (`t = -1`).

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

MAX, H = 64, 20
CELLS = MAX * MAX * H  # 81920

def write_world_bin(path, N, cells, cables=(), pipes=()):
    raw = bytearray()
    for i in range(CELLS):
        raw += struct.pack("<bb", -1, 0)
    for (x, y, z), (t, r) in cells.items():
        off = (x * (MAX * H) + z * H + y) * 2
        struct.pack_into("<bb", raw, off, t, r)

    blob = struct.pack("<Ii", 0x344F4E4D, N) + raw
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

## 8. Named saves — `.mns` (MNS4)

Directory:

```
%LOCALAPPDATA%\CreatingConstructs\Saves\*.mns
```

### 8.1 Layout (little-endian)

| Field | Size | Notes |
|-------|------|--------|
| Magic | `u32` **`0x34534E4D`** (`MNS4`) | See legacy table |
| Title | `wchar_t[64]` = **128 bytes** UTF-16LE, NUL-padded | Must be non-empty; `title[63] == 0` |
| SavedView | **76 bytes** | Camera + atmosphere (below) |
| `N` | `i32` | Plot size |
| Cells | `Cell[81920]` | Same dense 64×64×20 as autosave |
| Cable count | `u32` | |
| Cables | `6×f32` each | |
| Pipe count | `u32` | **MNS4 only** |
| Pipes | `PipeNode` × count | **MNS4 only** |
| Extensions | variable | Optional `MAT1` → `VEW1` → `DOR1` chunks; section 14 |

Legacy magic / grid:

| Magic | Grid | Pipes |
|-------|------|--------|
| `MNS1` `0x31534E4D` | 16×16×20 | no |
| `MNS2` `0x32534E4D` | 32×32×20 | no |
| `MNS3` `0x33534E4D` | 64×64×20 | no |
| `MNS4` `0x34534E4D` | 64×64×20 | **yes** |

### 8.2 SavedView (76 bytes)

Packed as little-endian:

```
float target[3]
float player[3]
float theta, phi, distance
float yaw, pitch, fov
float sunHeight, sunAngle, exposure, fog, sat
int32  mood          // 0..4
uint32 flags
```

`flags` bits:

| Bit | Meaning |
|-----|---------|
| 0 | night |
| 1 | rain |
| 2 | ao |
| 3 | walk |
| 4 | wind |
| 8…15 | clouds × 100 (0…100) |

Validation (native):

- All floats finite, `|f| ≤ 10000`
- `mood ∈ [0, 4]`
- `distance ∈ [4, 70]`, `fov ∈ [40, 110]`, `phi ∈ [0.1, 1.6]`
- `exposure ∈ [0.5, 1.8]`, `fog ∈ [0, 1]`
- `(flags & ~0xFF1F) == 0`

Mood indices:

| 0 | Overcast |
| 1 | Noon |
| 2 | Golden hour |
| 3 | Blue hour |
| 4 | Concrete overcast |

### 8.3 Python writer (MNS4)

```python
def pack_view(target, player, theta, phi, dist, yaw, pitch, fov,
              sun_h, sun_a, exp, fog, sat, mood, flags) -> bytes:
    v = struct.pack("<17fiI", *target, *player, theta, phi, dist,
                    yaw, pitch, fov, sun_h, sun_a, exp, fog, sat, mood, flags)
    assert len(v) == 76
    return v

def write_mns4(path, name: str, N, cells_raw: bytes, cables, pipes, view: bytes):
    title = name.encode("utf-16le").ljust(128, b"\0")
    KIND = {"pipe": 0, "valve": 1, "gauge": 2, "outlet": 3}
    pipe_blob = b"".join(
        struct.pack("<9b", p["cx"], p["cy"], p["cz"], p["face"], p["u"], p["v"],
                    KIND[p["kind"]], p.get("rot", 0), 1 if p.get("auto") else 0)
        for p in pipes
    )
    blob = (struct.pack("<I", 0x34534E4D) + title + view
            + struct.pack("<i", N) + cells_raw
            + struct.pack("<I", len(cables))
            + b"".join(struct.pack("<6f", *a) for a in cables)
            + struct.pack("<I", len(pipes)) + pipe_blob)
    path.write_bytes(blob)
```

---

## 9. Mesh export (native + HTML)

Native **World** panel: **Export OBJ** / **Export FBX** write under:

```
%LOCALAPPDATA%\CreatingConstructs\Exports\
```

- Formats bake the current static mesh after a forced geometry refresh (empty header-only files are rejected).
- Ghost / non-solid tris (material 10) are skipped.
- OBJ also writes sibling `world.mtl`.
- FBX is ASCII 7.4 (single mesh + normals).

HTML **Export OBJ** / **Export FBX** download the same idea from expanded `InstancedMesh` instances in the browser.

---

## 10. End-to-end: generate a world in code

### 10.1 Recommended pipeline for tools

1. Build a `dict[(x,y,z)] -> (t,r)` for solid architecture.
2. Optionally add `cab` endpoints and `pipes` nodes.
3. Emit **Share JSON** with `stride:64` for HTML / paste interchange.
4. Optionally emit **`world.bin` (MON4)** for native autosave (includes pipes).
5. Optionally emit **`.mns` (MNS4)** for the Saves browser (camera + title + pipes).

Repo examples:

- `tools/make_overgrown_construct.py` — large yard → `.json` + `.mns`
- `tools/tinyworld_to_mns.py` — TinyWorld JSON → `.mns`
- `tools/write_concept_save.py` / `tools/make_reference_atrium.py` — concept saves
- `worlds/*.json`, `Saves/*.mns` — samples

### 10.2 Full generator example

```python
"""Write a tiny brutalist courtyard as Share JSON."""
from pathlib import Path
import json

N = 12
STRIDE, H = 64, 20
cells = {}

def put(x, y, z, t, r=0):
    assert 0 <= x < N and 0 <= z < N and 0 <= y < 20
    cells[x, y, z] = (t, r)

def key(x, y, z):
    return x * (STRIDE * H) + z * H + y

for x in range(N):
    for z in range(N):
        put(x, 0, z, 47)  # Boardform

for z in range(1, 8):
    for y in range(1, 4):
        put(1, y, z, 47)

put(1, 1, 1, 51, 0)  # Portal 2×2 — clear footprint neighbors
for lx, ly in ((1, 0), (0, 1), (1, 1)):
    cells.pop((1 + lx, 1 + ly, 1), None)

put(3, 0, 3, 5)   # Moss
put(3, 1, 3, 14)  # Tree
put(0, 1, 3, 15)  # Ivy
put(4, 0, 4, 94)  # Quadpier (single cell)

pipes = [{
    "cx": 1, "cy": 2, "cz": 4, "face": 0,
    "u": 1, "v": 1, "kind": "pipe", "rot": 0, "auto": False,
}]

world = {
    "N": N,
    "stride": STRIDE,
    "c": [[key(x, y, z), {"t": t, "r": r}]
          for (x, y, z), (t, r) in sorted(cells.items())],
    "cab": [],
    "pipes": pipes,
}

Path("my-world.json").write_text(json.dumps(world, separators=(",", ":")), encoding="utf-8")
print("pieces", len(cells))
```

### 10.3 Design tips for generators

- **Floor first** — walk mode needs continuous solids under the player.
- Prefer **Boardform (47)** over Block (0) for brutalist mass.
- Use **Chase (54)** behind pipe runs.
- Cap piece count for performance (dense nature+ivy on 64² can get heavy).
- Keep ivy (`15`) on cells with a supporting wall behind their facing.
- Water (`40`) is traversable.
- Multi-cell footprints need cleared neighbors; walk-through pieces clear **≥1.6 m** standing height.

---

## 11. Runtime limits (native validation)

| Resource | Limit |
|----------|--------|
| Plot `N` | 4…64 |
| Height | 20 |
| Dense cells | 81920 (64×64×20) |
| Piece type | 0…127 (`PieceCount-1`) |
| Rotation | 0…3 |
| Cables | ≤ 2048 |
| Pipes | ≤ 8000 |
| Named save title | ≤ 63 UTF-16 code units |
| Named viewpoints | ≤ 16; names ≤ 39 UTF-16 code units |
| Material recipe | 9 bits; styles 0…23, scale values 0…2 |
| Door targets | Sparse open entries on Service entry roots (`t=96`) |

Invalid `.mns` / `world.bin` files are rejected without clobbering the open world (named load also writes `recovery.mns` first).

---

## 12. Format cheat sheet

| Format | Magic | Cells | Cables | Pipes | Camera | Best for |
|--------|-------|-------|--------|-------|--------|----------|
| Share JSON | — | sparse keys + `stride` | yes | yes | named `views` (native) | Generators, sharing |
| `world.bin` | `MON4` | dense 64×64×20 | yes | yes | no | Native autosave |
| `.mns` | `MNS4` | dense 64×64×20 | yes | yes | yes | Saves UI presets |
| OBJ / FBX | — | baked mesh | — | — | no | DCC / engine import |

---

## 13. Related source of truth

| Topic | File |
|-------|------|
| Piece names / counts | `src/generated/catalog.inc`, HTML `TN` |
| Sync catalog from HTML | `tools/sync_catalog.py` |
| Catalog audit | `tools/audit_catalog.py` (`--catalog-audit`) |
| JSON export/import (native) | `src/platform/windows.inc` (`exportWorldJson`, `loadWorldJson`) |
| Autosave R/W | `src/main.cpp` (`save`, `load`) |
| Named saves | `src/world/saves.inc` |
| Mesh export | `src/world/export.inc` |
| Native surfaces / extension chunks | `src/world/state.inc` |
| Service-door targets / animation | `src/world/state.inc`, `src/world/doors.inc` |
| Main menu / input routing | `src/platform/ui.inc`, `src/platform/windows.inc`, `src/platform/windows.inc` |
| Menu integration checks | `src/tests/regression.inc` |
| Pipe math | `src/world/pipes.inc`, HTML pipe block |
| Footprints / collision | `src/main.cpp`, `src/world/solidity.inc` |
| Tool notes | `tools/README.md` |
| Sample worlds | `worlds/*.json`, `Saves/*.mns` |

When piece IDs change, bump generators and re-run `tools/sync_catalog.py` so native and HTML stay aligned.


## 14. Native materials, viewpoints, and extension chunks

### 14.1 Surface recipes (`m`)

The Material dock sets the finish for **future placements**, preserving the held piece shape.
It does not paint existing pieces. There is no separate material popup or P shortcut.
Glass, foliage, lamps, and rails retain their specialized materials; eligible structural
surfaces and the service-door leaf receive the chosen finish.

Native Share cells can be written as `[key, {"t":96,"r":0,"m":132,"d":1}]`.
Here `132 = 4 + (0 << 5) + (1 << 7)` selects natural-scale red clay brick.
The piece must still satisfy its footprint and placement requirements.

```python
def material_recipe(style, rotation=0, scale=1):
    assert 0 <= style < 24 and 0 <= rotation < 4 and 0 <= scale < 3
    return 0 if style == 0 else style | (rotation << 5) | (scale << 7)
```

| Bits | Meaning |
|------|---------|
| 0…4 | Style ID, 0…23 |
| 5…6 | Texture rotation, quarter turns 0…3 |
| 7…8 | Pattern frequency: 0 = half / large, 1 = natural, 2 = double / fine |

| ID | Finish | ID | Finish |
|----|--------|----|--------|
| 0 | Original material | 12 | Tudor narrow 1 |
| 1 | Cast concrete | 13 | Tudor narrow 2 |
| 2 | Board-form concrete | 14 | Tudor rising |
| 3 | Aggregate | 15 | Tudor diagonal 1 |
| 4 | Red clay brick | 16 | Tudor diagonal 2 |
| 5 | Mossy red brick | 17 | Tudor falling |
| 6 | Stair brick | 18 | Tudor cross |
| 7 | Arch brick 1 | 19 | Tudor box |
| 8 | Arch brick 2 | 20 | Timber |
| 9 | Arch brick 3 | 21 | Steel |
| 10 | Brick keystone | 22 | Natural stone |
| 11 | Brick mortar | 23 | Soil |

Recipes must be integers in `0…511`, with style < 24 and scale < 3.
Use canonical `0` for the original material. An absent `m` has the same effect.

### 14.2 Named viewpoints (`views`)

Each native JSON viewpoint is a **59-number flat array**:

- Entries 0…39: 40 UTF-16 code units, NUL-padded; entry 39 must be zero.
- Entries 40…56: the 17 floats in SavedView order from section 8.2.
- Entry 57: integer mood, 0…4.
- Entry 58: integer flags, using section 8.2.

At most 16 viewpoints are accepted. Float and flag validation is the same as
`SavedView`. Viewpoints travel with the world and can be opened with V in native.
They are separate from the snapshot's primary SavedView camera.

### 14.3 Binary extension order

All values are little-endian, with **no padding between records**. Native writers
append all three chunks, including zero-count chunks, after the pipe array.

| Chunk | `u32` tag | Header after tag | Record layout | Record size |
|-------|-----------|------------------|---------------|-------------|
| `MAT1` | `0x3154414D` | `u32 count` | `u32 cellIndex`, `u16 recipe` | 6 bytes |
| `VEW1` | `0x31574556` | `u32 count` | 40 UTF-16LE code units, SavedView | 156 bytes |
| `DOR1` | `0x31524F44` | `u32 count` | `u32 cellIndex`, `u8 target` | 5 bytes |

The reader accepts EOF before `MAT1`, after `MAT1`, or after `VEW1`, preserving
older saves. To emit `DOR1`, first emit `MAT1` and `VEW1`, even if their counts are
zero. These are ordered optional tails, not arbitrary tagged chunks: do not
reorder them, insert unknown chunks, or assume a length-prefixed skip mechanism.

`MAT1` rejects duplicate / out-of-range indices, empty-cell owners, invalid recipes,
and counts above 81920. `VEW1` rejects counts above 16, unterminated names, and
invalid SavedView records. `DOR1` validation is described below.

### 14.4 HTML interoperability boundary

Native round-trips `m`, `d`, and `views`. HTML shares the core world fields and
104-piece catalog, but its materials UI, main menu, named viewpoint behavior,
and animated service doors do not implement these native extensions. Cell metadata
may survive HTML JSON handling as object properties; do not rely on HTML to apply
it visually or to preserve top-level `views`. Keep a native snapshot when these
features matter.

## 15. Native interactive service doors

Service entry **96** is in **Detail**, along with Air vent 97, Cable tray 98,
Ceiling light 99, Exit sign 100, Neon cyan 101, Neon rose 102, and Terminal 103.
There is no separate Liminal category. The palette has seven categories.

Service entry reserves a **1×1×2** footprint. Its steel frame is static; the leaf,
inset panels, and handles rotate through 90 degrees with smoothstep easing
`f*f*(3-2*f)`. Doorway 23 and Portal 51 remain open architectural frames.

F or left click while walking toggles an aimed door within **2.6 metres**;
gamepad X does the same. The closed leaf blocks movement. The collider and
occlusion follow the moving leaf; an obstructed closing swing reverses to open.
The leaf and hardware remain inside the reserved footprint in all four rotations.

### Authored state

- Native Share cell `d` is integer `0` (closed) or `1` (open); absence means closed.
- `d:1` requires a Service entry root with `t:96`. Do not attach open states to its
  reserved upper cell or another piece type.
- `DOR1` stores only open targets: each index must be unique, below 81920, own a
  `t:96` cell, and have target byte **1**. Its count may not exceed 81920.
- Autosave, named saves, Share JSON, and undo / redo preserve target state.
- Transient hinge fractions are not serialized. Loading reconstructs the authored
  open / closed pose; there is no half-open save value or level-specific script.

GPU cell bits **15…23** contain the material recipe; **24…30** contain the quantized
live hinge fraction, 0…127. This packing is renderer state, not the binary Cell
record: `Cell` remains two bytes. Mesh export bakes the current door pose.

`Saves/door-workshop.mns` is an ordinary editable example generated by
`tools/make_door_workshop.py`.

## 16. Native main menu and runtime entry points

### 16.1 Main / pause menu

A normal desktop launch opens the main menu over the current world. Its five
entries are Play / continue, Saved worlds, Settings, Controls, and Save and quit.
Escape opens it during play and resumes from its home page. Back / Escape from
Settings, Controls, or the menu's save browser returns to the menu. Loading a
selected save enters the game.

The menu clears held movement / placement input, releases drag capture, hides the
editor panels, and pauses first-person physics and door animation. The rendered
world and ambience remain behind it. Menu state is transient; it is not part of
`world.bin`, `.mns`, or Share JSON.

Main-menu keyboard navigation uses Up / Down and Enter. Gamepad Start opens or
resumes; D-pad and A select home-page entries, and B returns to the menu home page.
Settings uses the existing audio mixer, camera settings, resolution selector, and
fullscreen control. Audio / camera settings are saved when leaving menu settings
or exiting normally. Fullscreen remains available with F11.

### 16.2 Snapshot launch flags

Paths containing spaces must be quoted **after the equals sign** in the actual
Windows command line, for example `--review-save="N:\Monolith maker\Saves\reference-atrium.mns"`.

| Flag | Behavior |
|------|----------|
| `--open-save="path.mns"` | Back up the current world, load the snapshot, bypass the startup menu; normal exit saves into the player profile. |
| `--review-save="path.mns"` | Load an isolated review session using `review-world.bin` and `ReviewSaves`; normal exit does not autosave over the player world. |
| `--capture --fixed-frame --capture-save="path.mns"` | Capture a snapshot render and exit after six frames; no player autosave on exit. |
| `--benchmark --fixed-frame --capture-save="path.mns"` | 360-frame snapshot benchmark; 60 warmup and 300 measured frames; no player autosave on exit. |
| `--validate` | Enable Vulkan validation when available. |
| `--smoke-test --validate --verify-blur` | Use isolated `test-world.bin` / `TestSaves`; run menu, save, door, and existing regression checks for 250 frames. |
| `--catalog-audit --catalog-contracts --audit-dir="folder"` | Generate catalog previews and contract audit results. |
| `--generate=<0…4> [--seed=<n>] [--plot=<4…64>]` | Open an isolated review session (`review-world.bin` / `ReviewSaves`) on a generated construct (section 16.4); combine with `--capture --fixed-frame` for a six-frame still. The player autosave is never written. |

Direct snapshot, review, capture, benchmark, audit, and audio-test launches bypass
the ordinary startup menu. Escape can still open the menu in an interactive review.

### 16.3 QA previews

`--smoke-test --menu-review-images` writes app-rendered menu / settings previews
under `%TEMP%\CreatingConstructsMenuReview`. Add `--door-review-images` for door
phase captures under `%TEMP%\CreatingConstructsDoorReview`.

For a menu preview over an ordinary saved level, combine
`--review-save="path.mns" --menu-review-images --validate`. This opens the menu,
renders main / settings / saved-world previews, and exits after 15 frames in the
isolated review session. Menu previews composite the Vulkan readback with the same
native panel painter; separate Windows edit controls are not included in these QA
images.

### 16.4 Native build tools and construct generators

These are editor features. They change the authored world only through the same
placement rules (`canPlace`, footprints, surfaces, door targets), so everything they
produce round-trips through MON4 / MNS4 / Share JSON unchanged.

| Tool | Keys | Behavior |
|------|------|----------|
| Box | B (Brush → Box → Select) | Two clicks pick opposite corners; the volume fills with the selected piece and rotation, or erases every piece touching it with Shift / X / Erase. One undo step. |
| Select → Stamp | B to Select, Ctrl+V to re-enter Stamp | Two clicks copy roots, finishes (`m`) and door targets (`d`) into a clipboard; Stamp places the copy with its minimum corner at the aimed cell. R turns the copy in quarter steps; multi-cell footprints re-anchor so covered cells stay consistent. Esc leaves Stamp. |
| Paint | P (switches to the Material tab) | Click or drag applies the chosen finish recipe to existing pieces; *Original material* clears it. |
| Mirror | N | Off / X / Z / X+Z about the plot centre. Mirrored placements get mirrored rotations (`r → (4-r)%4` across X, `(6-r)%4` across Z); mirrored erase and paint follow the same cells. |
| Build plane | Page Up / Page Down / Home | A grid at layer `y` intercepts the pick ray inside the plot, so pieces land on that layer in open air. Occupied plane cells report the existing piece. |
| Day cycle | K | One full day every four minutes; `sunAngle` / `sunHeight` follow the arc and `night` toggles after dusk. Not persisted. |
| Turntable | O | Slowly orbits the build camera for showcases. |
| Torch | T (walk mode, night) | Borrows fixture slot 15 for a light ahead of the walker; releasing it rebuilds the fixture list. |

Generators (G, or World → *Generate a construct*) replace the plot with a seeded
construct and record one undo step. Plots smaller than 12 widen to 12 tiles.

| Generator | Content |
|-----------|---------|
| Brutalist tower | Setback board-form core, pilotis base, ribbon windows, cantilevers, roof pergola and amber light, plinth plate and planting |
| Monastery cloister | Slit outer wall with a doorway, columned arcade with arches, slab roof and pendants, courtyard pool, birches, benches, planters |
| Terraced hillside | Stepped aggregate terraces, random stairs between steps, grass / moss / shrubs / pines / boulders / logs, a still basin at the foot |
| Ruined colonnade | Broken columns and beams, half walls with ivy, moss, grass, boulders, logs, mushrooms, perimeter trees |
| Megastructure deck | Pilotis under a waffle deck, 2×2 lightwells, cores with bridges and floor plates, deck railings and lights, moss / grass / puddles below |

**Part files.** With a region copied (Select tool, or Import part), *Export part* writes
`Exports\<id>.part.json`: ordinary Share JSON whose keys are offsets from the region's
minimum corner, with `m` finishes and `d` door targets, plus an informational
`"part":{"w","h","d"}` object that loaders ignore. *Import part* opens any `.json` /
`.txt` Share world or part file into the clipboard and enters Stamp mode, so parts travel
between players and also load as small worlds in the HTML sandbox. The Share dialog's
*Save file* / *Open file* write and read whole worlds as `Exports\<id>.world.json`.

**Imported props.** Pieces 105–107, 109–112, 115 and 122–126 are Retro Official models
(OFFICE and URBAN packs) converted by `tools/prepare_props.py` into `src/generated/prop_models.inc`
and 128×128 texture layers in `prop_textures.inc`; material ids `PropMaterialBase + n` (59…)
double as texture-array layers, and tileable surfaces (carpet, plaster, pool tile, grating,
metal, office door) use planar cell UVs. `tools/embed_props.py` packs the same meshes and
textures into the web build. `assets/props/manifest.json` records sources and hashes.

The same seed and plot size always produce the same construct; *Shuffle seed* draws a
new one. `--smoke-test` validates every generator at 12 and 24 tiles, mirror
re-anchoring of the 2×2 doorway, box fill / erase counts, stamp rotation, paint,
build-plane picking, the day cycle and the torch.

## 17. Validation and authored examples

- `py -3 tools/verify_world_contract.py`: catalog, footprint, and documentation checks.
- `Play/Check-Game.cmd`: native smoke checks followed by the full catalog audit.
- Smoke checks cover menu click routing, keyboard navigation, input / gravity pause,
  save loading from the menu, real mixer sliders, and pause / resume, alongside
  door rotations, sprint blocking, obstruction reversal, state round-trips,
  live Vulkan poses, and the build tools / generators of section 16.4.
- `tools/make_test_garden.py` generates editable part / rotation, material, neighbor
  adaptation, and liminal service-court saves.
- `tools/make_reference_atrium.py` and `tools/make_door_workshop.py` generate normal
  snapshots; showcase layout belongs in saves, not special renderer logic.

Headless or contract checks alone do not certify every visual adaptation, physical
monitor presentation, or hardware/controller configuration.
