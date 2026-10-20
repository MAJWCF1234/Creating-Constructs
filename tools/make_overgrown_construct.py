"""Author a new named save. Never opens or overwrites the current autosave."""
from pathlib import Path
import struct,json,hashlib
R=Path(__file__).resolve().parents[1];cells={};cables=[]
def put(x,y,z,t,r=0):
 assert 0<=x<32 and 0<=z<32 and 0<=y<20
 if t<0:cells.pop((x,y,z),None)
 else:cells[x,y,z]=(t,r)
def box(x0,x1,y0,y1,z0,z1,t=0,r=0):
 for x in range(x0,x1+1):
  for y in range(y0,y1+1):
   for z in range(z0,z1+1):put(x,y,z,t,r)
def height(x,z,top):
 # Solid stair support, with an optional half-height final slab.
 for y in range(1,4):put(x,y,z,-1)
 for y in range(1,int(top)):put(x,y,z,0)
 if top%1:put(x,int(top),z,1)
# A raised construct yard with a genuine lower level beneath it.
box(0,15,0,0,0,15)
box(0,15,3,3,0,15)
for x in range(16):box(x,x,1,2,0,0);box(x,x,1,2,15,15)
for z in range(16):box(0,0,1,2,z,z);box(15,15,1,2,z,z)
# Lower gallery: pillars, a tunnel and a darker side chamber.
for x,z in [(3,3),(6,3),(3,6),(6,6),(11,8),(14,8)]:box(x,x,1,2,z,z,7)
box(9,9,1,2,1,6)
box(9,9,1,2,4,5,-1)
box(10,14,1,2,6,6)
box(11,12,1,2,6,6,-1)
# Courtyard lawn islands leave broad traversable concrete walks.
for x in range(1,7):
 for z in range(7,15):
  if not (x>=5 and z<=9):put(x,3,z,5)
for x,z in [(1,8),(4,13),(2,12),(14,8),(13,1)]:
 if (x,z)==(13,1):continue
 put(x,4,z,14)
# West test block: three storeys, open entrance and horizontal window bands.
for y in range(4,15):
 for x in range(0,5):
  for z in range(0,6):
   if x in (0,4) or z in (0,5):put(x,y,z,0)
for y in (7,11,15):box(0,4,y,y,0,5)
# Ground entrance and room windows. Interior remains empty and walkable.
box(2,3,4,5,5,5,-1)
for y in (5,9,13):
 for x in (1,2,3):
  put(x,y,5,3)
  if y>5:put(x,y+1,5,3)
box(2,3,4,5,5,5,-1)
for y in (5,9,13):
 for z in (1,2,3):put(4,y,z,32)
# Vertical climbing route within the west block reaches upper galleries.
for y in range(4,15):put(3,y,1,25,2)
for y in (7,11):put(3,y,1,-1)
# Ivy grows from continuous concrete piers rather than covering every window.
for x in (0,4):
 for y in range(4,15):put(x,y,6,15)
# East low building and its open doorway, roof garden and balcony.
for y in range(4,7):
 for x in range(11,16):
  for z in range(0,6):
   if x in (11,15) or z in (0,5):put(x,y,z,0)
box(11,15,7,7,0,5)
box(12,13,4,5,5,5,-1)
for z in (1,2,3):put(15,5,z,3)
for x in (11,14,15):put(x,6,5,12)
# Broad stair/ramp from the yard to the elevated terrace.
for i,z in enumerate(range(11,3,-1)):
 for x in (9,10):
  top=4.5+i*.5
  for y in range(4,int(top)):put(x,y,z,0)
  if top%1:put(x,int(top),z,1)
# Bridge at terrace level links both buildings.
box(5,10,7,7,2,3,34)
for x in range(5,11):
 put(x,8,2,11,0)
 if x<9:put(x,8,3,11,2)
# Access bridge through west and east walls.
box(4,4,8,9,2,3,-1)
box(11,11,8,9,2,3,-1)
# Roof seating, low planting beds and a sentinel tree.
for x,z in [(12,1),(14,1),(14,4)]:put(x,8,z,17)
put(13,8,1,14);put(12,8,4,39)
for z in range(1,5):put(15,8,z,11,1)
for x in (0,1,3,4):put(x,16,1,17)
put(2,16,2,14)
# Shallow water basin and a half-step exit.
for x in range(10,15):
 for z in range(11,15):
  put(x,2,z,0);put(x,3,z,40)
put(10,3,14,1)
# Basement descent: six half-metre steps, open overhead.
for i,z in enumerate(range(13,7,-1)):
 height(8,z,3.5-i*.5)
# The tunnel landing continues under the upper yard.
for z in (6,7):box(8,8,1,2,z,z,-1)
# Quiet pause points; eight real light sources across the route.
put(5,4,11,39,1);put(6,4,8,38);put(1,4,6,17)
for x,y,z,t,r in [(6,4,14,26,0),(6,4,8,26,0),(15,4,10,26,0),(3,5,6,27,0),(12,5,6,27,0),(3,2,4,13,0),(11,2,3,13,0),(8,2,7,13,0)]:put(x,y,z,t,r)
# Hanging cables add long diagonals between the volumes.
cables=[((4.6,14.7,5.6),(14.3,8.2,5.2)),((4.4,10.7,5.7),(10.2,8.2,10.1)),((.6,15.7,.7),(4.5,15.7,5.5))]
# Expand the compact architectural core into a full 32-square Construct yard.
cells={(x+8,y,z):value for (x,y,z),value in cells.items()}
cables=[((a[0]+8,a[1],a[2]),(b[0]+8,b[1],b[2])) for a,b in cables]
for x in range(32):
 for z in range(32):
  if (x,0,z) not in cells:put(x,0,z,0)
  if not (8<=x<24 and z<16):put(x,3,z,0)
# Closed retaining edges establish an actual lower level below the lawn.
for x in range(32):
 for z in (0,31):
  if 8<=x<24 and z==0:continue
  box(x,x,1,2,z,z)
for z in range(32):
 for x in (0,31):box(x,x,1,2,z,z)
# Broad overgrown west lawn and south field, broken by generous concrete routes.
for x in range(1,8):
 for z in range(5,31):put(x,3,z,5)
for x in range(9,24):
 for z in range(20,31):
  if x not in (15,16,17) and z not in (24,25):put(x,3,z,5)
# A larger water basin forms the eastern edge of the playable courtyard.
for x in range(25,31):
 for z in range(18,29):put(x,2,z,0);put(x,3,z,40)
for z in (20,25):put(25,3,z,1)
# The new trees have breathing room and leave the central sandbox open.
for x,z in [(2,7),(5,13),(2,20),(6,27),(11,23),(12,29),(20,22),(22,29),(27,8),(29,13)]:put(x,4,z,14)
for x,z,r in [(7,19,1),(19,19,0),(24,25,1),(14,27,0)]:put(x,4,z,39,r)
# Incomplete concrete test props on the open construction pad.
for x in range(26,30):put(x,4,5,10)
for x in range(26,30):put(x,5,5,34)
put(27,4,10,28);put(29,4,9,29);put(26,4,13,38)
# Mixed canopy: birch and pine break up the deciduous line (showcase foliage catalog).
for x,z in [(4,9),(3,25),(21,26),(28,22)]:put(x,4,z,41)  # birch
for x,z in [(1,15),(10,28),(23,21),(29,11)]:put(x,4,z,42)  # pine
# Extra ivy sheets on the east low building and courtyard lip.
for y in range(4,7):
 put(15,y,6,15);put(11,y,6,15)
for x in (20,21,22):put(x,4,19,15)
# Soft path lights along the main concrete spine and pool rim.
for x,z in [(16,10),(16,16),(16,22),(24,16),(9,18)]:put(x,4,z,13)
put(28,4,18,27);put(12,4,21,26)
# Face-mounted pipe run on the west test-block south facade (Share JSON / HTML).
# face 4 = +Z exterior; valve + gauge showcase the pipe kit.
pipes=[]
wx,wz,face=10,5,4  # mid south face of west block after +8 expand
for cy,u,v,kind,rot in [
 (4,1,0,'pipe',0),(4,1,1,'pipe',0),(4,1,2,'pipe',0),
 (5,1,0,'pipe',0),(5,1,1,'valve',1),(5,1,2,'pipe',0),
 (6,1,0,'pipe',0),(6,1,1,'gauge',1),(6,1,2,'pipe',0),
 (5,0,1,'pipe',0),(5,2,1,'pipe',0),
]:
 pipes.append({'cx':wx,'cy':cy,'cz':wz,'face':face,'u':u,'v':v,'kind':kind,'rot':rot,'auto':kind in ('valve','gauge')})
name='Overgrown Construct' 
raw=bytearray([-1 & 255,0])*20480
for (x,y,z),(t,r) in cells.items():struct.pack_into('bb',raw,(x*640+z*20+y)*2,t,r)
# SavedView is 2 vec3s, 11 scalar floats, mood int and flags uint (76 bytes).
def pack_view(target,player,theta,phi,dist,yaw,pitch,fov,sun_h,sun_a,exp,fog,sat,mood,flags):
 v=struct.pack('<17fiI',*target,*player,theta,phi,dist,yaw,pitch,fov,sun_h,sun_a,exp,fog,sat,mood,flags)
 assert len(v)==76
 return v
# Play save: first-person courtyard entrance (walk + AO).
play_view=pack_view((16.,7.8,14.5),(15.4,4.,17.4),.72,1.02,48.,0.,-.07,75.,32.,130.,1.06,.16,.08,0,12)
# Overview save: Blue-hour orbit for itch background / marketing stills.
overview_view=pack_view((16.,4.2,15.),(15.4,4.,17.4),.95,1.05,40.,0.,-.05,60.,8.,250.,1.05,.34,.55,3,4)
def write_mns(path,view):
 blob=struct.pack('<I',0x32534e4d)+name.encode('utf-16le').ljust(128,b'\0')+view+struct.pack('<i',32)+raw+struct.pack('<I',len(cables))+b''.join(struct.pack('<6f',*a,*b) for a,b in cables)
 path.write_bytes(blob)
 return blob
blob=write_mns(R/'worlds/Overgrown Construct.mns',play_view)
write_mns(R/'worlds/Overgrown Construct Overview.mns',overview_view)
share={'N':32,'c':[[x*1024+z*32+y,{'t':t,'r':r}] for (x,y,z),(t,r) in cells.items()],'cab':[[*a,*b] for a,b in cables],'pipes':pipes}
(R/'worlds/Overgrown Construct.json').write_text(json.dumps(share,separators=(',',':')))
print(len(cells),'pieces;',len(pipes),'pipes;',len(blob),'bytes; SHA256',hashlib.sha256(blob).hexdigest())
