"""Patch Creating Constructs.html for Stone/Boulder nets + Log/Stump/Mushroom pieces."""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HTML = ROOT / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8", errors="ignore")

# TN names
old_tn = "TN=['Block','Slab','Pillar','Glass','Slope','Moss','Stairs','Column','Window','Panel','Arch','Railing','Beam','Light','Tree','Ivy','Cable','Planter','Brick','Timber','Steel','Fins','Cobogó','Doorway','Pool','Ladder','Bollard','Sconce','Dome','Silo','Vault','Fence','Slit','Ribbon','Floor','Pergola','Fine stairs','Pendant','Obelisk','Bench','Water','Birch','Pine','Pipe','Valve','Gauge','Outlet','Boardform','Cantilever','Batter','Waffle','Portal','Clerestory','Plinth','Chase','Spout','Pilotis','Megastep','Lightwell','Bridge','Core','Aggregate','Corbel'];"
new_tn = "TN=['Block','Slab','Pillar','Glass','Slope','Moss','Stairs','Column','Window','Panel','Arch','Railing','Beam','Light','Tree','Ivy','Cable','Planter','Brick','Timber','Steel','Fins','Cobogó','Doorway','Pool','Ladder','Bollard','Sconce','Dome','Silo','Vault','Fence','Slit','Ribbon','Floor','Pergola','Fine stairs','Pendant','Obelisk','Bench','Water','Birch','Pine','Pipe','Valve','Gauge','Outlet','Boardform','Cantilever','Batter','Waffle','Portal','Clerestory','Plinth','Chase','Spout','Pilotis','Megastep','Lightwell','Bridge','Core','Aggregate','Corbel','Stone','Boulder','Log','Stump','Mushroom'];"
if old_tn not in s:
    raise SystemExit("TN array not found or already patched")
s = s.replace(old_tn, new_tn, 1)

# CATS Nature row
old_cats = "const CATS=[['Structure',[0,1,34,48,49,50,56,57,58,59,60,2]],['Shapes',[4,36,28,29,30,38,12,7,35,37,6,9]],['Openings',[3,51,52,8,23,22,21,32,33,31,10]],['Detail',[53,54,55,62,11,25,13,27,26,39,16]],['Nature',[5,14,15,17,24,40,41,42]],['Material',[47,61,18,19,20,0]],['Pipes',[43,44,45,46]]];"
new_cats = "const CATS=[['Structure',[0,1,34,48,49,50,56,57,58,59,60,2]],['Shapes',[4,36,28,29,30,38,12,7,35,37,6,9]],['Openings',[3,51,52,8,23,22,21,32,33,31,10]],['Detail',[53,54,55,62,11,25,13,27,26,39,16]],['Nature',[5,14,15,17,24,40,41,42,63,64,65,66,67]],['Material',[47,61,18,19,20,0]],['Pipes',[43,44,45,46]]];"
if old_cats not in s:
    raise SystemExit("CATS not found or already patched")
s = s.replace(old_cats, new_cats, 1)

# rock material next to aggM
if "const rockM=" not in s:
    s = s.replace(
        "aggM=new THREE.MeshStandardMaterial({map:tex,bumpMap:tex,bumpScale:1.4,color:0xb8b0a4,roughness:.98})",
        "aggM=new THREE.MeshStandardMaterial({map:tex,bumpMap:tex,bumpScale:1.4,color:0xb8b0a4,roughness:.98}),rockM=new THREE.MeshStandardMaterial({map:tex,bumpMap:tex,bumpScale:2.1,color:0x8f8a82,roughness:.97})",
        1,
    )

# Append defs for 63-67 before closing ]]; of defs
old_defs_end = "[[mg([bx(.55,.12,.5,0,.38,-.05),bx(.4,.12,.4,0,.24,0),bx(.26,.12,.3,0,.1,.05)]),conc]]];"
new_defs_end = (
    "[[mg([bx(.55,.12,.5,0,.38,-.05),bx(.4,.12,.4,0,.24,0),bx(.26,.12,.3,0,.1,.05)]),conc]],"
    "[[geos[0],rockM]],"
    "[[geos[0],rockM]],"
    "[[cyl(.18,.55,-.15),bark]],"
    "[[cyl(.22,.7,-.1),bark]],"
    "[[mg([cyl(.05,.22,-.28),new THREE.SphereGeometry(.16,8,6).translate(0,-.05,0)]),leaf]]];"
)
if old_defs_end not in s:
    raise SystemExit("defs end not found")
s = s.replace(old_defs_end, new_defs_end, 1)

# buildAdaptiveTrees — log/stump/mushroom
old_trees = """  if(v.t===42){const seed=natureSeedAt(x,y,z,rot),rng=ivyRng(seed+83),variant=rng.next()<.5?0:1;
   emitNatureModel(PINE_VARIANT_NAMES[variant],origin,rot,{x:0,y:-.5,z:0},rng.range(.52,.64)*PINE_SCALE_MUL[variant],rng.range(0,Math.PI*2),PINE_WIDTH[variant],buckets,amb)}
  if((v.t===5||v.t===17)&&!cells.has(key(x,y+1,z)))natureUnderstory(v.t===5?.51:-.07,v.t===17,origin,rot,buckets,amb);"""
new_trees = """  if(v.t===42){const seed=natureSeedAt(x,y,z,rot),rng=ivyRng(seed+83),variant=rng.next()<.5?0:1;
   emitNatureModel(PINE_VARIANT_NAMES[variant],origin,rot,{x:0,y:-.5,z:0},rng.range(.52,.64)*PINE_SCALE_MUL[variant],rng.range(0,Math.PI*2),PINE_WIDTH[variant],buckets,amb)}
  if(v.t===65){const rng=ivyRng(natureSeedAt(x,y,z,rot)+91);emitNatureModel('nature_log',origin,rot,{x:0,y:-.35,z:0},rng.range(.85,1.05),rng.range(0,Math.PI*2),1,buckets,amb)}
  if(v.t===66){const rng=ivyRng(natureSeedAt(x,y,z,rot)+97);emitNatureModel('nature_stump',origin,rot,{x:0,y:-.5,z:0},rng.range(.9,1.15),rng.range(0,Math.PI*2),1,buckets,amb)}
  if(v.t===67){const rng=ivyRng(natureSeedAt(x,y,z,rot)+103),name=rng.next()<.5?'nature_mushroom_1':'nature_mushroom_2';
   emitNatureModel(name,origin,rot,{x:0,y:-.5,z:0},rng.range(.55,.85),rng.range(0,Math.PI*2),1,buckets,amb)}
  if((v.t===5||v.t===17)&&!cells.has(key(x,y+1,z)))natureUnderstory(v.t===5?.51:-.07,v.t===17,origin,rot,buckets,amb);"""
if old_trees not in s:
    raise SystemExit("buildAdaptiveTrees block not found")
s = s.replace(old_trees, new_trees, 1)

# ivySolidType soft props
s = s.replace(
    "function ivySolidType(t){return t>=0&&t!==11&&t!==13&&t!==14&&t!==15&&t!==16&&t!==25&&t!==26&&t!==27&&t!==35&&t!==37&&t!==40&&t!==41&&t!==42}",
    "function ivySolidType(t){return t>=0&&t!==11&&t!==13&&t!==14&&t!==15&&t!==16&&t!==25&&t!==26&&t!==27&&t!==35&&t!==37&&t!==40&&t!==41&&t!==42&&t!==65&&t!==66&&t!==67}",
    1,
)

# refresh skip nets + nature props
old_refresh = "if(v.t!==5&&v.t!==15&&v.t!==14&&v.t!==41&&v.t!==42&&!(v.t>=43&&v.t<=46)){meshes[v.t].forEach(m=>{m.setMatrixAt(n[v.t],D.matrix);m.setColorAt(n[v.t],cc)});n[v.t]++;}"
new_refresh = "if(v.t!==5&&v.t!==63&&v.t!==64&&v.t!==15&&v.t!==14&&v.t!==41&&v.t!==42&&v.t!==65&&v.t!==66&&v.t!==67&&!(v.t>=43&&v.t<=46)){meshes[v.t].forEach(m=>{m.setMatrixAt(n[v.t],D.matrix);m.setColorAt(n[v.t],cc)});n[v.t]++;}"
if old_refresh not in s:
    raise SystemExit("refresh skip not found")
s = s.replace(old_refresh, new_refresh, 1)

old_build = "lampPos=lamps;lightsSync();buildMossSurfaceNets();buildAdaptiveTrees();buildAdaptiveIvy();"
new_build = "lampPos=lamps;lightsSync();buildMossSurfaceNets();buildRockSurfaceNets();buildAdaptiveTrees();buildAdaptiveIvy();"
if old_build not in s:
    raise SystemExit("refresh build call not found")
s = s.replace(old_build, new_build, 1)

# solidAt cases before default
old_solid = "case 62:return fy>.05&&lz<.55-.35*fy&&Math.abs(lx-.5)<.3;default:return true}}"
new_solid = "case 62:return fy>.05&&lz<.55-.35*fy&&Math.abs(lx-.5)<.3;case 63:return dx*dx+dz*dz+(fy-.5)*(fy-.5)<.22;case 64:return dx*dx+dz*dz+(fy-.5)*(fy-.5)<.28;case 65:return fy<.42&&Math.abs(lz-.5)<.28;case 66:return Math.abs(lx-.5)<.28&&Math.abs(lz-.5)<.28&&fy<.85;case 67:return false;default:return true}}"
if old_solid not in s:
    raise SystemExit("solid switch end not found")
s = s.replace(old_solid, new_solid, 1)

MOSS_NETS = r'''/*MOSS_NETS*/
// Adaptive surface nets: Moss (soft) + Stone/Boulder (hard). Face-connected same-family
// cells blend; the other family and architecture stay cubic contacts.
(function(){
const DIR6=[[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]];
const CORNERS=[[0,0,0],[1,0,0],[0,1,0],[1,1,0],[0,0,1],[1,0,1],[0,1,1],[1,1,1]];
const EDGES=[[0,1],[2,3],[4,5],[6,7],[0,2],[1,3],[4,6],[5,7],[0,4],[1,5],[2,6],[3,7]];
const FACES=[
 {c:[0,1,3,2],e:[0,5,1,4]},{c:[4,5,7,6],e:[2,7,3,6]},
 {c:[0,1,5,4],e:[0,9,2,8]},{c:[2,3,7,6],e:[1,11,3,10]},
 {c:[0,2,6,4],e:[4,10,6,8]},{c:[1,3,7,5],e:[5,11,7,9]}
];
const STEP=.25;
const PROFILES={
 moss:{blend:.42,relief:.23,bevel:.105,sideWarp:.14,rock:false,mat:null},
 rock:{blend:.22,relief:.34,bevel:.045,sideWarp:.22,rock:true,mat:null}
};
let mossNetMesh=null,rockNetMesh=null;
function softProp(t){
 if(t===13||t===14||t===15||t===16||t===25||t===26||t===27||t===37||t===40||t===41||t===42||t===65||t===66||t===67)return true;
 if(t>=43&&t<=46)return true;return false;
}
function mossRigid(t){if(t==null||t<0||t===5)return false;if(softProp(t))return false;return true;}
function rockRigid(t){if(t==null||t<0||t===63||t===64)return false;if(softProp(t))return false;return true;}
function sharpBox(px,py,pz,cx,cy,cz,hx,hy,hz){
 const qx=Math.abs(px-cx)-hx,qy=Math.abs(py-cy)-hy,qz=Math.abs(pz-cz)-hz;
 return Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0);
}
function roundBox(px,py,pz,x,y,z,t,P){
 let bevel=P.bevel,half=.5-bevel;if(t===64)half=.48-bevel*.35;
 const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
 let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 if(t===64)d-=.06;return d;
}
function smoothMin(a,b,k){
 if(k<.001)return Math.min(a,b);
 const h=Math.max(k-Math.abs(a-b),0)/k;
 return Math.min(a,b)-h*h*k*.25;
}
function deformPoint(px,py,pz,P){
 if(P.relief<.001)return [px,py,pz];
 let ridge,swell,height,topWeight,fy=py-Math.floor(py);
 if(P.rock){
  ridge=Math.sin(px*3.1+pz*1.4)*.7+Math.cos(pz*4.6-px*2.2)*.45+Math.sin(px*7.1+pz*6.3)*.28;
  swell=Math.sin(px*1.3-pz*1.1)*Math.cos(pz*1.7)*.4;
  height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.08)/.85));
  return [px+P.sideWarp*P.relief*Math.sin(py*4.1+pz*3.2)*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*3.7+px*2.8)*topWeight];
 }
 ridge=Math.sin(px*2.35+pz*.81)*.58+Math.cos(pz*3.7-px*1.31)*.33+Math.sin(px*5.3+pz*4.6)*.20;
 swell=Math.sin(px*.81-pz*.67)*Math.cos(pz*.93+px*.35)*.52;
 height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.16)/.70));
 return [px+P.sideWarp*P.relief*Math.sin(py*3.2+pz*2.1)*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*2.9+px*1.9)*topWeight];
}
function islands(keys){
 const todo=new Set(keys),out=[];
 while(todo.size){
  const first=todo.values().next().value;todo.delete(first);
  const [fx,fy,fz]=first.split(',').map(Number),group=[[fx,fy,fz]],queue=[[fx,fy,fz]];
  for(let i=0;i<queue.length;i++){
   const [x,y,z]=queue[i];
   for(const [dx,dy,dz] of DIR6){const nk=`${x+dx},${y+dy},${z+dz}`;
    if(todo.delete(nk)){group.push([x+dx,y+dy,z+dz]);queue.push([x+dx,y+dy,z+dz]);}}
  }
  out.push(group);
 }
 return out;
}
function isRigidAt(x,y,z,rigid){
 const v=cells.get(key(x,y,z));if(v&&rigid(v.t))return true;
 const root=footprintRoot(x,y,z);if(!root)return false;return rigid(root[3].t);
}
function buildIsland(group,types,positions,indices,P,rigid){
 const occupied=new Map();for(let i=0;i<group.length;i++)occupied.set(`${group[i][0]},${group[i][1]},${group[i][2]}`,types[i]);
 const contacts=[],localConcrete=new Set();
 for(const [x,y,z] of group)for(const [dx,dy,dz] of DIR6){
  const nx=x+dx,ny=y+dy,nz=z+dz;if(!isRigidAt(nx,ny,nz,rigid))continue;
  const rk=`${nx},${ny},${nz}`;localConcrete.add(rk);
  const h=.19,cx=x+.5,cy=y+.5,cz=z+.5;
  contacts.push({cx:cx+dx*(.5-h/2),cy:cy+dy*(.5-h/2),cz:cz+dz*(.5-h/2),hx:dx?h/2:.5,hy:dy?h/2:.5,hz:dz?h/2:.5});
 }
 const lo=[1e9,1e9,1e9],hi=[-1e9,-1e9,-1e9];
 for(const c of group)for(let a=0;a<3;a++){lo[a]=Math.min(lo[a],c[a]);hi[a]=Math.max(hi[a],c[a]);}
 const pad=1,x0=lo[0]-pad,y0=lo[1]-pad,z0=lo[2]-pad;
 const nx=Math.ceil((hi[0]-lo[0]+1+2*pad)/STEP)+1;
 const ny=Math.ceil((hi[1]-lo[1]+1+2*pad)/STEP)+1;
 const nz=Math.ceil((hi[2]-lo[2]+1+2*pad)/STEP)+1;
 if(nx*ny*nz>280000)return;
 const sidx=(x,y,z)=>(z*ny+y)*nx+x;
 const samples=new Float32Array(nx*ny*nz);
 function field(px,py,pz){
  const rawX=px,rawY=py,rawZ=pz;
  const dpt=deformPoint(px,py,pz,P);px=dpt[0];py=dpt[1];pz=dpt[2];
  const ax=Math.floor(px),ay=Math.floor(py),az=Math.floor(pz);
  const nearby=[];
  for(let z=az-1;z<=az+1;z++)for(let y=ay-1;y<=ay+1;y++)for(let x=ax-1;x<=ax+1;x++){
   const tk=`${x},${y},${z}`;if(!occupied.has(tk))continue;
   nearby.push({x,y,z,t:occupied.get(tk),d:roundBox(px,py,pz,x,y,z,occupied.get(tk),P)});
  }
  if(!nearby.length&&!contacts.length)return 2;
  nearby.sort((a,b)=>a.d-b.d);
  let value=nearby.length?nearby[0].d:2;
  if(nearby.length){const owner=nearby[0];
   for(let i=1;i<nearby.length;i++){
    const other=nearby[i],man=Math.abs(owner.x-other.x)+Math.abs(owner.y-other.y)+Math.abs(owner.z-other.z);
    value=man===1?smoothMin(value,other.d,P.blend):Math.min(value,other.d);
   }
  }
  for(const c of contacts){
   if(Math.abs(rawX-c.cx)>c.hx+.55||Math.abs(rawY-c.cy)>c.hy+.55||Math.abs(rawZ-c.cz)>c.hz+.55)continue;
   value=Math.min(value,sharpBox(rawX,rawY,rawZ,c.cx,c.cy,c.cz,c.hx,c.hy,c.hz));
  }
  for(const rk of localConcrete){
   const [x,y,z]=rk.split(',').map(Number);
   if(Math.abs(rawX-(x+.5))>1||Math.abs(rawY-(y+.5))>1||Math.abs(rawZ-(z+.5))>1)continue;
   value=Math.max(value,-sharpBox(rawX,rawY,rawZ,x+.5,y+.5,z+.5,.5,.5,.5));
  }
  return value;
 }
 for(let z=0;z<nz;z++)for(let y=0;y<ny;y++)for(let x=0;x<nx;x++)samples[sidx(x,y,z)]=field(x0+x*STEP,y0+y*STEP,z0+z*STEP);
 const cx=nx-1,cy=ny-1,cz=nz-1,cidx=(x,y,z)=>(z*cy+y)*cx+x;
 const edgeVerts=new Int32Array(cx*cy*cz*12);edgeVerts.fill(-1);
 for(let z=0;z<cz;z++)for(let y=0;y<cy;y++)for(let x=0;x<cx;x++){
  const values=CORNERS.map(c=>samples[sidx(x+c[0],y+c[1],z+c[2])]);
  let mask=0;for(let c=0;c<8;c++)if(values[c]<0)mask|=1<<c;
  if(!mask||mask===255)continue;
  const active=EDGES.map(([a,b])=>(values[a]<0)!==(values[b]<0));
  const parent=Array.from({length:12},(_,i)=>i);
  const root=i=>{while(parent[i]!==i){parent[i]=parent[parent[i]];i=parent[i];}return i;};
  const link=(a,b)=>{parent[root(a)]=root(b);};
  for(const face of FACES){
   const crossed=face.e.filter(e=>active[e]);
   if(crossed.length===2){link(crossed[0],crossed[1]);continue;}
   if(crossed.length===4){
    const avg=face.c.reduce((s,c)=>s+values[c],0)*.25,inside=avg<0;
    for(let i=0;i<4;i++)if((values[face.c[i]]<0)!==inside)link(face.e[(i+3)%4],face.e[i]);
   }
  }
  const loops=new Map();
  for(let e=0;e<12;e++)if(active[e]){const k=root(e);if(!loops.has(k))loops.set(k,[]);loops.get(k).push(e);}
  for(const crossings of loops.values()){
   let vx=0,vy=0,vz=0;
   for(const e of crossings){const [a,b]=EDGES[e],va=values[a],vb=values[b],t=va/(va-vb),p=CORNERS[a],q=CORNERS[b];
    vx+=p[0]+(q[0]-p[0])*t;vy+=p[1]+(q[1]-p[1])*t;vz+=p[2]+(q[2]-p[2])*t;}
   const n=crossings.length,id=positions.length/3;
   positions.push(x0+(x+vx/n)*STEP,y0+(y+vy/n)*STEP,z0+(z+vz/n)*STEP);
   for(const e of crossings)edgeVerts[cidx(x,y,z)*12+e]=id;
  }
 }
 const ev=(x,y,z,edge)=>edgeVerts[cidx(x,y,z)*12+edge];
 const addQuad=(a,b,c,d,front)=>{if(a<0||b<0||c<0||d<0)return;if(front)indices.push(a,b,c,a,c,d);else indices.push(a,c,b,a,d,c);};
 for(let z=1;z<nz-1;z++)for(let y=1;y<ny-1;y++)for(let x=0;x<nx-1;x++){
  const a=samples[sidx(x,y,z)],b=samples[sidx(x+1,y,z)];if((a<0)===(b<0))continue;
  addQuad(ev(x,y-1,z-1,3),ev(x,y,z-1,2),ev(x,y,z,0),ev(x,y-1,z,1),a<0);
 }
 for(let z=1;z<nz-1;z++)for(let y=0;y<ny-1;y++)for(let x=1;x<nx-1;x++){
  const a=samples[sidx(x,y,z)],b=samples[sidx(x,y+1,z)];if((a<0)===(b<0))continue;
  addQuad(ev(x-1,y,z-1,7),ev(x-1,y,z,5),ev(x,y,z,4),ev(x,y,z-1,6),a<0);
 }
 for(let z=0;z<nz-1;z++)for(let y=1;y<ny-1;y++)for(let x=1;x<nx-1;x++){
  const a=samples[sidx(x,y,z)],b=samples[sidx(x,y,z+1)];if((a<0)===(b<0))continue;
  addQuad(ev(x-1,y-1,z,11),ev(x,y-1,z,10),ev(x,y,z,8),ev(x-1,y,z,9),a<0);
 }
}
function buildFamily(isFamily,P,rigid,mat,holder){
 if(holder.mesh){S.remove(holder.mesh);holder.mesh.geometry.dispose();holder.mesh=null;}
 const keys=[],types=[];
 cells.forEach((v,k)=>{if(!isFamily(v.t))return;const y=k&31,z=(k>>5)&31,x=k>>10;keys.push(`${x},${y},${z}`);types.push(v.t);});
 if(!keys.length)return;
 // rebuild types aligned with islands via lookup
 const typeAt=new Map();cells.forEach((v,k)=>{if(isFamily(v.t)){const y=k&31,z=(k>>5)&31,x=k>>10;typeAt.set(`${x},${y},${z}`,v.t);}});
 const positions=[],indices=[];
 for(const group of islands([...typeAt.keys()])){
  const gTypes=group.map(([x,y,z])=>typeAt.get(`${x},${y},${z}`));
  buildIsland(group,gTypes,positions,indices,P,rigid);
 }
 if(!indices.length)return;
 const geo=new THREE.BufferGeometry();
 geo.setAttribute('position',new THREE.Float32BufferAttribute(positions,3));
 geo.setIndex(indices);geo.computeVertexNormals();
 const m=mat.clone();m.flatShading=true;m.side=THREE.DoubleSide;if(!P.rock)foliageWrap(m);
 holder.mesh=new THREE.Mesh(geo,m);holder.mesh.castShadow=true;holder.mesh.receiveShadow=true;S.add(holder.mesh);
}
const mossHolder={get mesh(){return mossNetMesh},set mesh(v){mossNetMesh=v}};
const rockHolder={get mesh(){return rockNetMesh},set mesh(v){rockNetMesh=v}};
function buildMossSurfaceNets(){buildFamily(t=>t===5,PROFILES.moss,mossRigid,moss,mossHolder)}
function buildRockSurfaceNets(){buildFamily(t=>t===63||t===64,PROFILES.rock,rockRigid,rockM,rockHolder)}
window.buildMossSurfaceNets=buildMossSurfaceNets;
window.buildRockSurfaceNets=buildRockSurfaceNets;
})();
/*MOSS_NETS_END*/
'''

m = re.search(r"/\*MOSS_NETS\*/.*?/\*MOSS_NETS_END\*/", s, re.S)
if not m:
    raise SystemExit("MOSS_NETS block missing")
s = s[: m.start()] + MOSS_NETS + s[m.end() :]

HTML.write_text(s, encoding="utf-8")
print("patched Creating Constructs.html nature wave 3")
