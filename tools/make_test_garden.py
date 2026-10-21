"""Author ordinary editable MNS4 test gardens; no scene-specific engine behavior."""
from pathlib import Path
import json, math, struct, re

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'output' / 'test-garden'
OUT.mkdir(parents=True, exist_ok=True)
CELLS = 64 * 64 * 20

class Garden:
    def __init__(self):
        self.cells = bytearray([255, 0] * CELLS)
        self.materials = {}
        self.pipes = []
        self.cables = []
        self.stations = []
        self.views = []

    def put(self, x, y, z, part=0, rotation=0, finish=0):
        assert 0 <= x < 64 and 0 <= z < 64 and 0 <= y < 20
        i = x * 1280 + z * 20 + y
        self.cells[i*2:i*2+2] = bytes([part, rotation])
        if finish:
            self.materials[i] = finish

    def floor(self, x0, x1, z0, z1):
        for x in range(x0, x1):
            for z in range(z0, z1):
                self.put(x, 0, z, 34)

    def camera(self, name, target, theta=.7, phi=.9, distance=22):
        values = [*target, target[0], 1, target[2]+4, theta, phi, distance, 0, 0, 65, 28, 205, .92, .12, .45]
        view = struct.pack('<17fiI', *values, 2, 4)
        self.views.append((name, view))
        return view

    def write(self, name, title):
        view = self.views[0][1]
        blob = struct.pack('<I', 0x34534e4d) + title.encode('utf-16-le')[:126].ljust(128, b'\0') + view
        blob += struct.pack('<i', 64) + self.cells
        blob += struct.pack('<I', len(self.cables)) + b''.join(struct.pack('<6f', *c) for c in self.cables)
        blob += struct.pack('<I', len(self.pipes)) + b''.join(struct.pack('<9b', *p) for p in self.pipes)
        blob += struct.pack('<II', 0x3154414d, len(self.materials))
        blob += b''.join(struct.pack('<IH', k, v) for k, v in sorted(self.materials.items()))
        blob += struct.pack('<II', 0x31574556, len(self.views))
        blob += b''.join(n.encode('utf-16-le')[:78].ljust(80, b'\0') + v for n, v in self.views)
        path = OUT / (name + '.mns')
        path.write_bytes(blob)
        (OUT / (name + '-stations.json')).write_text(json.dumps(self.stations, indent=2))
        print(path, 'pieces', sum(self.cells[i] != 255 for i in range(0, len(self.cells), 2)))

# One station for every catalog part in all four rotations. Wide pieces rotate
# into their reserved pad, keeping the circulation lanes clear.
g = Garden()
g.floor(0, 64, 0, 64)
catalog=(ROOT/'src/catalog.inc').read_text()
part_count=int(re.search(r'PieceCount=(\d+)',catalog).group(1))
for part in range(part_count):
    for rotation in range(4):
        index = part * 4 + rotation
        x, z = 2 + index % 20 * 3, 2 + index // 20 * 3
        g.put(x, 1, z, part, rotation)
        if part in (15, 77, 82):
            dx, dz = ((0, -1), (-1, 0), (0, 1), (1, 0))[rotation]
            g.put(x+dx, 1, z+dz)
        if part in (87, 90, 92):
            g.put(x, 2, z, 34)
        if part in (88, 89):
            g.put(x, 0, z, 4, rotation)
        if part == 91:
            g.put(x, 0, z, 1)
        if part == 16:
            g.cables.append((x-.3, 2, z+.5, x+1.3, 2.4, z+.5))
        if 43 <= part <= 46:
            g.put(x, 1, z)
            g.pipes.append((x, 1, z, rotation, 1, 1, part-43, rotation, 1))
        g.stations.append(dict(part=part, rotation=rotation, position=[x, 1, z]))
g.camera('All parts - overview', (31, 1, 30), distance=68)
for i, title in enumerate(('Structure and openings', 'Detail and water', 'Nature and new parts')):
    g.camera(title, (30, 2, 9+i*21), distance=28)
g.write('test-garden-parts', 'Test garden - all parts and rotations')

# Materials are shown on vertical walls, floors, slopes and real stair shapes.
m = Garden()
m.floor(1, 43, 1, 37)
for style in range(24):
    x, z = 3 + style % 6 * 6, 4 + style // 6 * 8
    recipe = style+128 if style else 0
    for xx in range(x, x+3):
        for y in range(1, 4):
            m.put(xx, y, z+3, 0, finish=recipe)
        m.put(xx, 1, z, 34, finish=recipe)
    m.put(x, 1, z+1, 4, finish=recipe)
    m.put(x+2, 1, z+1, 6, finish=recipe)
    m.stations.append(dict(surface=style, position=[x, 1, z]))
m.camera('Materials - overview', (21, 2, 18), distance=48)
for row in range(4):
    m.camera('Material row '+str(row+1), (20, 2, 7+row*8), theta=.2, phi=1.15, distance=28)
m.write('test-garden-materials', 'Test garden - surface materials')

# Deliberate adjacent/stacked cases make adaptations easy to revisit in play.
a = Garden()
a.floor(1, 40, 1, 40)
types = [6, 36, 78, 15, 77, 82, 87, 88, 89, 90, 91, 92, 17, 24, 40, 63, 64, 22, 23, 51, 59]
for index, part in enumerate(types):
    x, z = 4 + index % 5*7, 5 + index//5*8
    for dx in (0, 1):
        a.put(x+dx, 1, z, part)
    if part in (15, 77, 82):
        for y in range(1, 5):
            a.put(x, y, z, part)
            a.put(x, y, z-1)
    if part in (87, 90, 92):
        for dx in (0, 1):
            a.put(x+dx, 2, z, 34)
            a.put(x+dx, 0, z, part)
    if part in (88, 89):
        for dx in (0, 1): a.put(x+dx, 0, z, 4)
    if part == 91:
        for dx in (0, 1): a.put(x+dx, 0, z, 1)
    if part in (23, 51, 59):
        # Two-wide footprints cannot be authored one cell apart.
        a.put(x+1, 1, z, 255)
    a.stations.append(dict(part=part, position=[x, 1, z], setup='adjacent or stacked'))
for face in range(6):
    x, z = 4+face*5, 39
    a.put(x, 1, z)
    for u, v, kind in ((1,1,0),(0,1,1),(2,1,2),(1,0,3),(1,2,0)):
        a.pipes.append((x, 1, z, face, u, v, kind, 0, 1))
a.camera('Adaptations - overview', (20, 2, 21), distance=48)
for row in range(5):
    a.camera('Adaptation row '+str(row+1), (19, 2, 5+row*8), theta=.2, phi=1.1, distance=24)
a.write('test-garden-adaptations', 'Test garden - neighboring adaptations')

(OUT/'README.md').write_text('''# Editable test gardens

These are ordinary MNS4 saves. Load them through Saved worlds; all geometry remains editable.
Parts contains 104 catalog entries in four rotations. Materials contains 24 finishes on walls,
floor pieces, slopes and stairs. Adaptations contains neighboring and stacked examples plus
pipe connections on all six faces. Named viewpoints jump between rows. Station JSON files
identify each pad. The native catalog checker additionally captures all 21 source plant meshes
and all eight tree selections, which are procedural variants rather than separate saved parts.
''')


l = Garden()
l.floor(2, 24, 2, 24)
for z in range(3, 23):
    for y in range(1, 5):
        l.put(3, y, z, 47)
        l.put(22, y, z, 0)
    for x in range(3, 23): l.put(x, 5, z, 34)
for x in range(4, 22):
    for y in range(1, 5): l.put(x, y, 22, 0)
for x in (7, 12, 17):
    l.put(x, 1, 12, 96)
    l.put(x, 3, 12, 100)
    for z in (5, 10, 15, 20): l.put(x, 4, z, 99)
for z in range(4, 22): l.put(19, 3, z, 98, 1)
for index, part in enumerate((97, 101, 102, 103)):
    x = 6 + index*4
    l.put(x, 2, 21, part)
    l.stations.append(dict(part=part, position=[x, 2, 21]))
l.camera('Liminal service court', (13, 2, 14), theta=.4, phi=1.2, distance=18)
# A saved first-person blue-hour view, with normal mood controls still editable.
view = [13, 2, 14, 13, 1, 5, .4, 1.2, 18, 3.14159265, .02, 70, 11, 205, .92, .16, .55]
l.views.append(('Night service passage', struct.pack('<17fiI', *view, 3, 4|8|1)))
l.write('test-garden-liminal', 'Test garden - liminal service court')
