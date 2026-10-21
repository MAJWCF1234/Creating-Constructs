"""Author an editable service-door workshop using the ordinary MNS4 save format."""
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'output' / 'doors'
OUT.mkdir(parents=True, exist_ok=True)
cells = bytearray([255, 0] * (64*64*20))

def put(x, y, z, kind=0, rotation=0):
    index = x*1280+z*20+y
    cells[index*2:index*2+2] = bytes((kind, rotation))

for x in range(2, 10):
    for z in range(2, 11):
        put(x, 0, z, 34)
for x in range(2, 10):
    for y in range(1, 4):
        if x != 5 or y == 3:
            put(x, y, 6, 47)
put(5, 1, 6, 96)
for z in range(7, 11):
    for y in range(1, 4):
        put(2, y, z, 47)
        put(9, y, z, 47)
for x in range(2, 10):
    for z in range(7, 11):
        put(x, 4, z, 34)
put(5, 3, 8, 99)
put(8, 2, 7, 97, 1)

def camera(player, yaw, target=(5.5, 2, 6.5), walk=True):
    values = [*target, *player, .75, 1.05, 8, yaw, 0, 68, 32, 205, .92, .18, .5]
    return struct.pack('<17fiI', *values, 2, 4 | (8 if walk else 0))

views = [('Front - F opens door', camera((5.5, 1, 3.7), 3.141592654)),
         ('Back - F closes door', camera((5.5, 1, 8.5), 0)),
         ('Door and wall overview', camera((5.5, 1, 3.7), 0, walk=False))]
title = 'Door workshop - open and close'
data = struct.pack('<I', 0x34534e4d) + title.encode('utf-16-le').ljust(128, b'\0') + views[0][1]
data += struct.pack('<i', 12) + cells + struct.pack('<II', 0, 0)
data += struct.pack('<II', 0x3154414d, 0)
data += struct.pack('<II', 0x31574556, len(views))
data += b''.join(name.encode('utf-16-le')[:78].ljust(80, b'\0') + view for name, view in views)
data += struct.pack('<II', 0x31524f44, 0)
path = OUT / 'door-workshop.mns'
path.write_bytes(data)
(OUT/'README.md').write_text('Load Door workshop - open and close from Saves. Aim at the door and press F or left click. Gamepad X also operates doors. Viewpoints switches to the other side. The entire workshop is ordinary editable geometry.\n')
print(path)
