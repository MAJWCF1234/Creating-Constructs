"""Build one connected, editable reference atrium and reproducible matching viewpoints."""
from pathlib import Path
import math,struct,json,argparse,shutil
R=Path(__file__).resolve().parents[1];OUT=R/'output/reference-atrium';OUT.mkdir(parents=True,exist_ok=True)
STRIDE=64;HEIGHT=20;N=32
cells=bytearray([255,0]*(STRIDE*STRIDE*HEIGHT));pipes=[]
def put(x,y,z,t,r=0):
 assert 0<=x<N and 0<=z<N and 0<=y<HEIGHT
 k=2*(x*STRIDE*HEIGHT+z*HEIGHT+y);cells[k:k+2]=bytes((t&255,r))
def box(x0,x1,y0,y1,z0,z1,t=0,r=0):
 for x in range(x0,x1):
  for z in range(z0,z1):
   for y in range(y0,y1):put(x,y,z,t,r)
# Continuous floor, quiet cast-concrete left wall, tall fins and heavy suspended roof.
box(3,29,0,1,2,31)
box(3,5,1,18,2,30)
for z in (3,10,17,24,29):box(15,17,1,18,z,min(z+1,31))
box(3,17,18,19,2,30)
# Outer giant pilasters: narrow skylit gaps, never a closed box.
for z in (5,14,23):box(23,24,1,18,z,z+1)
box(23,24,18,19,4,29)
# Deep rear portal and its descending diagonal concrete brace.
box(4,7,1,18,28,30);box(13,17,1,18,28,30);box(7,13,16,18,28,30)
for x,y in ((8,14),(9,12),(10,10),(11,8),(12,6)):
 put(x,y,28,68)
# Overhead bridge; its stair mouths stay open.
box(5,24,7,8,19,21,34)
for x in range(5,24):
 if x>7:put(x,8,19,11,0)
 if x<19 or x>21:put(x,8,20,11,2)
for z in range(12,19):
 y=z-11
 for x in range(5,8):put(x,y,z,6,2)
for z in range(21,29):
 y=z-13
 for x in range(19,22):put(x,y,z,6,2)
box(18,25,15,16,29,31,34)
for x in range(18,25):put(x,16,30,11,2)
# Foreground low concrete mass and a rising shoulder beside the narrow garden path.
for z in range(11,16):
 h=4+z-11
 box(11,14,1,h,z,z+1)
 for x in range(11,14):put(x,h,z,4,2)
# Irregular beds sit level with the paving. Leave three-metre circulation lanes.
for x in range(5,29):
 for z in range(3,30):
  planted=(x in (5,6) and z<11) or (11<=x<=14 and (z<10 or 17<=z<=27)) or (18<=x<=28 and z<=17) or (24<=x<=28 and z>=22)
  if not planted:continue
  if (x*31+z*17)%11==0:continue
  put(x,0,z,5)
  k=(x*73+z*47)%17
  # Tall shrubs, ferns and low grass mix at unequal spacings.
  if k in (0,1,2,3):put(x,1,z,86)
  elif k in (4,5,6,7,8):put(x,1,z,76)
  elif k in (9,10,11):put(x,1,z,88)
for x,z in ((13,6),(12,23),(20,5),(26,7),(24,13),(27,24),(22,12),(24,16)):
 put(x,0,z,5);put(x,1,z,14)
# Ivy grows from selected wall seams rather than uniform curtains.
for y,z in ((6,10),(5,10),(4,10),(9,24),(8,24),(7,24)):
 put(5,y,z,15,1)
# Fine service pipes frame the first-person walkway, like the reference downpipe.
for y in range(1,13):
 for v in range(3):pipes.append((4,y,6,0,1,v,0,1,0))
pipes.append((4,0,6,0,1,0,3,1,0))
# Mirror into the reference's handed composition: solid wall on the left of +Z views.
original=bytes(cells);cells=bytearray([255,0]*(STRIDE*STRIDE*HEIGHT))
for x in range(N):
 for z in range(N):
  for y in range(HEIGHT):
   k=2*(x*STRIDE*HEIGHT+z*HEIGHT+y);t,rot=original[k:k+2]
   if t!=255:put(N-1-x,y,z,t,(2-rot)%4 if t==68 else (-rot)%4)
pipes=[(N-1-x,y,z,1 if f==0 else 0 if f==1 else f,2-u,v,k,rot,auto) for x,y,z,f,u,v,k,rot,auto in pipes]
# Select leafy placement rotations for this living garden, retaining the same real tree models.
def tree_variant(x,z,rot):
 seed=(((2*x+1)*73856093)^((2*z+1)*19349663)^(3*83492791)^(rot*2654435761))&0xffffffff
 state=((seed+59)*747796405+2891336453)&0xffffffff
 v=(((state>>((state>>28)+4))^state)*277803737)&0xffffffff
 v=(v>>22)^v
 return int((v&0xffffff)/16777216*8)
for x in range(N):
 for z in range(N):
  k=2*(x*STRIDE*HEIGHT+z*HEIGHT+1)
  if cells[k]==14:
   rot=min(range(4),key=lambda r:tree_variant(x,z,r));put(x,1,z,14,rot)
views={
 'walkway':{'player':(8.6,1,3.8),'direction':(0,1),'pitch':.07,'fov':65},
 'colonnade':{'player':(8.6,1,6.0),'direction':(-.1,1),'pitch':.16,'fov':65},
 'bridge':{'player':(10.0,1,16.5),'direction':(.05,1),'pitch':.48,'fov':70},
 'courtyard':{'player':(21.5,1,9.1),'direction':(-.8,.6),'pitch':.32,'fov':67},
}
def write(path,title,v):
 dx,dz=v['direction'];dx=-dx;yaw=math.atan2(-dx,-dz);player=(N-v['player'][0],v['player'][1],v['player'][2])
 floats=[10,5,17,*player,.8,1.2,20,yaw,v['pitch'],v['fov'],12,200,.82,.34,.48]
 view=struct.pack('<17fiI',*floats,2,4|8)
 blob=struct.pack('<I',0x34534e4d)+title.encode('utf-16-le')[:126].ljust(128,b'\0')+view+struct.pack('<i',N)+cells+struct.pack('<II',0,len(pipes))+b''.join(struct.pack('<9b',*p) for p in pipes)
 path.write_bytes(blob)
for name,v in views.items():write(OUT/(name+'.mns'),'Reference atrium - '+name,v)
primary=OUT/'reference-atrium.mns';write(primary,'Concrete garden - reference atrium',views['walkway'])
(OUT/'views.json').write_text(json.dumps(views,indent=2))
print('Wrote connected atrium:',sum(cells[i]!=255 for i in range(0,len(cells),2)),'pieces;',len(pipes),'pipe nodes')
# Standard Share JSON: the reconstruction consists exclusively of ordinary catalog cells and pipe nodes.
entries=[]
for x in range(N):
 for z in range(N):
  for y in range(HEIGHT):
   index=x*STRIDE*HEIGHT+z*HEIGHT+y;t,rot=cells[2*index:2*index+2]
   if t!=255:entries.append([index,{'t':t,'r':rot}])
kinds=['pipe','valve','gauge','outlet']
serialized=[dict(cx=x,cy=y,cz=z,face=f,u=u,v=v,kind=kinds[k],rot=rot,auto=bool(auto)) for x,y,z,f,u,v,k,rot,auto in pipes]
(OUT/'reference-atrium.json').write_text(json.dumps(dict(N=N,stride=STRIDE,c=entries,cab=[],pipes=serialized),separators=(',',':')))
# GPU light-occlusion fixtures: an entirely sealed room and the same room with a side opening.
scene_cells,scene_pipes=cells,pipes
cells=bytearray([255,0]*(STRIDE*STRIDE*HEIGHT));pipes=[]
box(6,15,0,1,5,15);box(6,15,8,9,5,15)
box(6,7,1,8,5,15);box(14,15,1,8,5,15);box(6,15,1,8,5,6);box(6,15,1,8,14,15)
fixture={'player':(21.5,1,8),'direction':(0,1),'pitch':.1,'fov':65}
write(OUT/'sealed-room.mns','Light test - sealed',fixture)
box(6,7,1,7,7,12,-1)
write(OUT/'window-room.mns','Light test - window',fixture)
cells,pipes=scene_cells,scene_pipes
if '--install' in __import__('sys').argv:
 folder=Path.home()/'AppData/Local/CreatingConstructs/Saves';folder.mkdir(exist_ok=True)
 dest=folder/'reference-atrium.mns'
 if dest.exists():
  i=2
  while (folder/f'reference-atrium-{i}.mns').exists():i+=1
  dest=folder/f'reference-atrium-{i}.mns'
 shutil.copy2(primary,dest);print('Installed',dest)
