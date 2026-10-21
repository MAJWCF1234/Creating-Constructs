"""Seal basin wall gaps, fix pipe hit without add-cell, improve rock tex, wet-moss pebbles."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")
changed = []

# --- Basin walls: full-length sides + better corners + rim flush ---
old_basin = """function emitIsland(group,type){
 const occ=new Set(group.map(([x,y,z])=>`${x},${y},${z}`)),rim=.38;
 for(const[x,y,z] of group){
  const g=new THREE.Group();g.position.set(x+.5,y+.5,z+.5);basinRoot.add(g);
  addBox(g,1,.08,1,0,-.46,0,conc);
  const b=basinFillBounds(x,y,z,occ,rim),cx=(b.x0+b.x1)*.5,cz=(b.z0+b.z1)*.5,w=b.x1-b.x0,d=b.z1-b.z0;
  if(type===17)addBox(g,w,.36,d,cx,-.30,cz,moss);
  else addWaterPlane(g,w,d,cx,-.14,cz);
 }
 for(const[x,y,z] of group){
  const g=new THREE.Group();g.position.set(x+.5,y+.5,z+.5);basinRoot.add(g);
  const b=basinFillBounds(x,y,z,occ,rim);
  if(b.pz)addBox(g,1,.5,.1,0,-.25,.45,conc);if(b.nz)addBox(g,1,.5,.1,0,-.25,-.45,conc);
  if(b.nx)addBox(g,.1,.5,.8,-.45,-.25,0,conc);if(b.px)addBox(g,.1,.5,.8,.45,-.25,0,conc);
  if(b.px&&b.pz)addBox(g,.12,.5,.12,.44,-.25,.44,conc);if(b.px&&b.nz)addBox(g,.12,.5,.12,.44,-.25,-.44,conc);
  if(b.nx&&b.pz)addBox(g,.12,.5,.12,-.44,-.25,.44,conc);if(b.nx&&b.nz)addBox(g,.12,.5,.12,-.44,-.25,-.44,conc);
 }
}"""

new_basin = """function emitIsland(group,type){
 const occ=new Set(group.map(([x,y,z])=>`${x},${y},${z}`)),rim=.40,wallT=.12,wallH=.5,yo=-.25;
 for(const[x,y,z] of group){
  const g=new THREE.Group();g.position.set(x+.5,y+.5,z+.5);basinRoot.add(g);
  addBox(g,1,.08,1,0,-.46,0,conc);
  const b=basinFillBounds(x,y,z,occ,rim),cx=(b.x0+b.x1)*.5,cz=(b.z0+b.z1)*.5,w=b.x1-b.x0,d=b.z1-b.z0;
  if(type===17)addBox(g,w,.36,d,cx,-.30,cz,moss);
  else addWaterPlane(g,w,d,cx,-.14,cz);
 }
 for(const[x,y,z] of group){
  const g=new THREE.Group();g.position.set(x+.5,y+.5,z+.5);basinRoot.add(g);
  const b=basinFillBounds(x,y,z,occ,rim),cxy=.5-wallT*.35,cs=wallT*1.15;
  if(b.pz)addBox(g,1,wallH,wallT,0,yo,.5-wallT*.5,conc);if(b.nz)addBox(g,1,wallH,wallT,0,yo,-.5+wallT*.5,conc);
  if(b.nx)addBox(g,wallT,wallH,1,-.5+wallT*.5,yo,0,conc);if(b.px)addBox(g,wallT,wallH,1,.5-wallT*.5,yo,0,conc);
  if(b.px&&b.pz)addBox(g,cs,wallH,cs,cxy,yo,cxy,conc);if(b.px&&b.nz)addBox(g,cs,wallH,cs,cxy,yo,-cxy,conc);
  if(b.nx&&b.pz)addBox(g,cs,wallH,cs,-cxy,yo,cxy,conc);if(b.nx&&b.nz)addBox(g,cs,wallH,cs,-cxy,yo,-cxy,conc);
 }
}"""

if old_basin in s:
    s = s.replace(old_basin, new_basin, 1)
    changed.append("basin walls")
elif "wallT=.12" in s and "rim=.40" in s:
    changed.append("basin walls already")
else:
    raise SystemExit("emitIsland block not found")

# --- Wet moss pebbles: rockM/dirtM (not aggM which fights ivy) ---
old_peb = "addBox(g,sz,sz*.65,sz*rng.range(.7,1.2),px,py,pz,rng.next()<.45?rockM:aggM)"
new_peb = "addBox(g,sz,sz*.65,sz*rng.range(.7,1.2),px,py,pz,rng.next()<.55?rockM:dirtM)"
if old_peb in s:
    s = s.replace(old_peb, new_peb, 1)
    changed.append("wet pebbles")
elif new_peb in s:
    changed.append("wet pebbles already")

# --- Pipe hit: work without add cell (edge of world / hollow basins) ---
old_hit = """function pipeHitFromPick(h){if(!h||!h.rem||!h.add)return null;
 const rem=h.rem,add=h.add,dx=add[0]-rem[0],dy=add[1]-rem[1],dz=add[2]-rem[2];
 let face=-1;if(dx===1)face=0;else if(dx===-1)face=1;else if(dy===1)face=2;else if(dy===-1)face=3;else if(dz===1)face=4;else if(dz===-1)face=5;else return null;
 const N=PIPE_N[face],U=PIPE_U[face],V=PIPE_V[face];
 const hp=h.hp||[rem[0]+.5+N[0]*.5,rem[1]+.5+N[1]*.5,rem[2]+.5+N[2]*.5];
 const fx=rem[0]+.5+N[0]*.5,fy=rem[1]+.5+N[1]*.5,fz=rem[2]+.5+N[2]*.5;
 const lx=hp[0]-fx,ly=hp[1]-fy,lz=hp[2]-fz;
 const su=lx*U[0]+ly*U[1]+lz*U[2],sv=lx*V[0]+ly*V[1]+lz*V[2];
 return{cx:rem[0],cy:rem[1],cz:rem[2],face,u:Math.max(0,Math.min(2,Math.round(su/PIPE_PITCH+1))),v:Math.max(0,Math.min(2,Math.round(sv/PIPE_PITCH+1)))}}"""

new_hit = """function pipeHitFromPick(h){if(!h||!h.rem)return null;
 const rem=h.rem;let face=-1;
 if(h.add){const add=h.add,dx=add[0]-rem[0],dy=add[1]-rem[1],dz=add[2]-rem[2];
  if(dx===1)face=0;else if(dx===-1)face=1;else if(dy===1)face=2;else if(dy===-1)face=3;else if(dz===1)face=4;else if(dz===-1)face=5}
 if(face<0){const hp=h.hp||[rem[0]+.5,rem[1]+.5,rem[2]+.5],dx=hp[0]-(rem[0]+.5),dy=hp[1]-(rem[1]+.5),dz=hp[2]-(rem[2]+.5);
  const ax=Math.abs(dx),ay=Math.abs(dy),az=Math.abs(dz);
  if(ax>=ay&&ax>=az)face=dx>=0?0:1;else if(ay>=ax&&ay>=az)face=dy>=0?2:3;else face=dz>=0?4:5}
 const N=PIPE_N[face],U=PIPE_U[face],V=PIPE_V[face];
 const hp=h.hp||[rem[0]+.5+N[0]*.5,rem[1]+.5+N[1]*.5,rem[2]+.5+N[2]*.5];
 const fx=rem[0]+.5+N[0]*.5,fy=rem[1]+.5+N[1]*.5,fz=rem[2]+.5+N[2]*.5;
 const lx=hp[0]-fx,ly=hp[1]-fy,lz=hp[2]-fz;
 const su=lx*U[0]+ly*U[1]+lz*U[2],sv=lx*V[0]+ly*V[1]+lz*V[2];
 return{cx:rem[0],cy:rem[1],cz:rem[2],face,u:Math.max(0,Math.min(2,Math.round(su/PIPE_PITCH+1))),v:Math.max(0,Math.min(2,Math.round(sv/PIPE_PITCH+1)))}}"""

if old_hit in s:
    s = s.replace(old_hit, new_hit, 1)
    changed.append("pipe hit")
elif "if(!h||!h.rem)return null" in s and "face<0" in s:
    changed.append("pipe hit already")
else:
    raise SystemExit("pipeHitFromPick not found")

# --- Better rock canvas texture ---
old_rock = (
    'const ROCK_TEX=(function(){const c=document.createElement("canvas");c.width=c.height=128;'
    "const g=c.getContext(\"2d\"),id=g.createImageData(128,128);"
    "for(let y=0;y<128;y++)for(let x=0;x<128;x++){const i=(y*128+x)*4;"
    "const n1=Math.sin(x*.17+y*.11)*Math.cos(x*.09-y*.19)*.5+.5;"
    "const n2=((Math.sin(x*1.7+y*2.3)*43758.5453)%1+1)%1;"
    "const n3=((Math.sin(x*.41+y*.73+11)*24571.3)%1+1)%1;"
    "const vein=Math.abs(((x*1.7+y*2.1)/128)%1-.5)*2;"
    "let v=.42+n1*.18+n2*.12+n3*.08-vein*.06;v=Math.max(.05,Math.min(.95,v));"
    "id.data[i]=(v*145+35)|0;id.data[i+1]=(v*136+34)|0;id.data[i+2]=(v*124+32)|0;id.data[i+3]=255}"
    "g.putImageData(id,0,0);const t=new THREE.CanvasTexture(c);"
    "t.wrapS=t.wrapT=THREE.RepeatWrapping;t.repeat.set(2.2,2.2);t.needsUpdate=true;return t})();"
)
new_rock = (
    'const ROCK_TEX=(function(){const c=document.createElement("canvas");c.width=c.height=128;'
    "const g=c.getContext(\"2d\"),id=g.createImageData(128,128);"
    "for(let y=0;y<128;y++)for(let x=0;x<128;x++){const i=(y*128+x)*4;"
    "const n1=Math.sin(x*.11+y*.09)*Math.cos(x*.07-y*.13)*.5+.5;"
    "const n2=((Math.sin(x*2.1+y*1.7)*43758.5453)%1+1)%1;"
    "const n3=((Math.sin(x*.53+y*.91+11)*24571.3)%1+1)%1;"
    "const n4=((Math.sin(x*5.3+y*4.1+3)*91234.1)%1+1)%1;"
    "const vein=Math.abs(((x*1.1+y*1.6)/128)%1-.5)*2;"
    "const lichen=Math.max(0,n3-.55)*Math.max(0,n1-.35);"
    "let v=.34+n1*.22+n2*.14+n4*.1-vein*.08;v=Math.max(.08,Math.min(.92,v));"
    "id.data[i]=(v*130+28+lichen*40)|0;id.data[i+1]=(v*122+30+lichen*55)|0;id.data[i+2]=(v*110+28+lichen*18)|0;id.data[i+3]=255}"
    "g.putImageData(id,0,0);const t=new THREE.CanvasTexture(c);"
    "t.wrapS=t.wrapT=THREE.RepeatWrapping;t.repeat.set(1.6,1.6);t.needsUpdate=true;return t})();"
)

if old_rock in s:
    s = s.replace(old_rock, new_rock, 1)
    changed.append("rock tex")
elif "lichen=Math.max" in s:
    changed.append("rock tex already")
else:
    # try looser match
    if "const ROCK_TEX=(function()" in s and "lichen=Math.max" not in s:
        raise SystemExit("ROCK_TEX found but unexpected shape")
    changed.append("rock tex skip")

old_rm = "rockM=new THREE.MeshStandardMaterial({map:ROCK_TEX,bumpMap:ROCK_TEX,bumpScale:1.6,color:0xc4beb6,roughness:.97,flatShading:true})"
new_rm = "rockM=new THREE.MeshStandardMaterial({map:ROCK_TEX,bumpMap:ROCK_TEX,bumpScale:2.2,color:0xb9b2a8,roughness:.9,flatShading:true})"
if old_rm in s:
    s = s.replace(old_rm, new_rm, 1)
    changed.append("rockM")
elif new_rm in s:
    changed.append("rockM already")

HTML.write_text(s, encoding="utf-8")
print("patched:", ", ".join(changed))
