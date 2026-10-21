"""Convert a decoded Tiny Brutalism world JSON into a Creating Constructs .mns save."""
from __future__ import annotations

import argparse
import json
import math
import struct
from pathlib import Path

# Creating Constructs dense grid (MNS1 = 16³ plot slab, height always 20)
CC_HEIGHT = 20
MNS1 = 0x31534E4D  # 16-plot named save

# TinyWorld facade block.kind → CC piece id
BLOCK_KIND = {
    1: 71,  # Hooded → Hood
    2: 32,  # Slit
    3: 8,  # Punched → Window
    4: 51,  # Portal
    # 5 Blank — leave underlying solid (no override)
    6: 3,  # Pane → Glass
    7: 21,  # Fins
    8: 84,  # GlassBlock
    9: 85,  # Embrasure
    10: 22,  # Cobogó
    11: 33,  # Ribbon
}

# TinyWorld structural piece.kind → CC piece id (skip PieceNone=1)
PIECE_KIND = {
    2: 6,  # Stair
    3: 4,  # Slope
    4: 68,  # Strut
    5: 2,  # Pillar
    6: 59,  # Bridge
    7: 93,  # SlopeDown → Downslope
    8: 4,  # SlopeUp → Slope
    9: 2,  # PillarOne
    10: 94,  # PillarFour → Quadpier
}

# TinyWorld plant.kind → CC piece id
PLANT_KIND = {
    0: 14,  # Tree
    1: 86,  # Shrub
    2: 76,  # Meadow → Grass
    3: 15,  # Ivy
    4: 77,  # Cascade
    5: 87,  # Hanging → Hang
    6: 82,  # WallMeadow → Green wall
    7: 88,  # SlopeMeadow → Slopegrass
    8: 89,  # WedgeTop → Captop
    9: 90,  # WedgeUnder → Soffitveg
    10: 91,  # BridgeDeck → Deckgrass
    11: 92,  # BridgeHang → Dropveg
}

SOLID = 47  # Boardform — béton mass


def face_rot(axis: int, sign: int) -> int | None:
    """Map TinyWorld face normal → CC yaw so the open/−Z mesh side is flush."""
    if axis == 0:
        return 3 if sign < 0 else 1  # opening −X / +X
    if axis == 2:
        return 2 if sign < 0 else 0  # opening −Z / +Z
    return None  # vertical / unknown


def slope_rot(axis: int, sign: int) -> int:
    if axis == 0:
        return 1 if sign > 0 else 3
    if axis == 2:
        return 0 if sign > 0 else 2
    return 0


def idx(x: int, y: int, z: int, plot: int) -> int:
    return x * (plot * CC_HEIGHT) + z * CC_HEIGHT + y


def put(cells: bytearray, plot: int, x: int, y: int, z: int, t: int, r: int = 0) -> None:
    if not (0 <= x < plot and 0 <= z < plot and 0 <= y < CC_HEIGHT):
        return
    i = idx(x, y, z, plot) * 2
    cells[i] = t & 0xFF
    cells[i + 1] = r & 3


def pack_view(world: dict) -> bytes:
    cam = world.get("camTarget") or {}
    tx = float(cam.get("x", 8.0))
    ty = float(cam.get("y", 2.0))
    tz = float(cam.get("z", 8.0))
    plot = int(world.get("plot") or 16)
    # TinyWorld cam often sits near origin — aim at the slab center from a high aerial.
    if abs(tx) < 2 and abs(tz) < 2:
        tx, ty, tz = plot * 0.5, 4.5, plot * 0.5
    player = (tx, max(1.0, ty), tz)
    dist = float(world.get("camDistance") or 32.0)
    dist = min(70.0, max(4.0, dist if dist >= 4 else 32.0))
    pitch = float(world.get("camPitch") or 42.0)
    # Degrees → radians when clearly not already radians.
    phi = math.radians(pitch) if abs(pitch) > 2.5 else abs(pitch)
    phi = min(1.55, max(0.1, phi))
    yaw = float(world.get("camYaw") or 0.0)
    # Collapse huge wrapped yaw into a build-orbit theta.
    theta = math.radians(yaw % 360.0) if abs(yaw) > 6.3 else yaw
    sun_h = float(world.get("sunElevation") or 45.0)
    sun_a = float(world.get("sunAzimuth") or 90.0)
    fog = float(world.get("fog") if world.get("fog") is not None else 0.62)
    if fog < 0 or fog > 1:
        fog = 0.62  # TinyWorld uses -1 as “default” — misty aerial
    clouds = float(world.get("clouds") if world.get("clouds") is not None else 0.65)
    if clouds < 0 or clouds > 1:
        clouds = 0.65
    night = 1 if world.get("night") else 0
    wind = 16 if world.get("wind") else 0
    cloud_bits = max(0, min(100, int(clouds * 100 + 0.5))) << 8
    flags = night | 4 | wind | cloud_bits  # AO on
    floats = [
        tx, ty, tz,
        *player,
        theta,
        phi,
        dist,
        0.0,
        0.0,
        55.0,
        sun_h,
        sun_a,
        1.15,
        fog,
        0.0,
    ]
    return struct.pack("<17f i I", *floats, 0, flags)


def convert(world: dict) -> tuple[bytearray, int, dict]:
    plot = int(world["plot"])
    if plot not in (16, 32, 64):
        raise SystemExit(f"unsupported plot size {plot}")
    raw = world["cells"]
    height = len(raw) // (plot * plot)
    cells = bytearray([0xFF, 0] * (plot * plot * CC_HEIGHT))
    stats = {
        "solid": 0,
        "blocks": 0,
        "pieces": 0,
        "plants": 0,
        "lights": 0,
        "skipped_plants": 0,
    }

    for y in range(height):
        for z in range(plot):
            for x in range(plot):
                if raw[(y * plot + z) * plot + x] == "1":
                    put(cells, plot, x, y, z, SOLID, 0)
                    stats["solid"] += 1

    for b in world.get("blocks") or []:
        c = b["cell"]
        t = BLOCK_KIND.get(int(b["kind"]))
        if t is None:
            continue
        r = face_rot(int(b["axis"]), int(b["sign"]))
        if r is None:
            r = 0
        put(cells, plot, int(c["x"]), int(c["y"]), int(c["z"]), t, r)
        stats["blocks"] += 1

    for p in world.get("pieces") or []:
        kind = int(p["kind"])
        t = PIECE_KIND.get(kind)
        if t is None:
            continue
        a = p["air"]
        r = slope_rot(int(p["axis"]), int(p["sign"]))
        put(cells, plot, int(a["x"]), int(a["y"]), int(a["z"]), t, r)
        stats["pieces"] += 1

    for p in world.get("plants") or []:
        t = PLANT_KIND.get(int(p["kind"]))
        if t is None:
            continue
        c = p["cell"]
        x, y, z = int(c["x"]), int(c["y"]), int(c["z"])
        if y < 0:
            y = 0
        if not (0 <= x < plot and 0 <= z < plot and 0 <= y < CC_HEIGHT):
            stats["skipped_plants"] += 1
            continue
        r = face_rot(int(p["axis"]), int(p["sign"]))
        put(cells, plot, x, y, z, t, 0 if r is None else r)
        stats["plants"] += 1

    for L in world.get("lights") or []:
        c = L.get("cell") or {}
        x, y, z = int(c.get("x", 0)), int(c.get("y", 0)), int(c.get("z", 0))
        n = L.get("normal") or {}
        nx, ny, nz = float(n.get("x", 0)), float(n.get("y", 0)), float(n.get("z", 0))
        kind = int(L.get("kind") or 0)
        if L.get("ground") or (abs(ny) > abs(nx) and abs(ny) > abs(nz)):
            t, r = 13, 0  # freestanding strip Light
        elif kind == 1:
            # Spot → Angle (downward wall grazelight), flush to the hit face
            t = 95
            if abs(nx) >= abs(nz):
                r = 1 if nx > 0 else 3
            else:
                r = 0 if nz > 0 else 2
        else:
            t = 27  # Bar → Sconce
            if abs(nx) >= abs(nz):
                r = 1 if nx > 0 else 3
            else:
                r = 0 if nz > 0 else 2
        put(cells, plot, x, y, z, t, r)
        stats["lights"] += 1

    return cells, plot, stats


def write_mns(path: Path, title: str, cells: bytearray, plot: int, world: dict) -> None:
    if plot != 16:
        raise SystemExit("this writer currently emits MNS1 (16-plot) only")
    title_buf = title.encode("utf-16-le")[:126].ljust(128, b"\0")
    blob = b"".join(
        [
            struct.pack("<I", MNS1),
            title_buf,
            pack_view(world),
            struct.pack("<i", plot),
            bytes(cells),
            struct.pack("<I", 0),  # no cables (TinyWorld cable packing differs)
        ]
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(blob)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "json",
        nargs="?",
        default=r"C:\Users\majwc\Downloads\tinyworld_deconstruction\decoded\neighborhood.json",
    )
    ap.add_argument("-o", "--out-name", default="")
    args = ap.parse_args()
    src = Path(args.json)
    data = json.loads(src.read_text(encoding="utf-8"))
    world = data["world"] if "world" in data else data
    cells, plot, stats = convert(world)
    filled = sum(1 for i in range(0, len(cells), 2) if cells[i] != 0xFF)
    title = args.out_name or str(world.get("name") or src.stem)
    title = title[:63]
    slug = "".join(ch if ch.isalnum() or ch in "-_" else "-" for ch in title).strip("-") or "tinyworld"
    roots = [
        Path(r"N:\Monolith maker\Saves"),
        Path.home() / "AppData/Local/CreatingConstructs/Saves",
    ]
    for root in roots:
        out = root / f"tiny-{slug}.mns"
        write_mns(out, title, cells, plot, world)
        print(f"wrote {out} ({out.stat().st_size} bytes, {filled} cells)")
    print("stats", stats)


if __name__ == "__main__":
    main()
