"""Write the concept-art béton map as ONE save with TWO distinct zones (not blended)."""
from __future__ import annotations
import struct
from pathlib import Path

MaxPlot, WorldHeight = 32, 20
WorldCells = MaxPlot * MaxPlot * WorldHeight
MAGIC = 0x32534E4D  # MNS2

BLOCK, SLOPE, PILLAR = 0, 4, 2
MOSS, IVY, PLANTER = 5, 15, 17
SLIT = 32
BOARDFORM, PLINTH, AGGREGATE = 47, 53, 61
STRUT, GIRDER, NICHE, HOOD = 68, 69, 70, 71


def idx(x: int, y: int, z: int) -> int:
    return x * (MaxPlot * WorldHeight) + z * WorldHeight + y


def put(cells: bytearray, x: int, y: int, z: int, t: int, r: int = 0) -> None:
    i = idx(x, y, z) * 2
    cells[i] = t & 0xFF
    cells[i + 1] = r & 0xFF


def clear(cells: bytearray, x: int, y: int, z: int) -> None:
    i = idx(x, y, z) * 2
    cells[i] = 0xFF
    cells[i + 1] = 0


def fill_box(cells: bytearray, x0: int, x1: int, y0: int, y1: int, z0: int, z1: int, t: int, r: int = 0) -> None:
    for x in range(x0, x1):
        for y in range(y0, y1):
            for z in range(z0, z1):
                put(cells, x, y, z, t, r)


def moss_bed(cells: bytearray, x0: int, x1: int, z0: int, z1: int) -> None:
    for z in range(z0, z1):
        for x in range(x0, x1):
            put(cells, x, 0, z, PLANTER if (x + z) % 2 == 0 else MOSS)


def build_zone_corridor(cells: bytearray) -> None:
    """Zone A — first concept shot: long hall, thin right fins, mid portal + strut."""
    h = 11
    z0, z1 = 1, 14
    wx0, wx1 = 3, 6
    fin_x = 8
    portal_z = 8

    for z in range(z0, z1):
        for x in range(wx0, wx1):
            put(cells, x, 0, z, BLOCK)
    for x in range(wx0, wx1):
        put(cells, x, 0, z0, PLINTH)

    moss_bed(cells, 2, 3, z0, z1)
    moss_bed(cells, 6, 7, z0, z1)
    for xz in ((3, 3), (5, 5), (3, 9), (5, 11), (6, 7)):
        put(cells, xz[0], 0, xz[1], MOSS)

    # left mass
    fill_box(cells, 0, 2, 0, h, z0, z1, BOARDFORM)
    fill_box(cells, 1, 2, 0, h, z0, z1, AGGREGATE)
    for z in (3, 6, 10):
        for y in range(2, 7):
            clear(cells, 1, y, z)
            put(cells, 1, y, z, SLIT)
    put(cells, 1, 4, 3, IVY)
    put(cells, 1, 5, 3, IVY)

    # mid portal + strut
    fill_box(cells, 0, fin_x, 0, h, portal_z, portal_z + 1, BOARDFORM)
    ox0, ox1 = 2, 7
    for y in range(1, h):
        for x in range(ox0, ox1):
            clear(cells, x, y, portal_z)
    fill_box(cells, ox0 - 1, ox0, 0, h, portal_z, portal_z + 1, AGGREGATE)
    fill_box(cells, ox1, ox1 + 1, 0, h, portal_z, portal_z + 1, AGGREGATE)
    put(cells, ox0, h - 1, portal_z, GIRDER, 0)
    clear(cells, ox0 + 1, h - 1, portal_z)
    for x in range(ox0 + 2, ox1):
        put(cells, x, h - 1, portal_z, BOARDFORM)
    put(cells, ox1 - 1, h - 3, portal_z, STRUT, 0)
    clear(cells, ox1 - 1, h - 2, portal_z)

    # thin right fins (NOT niche piers — those live in zone B)
    for pz in (2, 5, 9, 12):
        for y in range(h):
            put(cells, fin_x, y, pz, BOARDFORM)
        put(cells, fin_x, 0, pz, PLINTH)
        put(cells, fin_x, h - 1, pz, SLOPE, 1)
        if pz in (5, 12):
            for y in range(h):
                put(cells, fin_x + 1, y, pz, PILLAR)

    # ceiling over zone A only
    for z in range(z0, z1):
        for x in range(0, fin_x + 1):
            put(cells, x, h, z, BOARDFORM)
    put(cells, ox0, h, portal_z, GIRDER, 0)
    clear(cells, ox0 + 1, h, portal_z)
    clear(cells, ox1 - 1, h - 2, portal_z)


def build_zone_niches(cells: bytearray) -> None:
    """Zone B — second concept shot: pierced niche colonnade + hoods (separate wing)."""
    h = 10
    # Sit this wing past the corridor, same plot — connected by a short walk spur
    # Layout: walk spur along z=16..22 at x=4..6, pier row at x=10 facing −X
    z0, z1 = 16, 24
    wx0, wx1 = 4, 7
    pier_x = 10

    # connector from corridor into this wing
    for z in range(13, z0 + 1):
        for x in range(wx0, wx1):
            put(cells, x, 0, z, BLOCK)

    for z in range(z0, z1):
        for x in range(wx0, wx1):
            put(cells, x, 0, z, BLOCK)
    moss_bed(cells, 3, 4, z0, z1)
    moss_bed(cells, 7, 9, z0, z1)
    for z in range(z0, z1):
        put(cells, 8, 0, z, MOSS)
        if z % 2 == 0:
            put(cells, 9, 0, z, MOSS)

    # left solid wall for this wing (foreground mass in the niche shot)
    fill_box(cells, 0, 3, 0, h, z0, z1, BOARDFORM)
    fill_box(cells, 1, 2, 0, h, z0, z1, AGGREGATE)

    # three pierced piers, 1-cell gaps — Hood replaces Niche on outer faces (same cell, no float)
    pier_zs = (17, 19, 21)
    for i, pz in enumerate(pier_zs):
        fill_box(cells, pier_x, pier_x + 2, 0, h, pz, pz + 1, BOARDFORM)
        put(cells, pier_x, 0, pz, PLINTH)
        put(cells, pier_x + 1, 0, pz, PLINTH)
        hooded = i >= 1  # mid + far: projecting hooded niches; near: flush niches
        for y in (2, 4, 6, 8):
            put(cells, pier_x, y, pz, HOOD if hooded else NICHE, 1)  # r=1 → open toward walk (−X)
        put(cells, pier_x, h - 1, pz, SLOPE, 1)
        put(cells, pier_x + 1, h - 1, pz, SLOPE, 1)

    # ceiling over niche wing (stops at pier front so light pours from +X void)
    for z in range(z0, z1):
        for x in range(0, pier_x + 1):
            put(cells, x, h, z, BOARDFORM)


def build_map() -> bytearray:
    cells = bytearray([0xFF, 0] * WorldCells)
    build_zone_corridor(cells)
    build_zone_niches(cells)
    return cells


def pack_view() -> bytes:
    # Start aimed at Zone A corridor; player can walk +Z into Zone B
    target = (5.0, 3.2, 7.0)
    player = (4.5, 1.0, 2.5)
    floats = [
        *target,
        *player,
        0.08,
        1.2,
        14.0,
        0.0,
        0.0,
        52.0,
        36.0,
        88.0,
        1.5,
        0.78,
        0.0,
    ]
    blob = struct.pack("<17f i I", *floats, 4, 4)
    assert len(blob) == 76
    return blob


def write_mns(path: Path, title: str) -> int:
    cells = build_map()
    pieces = sum(1 for i in range(0, len(cells), 2) if cells[i] != 0xFF)
    title_buf = title.encode("utf-16-le")[:126].ljust(128, b"\0")
    n = 24  # room for corridor (z~1–13) + niche wing (z~16–23)
    blob = b"".join(
        [
            struct.pack("<I", MAGIC),
            title_buf,
            pack_view(),
            struct.pack("<i", n),
            bytes(cells),
            struct.pack("<I", 0),
        ]
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(blob)
    return pieces


def main() -> None:
    roots = [
        Path(r"N:\Monolith maker\Saves"),
        Path.home() / "AppData/Local/CreatingConstructs/Saves",
    ]
    title = "Concept béton map"
    for root in roots:
        out = root / "concept-beton-corridor.mns"
        pieces = write_mns(out, title)
        print(f"wrote {out} ({pieces} pieces, {out.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
