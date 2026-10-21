"""Fix HTML merged basins: inset fills, top-only pool water (no orange side bleed)."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

old = """function addBox(parent,w,h,d,x,y,z,mat){
 const m=new THREE.Mesh(new THREE.BoxGeometry(w,h,d),mat);m.position.set(x,y,z);m.castShadow=true;m.receiveShadow=true;parent.add(m)
}
function emitIsland(group,type){
 const occ=new Set(group.map(([x,y,z])=>`${x},${y},${z}`));
 const fill=type===17?moss:waterM;
 for(const[x,y,z] of group){
  const g=new THREE.Group();g.position.set(x+.5,y+.5,z+.5);basinRoot.add(g);
  addBox(g,.92,.1,.92,0,-.45,0,conc);addBox(g,.88,type===17?.4:.04,.88,0,type===17?-.28:-.14,0,fill);
 }
 for(const[x,y,z] of group){
  const g=new THREE.Group();g.position.set(x+.5,y+.5,z+.5);basinRoot.add(g);
  const px=!occ.has(`${x+1},${y},${z}`),nx=!occ.has(`${x-1},${y},${z}`),pz=!occ.has(`${x},${y},${z+1}`),nz=!occ.has(`${x},${y},${z-1}`);
  if(pz)addBox(g,1,.5,.1,0,-.25,.45,conc);if(nz)addBox(g,1,.5,.1,0,-.25,-.45,conc);
  if(nx)addBox(g,.1,.5,.8,-.45,-.25,0,conc);if(px)addBox(g,.1,.5,.8,.45,-.25,0,conc);
  if(px&&pz)addBox(g,.12,.5,.12,.44,-.25,.44,conc);if(px&&nz)addBox(g,.12,.5,.12,.44,-.25,-.44,conc);
  if(nx&&pz)addBox(g,.12,.5,.12,-.44,-.25,.44,conc);if(nx&&nz)addBox(g,.12,.5,.12,-.44,-.25,-.44,conc);
 }
}"""

new = """function addBox(parent,w,h,d,x,y,z,mat){
 const m=new THREE.Mesh(new THREE.BoxGeometry(w,h,d),mat);m.position.set(x,y,z);m.castShadow=true;m.receiveShadow=true;parent.add(m)
}
function addWaterPlane(parent,w,d,x,y,z){
 const m=new THREE.Mesh(new THREE.PlaneGeometry(w,d),waterM);m.rotation.x=-Math.PI/2;m.position.set(x,y,z);m.receiveShadow=true;parent.add(m)
}
function basinFillBounds(x,y,z,occ,rim){
 const px=!occ.has(`${x+1},${y},${z}`),nx=!occ.has(`${x-1},${y},${z}`),pz=!occ.has(`${x},${y},${z+1}`),nz=!occ.has(`${x},${y},${z-1}`);
 return{x0:nx?-rim:-.5,x1:px?rim:.5,z0:nz?-rim:-.5,z1:pz?rim:.5,px,nx,pz,nz}
}
function emitIsland(group,type){
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

if old not in s:
    if "addWaterPlane" in s:
        print("HTML basins already fixed")
    else:
        raise SystemExit("emitIsland block not found")
else:
    s = s.replace(old, new, 1)
    HTML.write_text(s, encoding="utf-8")
    print("patched HTML basin water")
