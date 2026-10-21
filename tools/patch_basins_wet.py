"""Patch Creating Constructs.html: merged planters/pools + wet moss beds/water plants."""
from __future__ import annotations
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

# Dirt material next to rockM
if "dirtM=" not in s:
    old = "rockM=new THREE.MeshStandardMaterial({map:tex,bumpMap:tex,bumpScale:2.1,color:0x8f8a82,roughness:.97})"
    new = old + ",dirtM=new THREE.MeshStandardMaterial({map:tex,bumpMap:tex,bumpScale:1.9,color:0x735438,roughness:.99})"
    if old not in s:
        raise SystemExit("rockM not found")
    s = s.replace(old, new, 1)

# Replace natureUnderstory + buildAdaptiveTrees understory calls
old_under = """function natureUnderstory(height,planter,origin,rot,buckets,amb){
 const rng=ivyRng(natureSeedAt(Math.floor(origin.x),Math.floor(origin.y),Math.floor(origin.z),rot)^319),spread=planter?.29:.44,community=Math.min(5,rng.next()*6|0),fertility=rng.range(.38,1);
 for(let i=0;i<(planter?4:9);i++){if(i>0&&rng.next()>fertility)continue;
  const root={x:rng.range(-spread,spread),y:height,z:rng.range(-spread,spread)},a=rng.range(0,Math.PI*2),s=rng.range(.72,1.17),variant=Math.min(5,rng.next()*6|0);
  if(i===0)natureFern(variant,root,s*(planter?.48:.68),a,origin,rot,buckets,amb);
  else if(i===1)natureGrass(variant,root,s,a,false,origin,rot,buckets,amb);
  else if(i===3||i===6){const h=s*(planter?.48:.72);
   if(community===1)emitNatureModel(variant&1?'nature_dogbane':'nature_dogbane_2',origin,rot,root,h,a,1,buckets,amb);
   else if(community===2)emitNatureModel('nature_aster',origin,rot,root,h,a,1,buckets,amb);
   else if(community===3)emitNatureModel('nature_goldenrod',origin,rot,root,h*.8,a,1,buckets,amb);
   else natureGrass(variant,root,h,a,true,origin,rot,buckets,amb);
  }else if(community===4)emitNatureModel('nature_clover',origin,rot,root,s*(planter?.48:.68),a,1,buckets,amb);
  else if(community===5)natureFern(variant,root,s*(planter?.42:.6),a,origin,rot,buckets,amb);
  else natureGrass(variant,root,s,a,false,origin,rot,buckets,amb);
 }
}"""

new_under = """function natureUnderstory(height,planter,origin,rot,buckets,amb){
 const rng=ivyRng(natureSeedAt(Math.floor(origin.x),Math.floor(origin.y),Math.floor(origin.z),rot)^319),spread=planter?.29:.44,community=Math.min(5,rng.next()*6|0),fertility=rng.range(.38,1);
 for(let i=0;i<(planter?4:9);i++){if(i>0&&rng.next()>fertility)continue;
  const root={x:rng.range(-spread,spread),y:height,z:rng.range(-spread,spread)},a=rng.range(0,Math.PI*2),s=rng.range(.72,1.17),variant=Math.min(5,rng.next()*6|0);
  if(i===0)natureFern(variant,root,s*(planter?.48:.68),a,origin,rot,buckets,amb);
  else if(i===1)natureGrass(variant,root,s,a,false,origin,rot,buckets,amb);
  else if(i===3||i===6){const h=s*(planter?.48:.72);
   if(community===1)emitNatureModel(variant&1?'nature_dogbane':'nature_dogbane_2',origin,rot,root,h,a,1,buckets,amb);
   else if(community===2)emitNatureModel('nature_aster',origin,rot,root,h,a,1,buckets,amb);
   else if(community===3)emitNatureModel('nature_goldenrod',origin,rot,root,h*.8,a,1,buckets,amb);
   else natureGrass(variant,root,h,a,true,origin,rot,buckets,amb);
  }else if(community===4)emitNatureModel('nature_clover',origin,rot,root,s*(planter?.48:.68),a,1,buckets,amb);
  else if(community===5)natureFern(variant,root,s*(planter?.42:.6),a,origin,rot,buckets,amb);
  else natureGrass(variant,root,s,a,false,origin,rot,buckets,amb);
 }
}
function natureWaterUnderstory(height,origin,rot,buckets,amb){
 const rng=ivyRng(natureSeedAt(Math.floor(origin.x),Math.floor(origin.y),Math.floor(origin.z),rot)^521),count=3+(rng.next()*5|0);
 for(let i=0;i<count;i++){
  const root={x:rng.range(-.42,.42),y:height,z:rng.range(-.42,.42)},a=rng.range(0,Math.PI*2),s=rng.range(.75,1.25),variant=Math.min(5,rng.next()*6|0),kind=rng.next()*4|0;
  if(kind===0)natureGrass(variant,root,s*rng.range(1.15,1.55),a,true,origin,rot,buckets,amb);
  else if(kind===1)emitNatureModel('nature_clover',origin,rot,{x:root.x,y:root.y-rng.range(.02,.08),z:root.z},s*rng.range(.95,1.35),a,1,buckets,amb);
  else if(kind===2)emitNatureModel(variant&1?'nature_dogbane':'nature_dogbane_2',origin,rot,root,s*.7,a,1,buckets,amb);
  else natureFern(variant,root,s*.55,a,origin,rot,buckets,amb);
 }
}
function mossWetAt(x,y,z){
 if((cells.get(key(x,y+1,z))||{}).t===40)return true;
 for(const[dx,dy,dz] of [[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]]){const t=(cells.get(key(x+dx,y+dy,z+dz))||{}).t;if(t===40||t===24)return true}
 return false;
}"""

if old_under not in s:
    raise SystemExit("natureUnderstory not found")
s = s.replace(old_under, new_under, 1)

old_trees_bit = """  if((v.t===5||v.t===17)&&!cells.has(key(x,y+1,z)))natureUnderstory(v.t===5?.51:-.07,v.t===17,origin,rot,buckets,amb);
 });"""

new_trees_bit = """  if(v.t===5){
   const wet=mossWetAt(x,y,z),above=(cells.get(key(x,y+1,z))||{}).t;
   if(wet&&above===40)natureWaterUnderstory(.92,origin,rot,buckets,amb);
   else if(above==null)natureUnderstory(.51,false,origin,rot,buckets,amb);
  }
  if(v.t===17&&!cells.has(key(x,y+1,z)))natureUnderstory(-.07,true,origin,rot,buckets,amb);
  if(v.t===40&&(cells.get(key(x,y-1,z))||{}).t===5)natureWaterUnderstory(-.38,origin,rot,buckets,amb);
 });"""

if old_trees_bit not in s:
    raise SystemExit("buildAdaptiveTrees understory bit not found")
s = s.replace(old_trees_bit, new_trees_bit, 1)

BASIN_JS = r"""
/*BASINS*/
// Merged planters (17) / pools (24) + wet-moss dirt/stone beds.
(function(){
let basinRoot=null,wetRoot=null;
function clearGroup(g){if(!g)return;while(g.children.length){const m=g.children[0];g.remove(m);if(m.geometry)m.geometry.dispose()}}
function pack(x,y,z){return (x<<10)|(z<<5)|y}
function islands(type){
 const todo=new Set();cells.forEach((v,k)=>{if(v.t===type){const y=k&31,z=(k>>5)&31,x=k>>10;todo.add(`${x},${y},${z}`)}});
 const out=[];
 while(todo.size){
  const first=todo.values().next().value;todo.delete(first);
  const[fx,fy,fz]=first.split(',').map(Number),group=[[fx,fy,fz]],q=[[fx,fy,fz]];
  for(let i=0;i<q.length;i++){const[x,y,z]=q[i];
   for(const[dx,dz] of [[1,0],[-1,0],[0,1],[0,-1]]){const nk=`${x+dx},${y},${z+dz}`;
    if(todo.delete(nk)){group.push([x+dx,y,z+dz]);q.push([x+dx,y,z+dz])}}}
  out.push(group);
 }
 return out;
}
function addBox(parent,w,h,d,x,y,z,mat){
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
}
function buildMergedBasins(){
 if(basinRoot){S.remove(basinRoot);clearGroup(basinRoot)}
 basinRoot=new THREE.Group();S.add(basinRoot);
 for(const type of [17,24])for(const g of islands(type))emitIsland(g,type);
}
function buildWetMossBeds(){
 if(wetRoot){S.remove(wetRoot);clearGroup(wetRoot)}
 wetRoot=new THREE.Group();S.add(wetRoot);
 cells.forEach((v,k)=>{if(v.t!==5)return;const y=k&31,z=(k>>5)&31,x=k>>10;if(!mossWetAt(x,y,z))return;
  const g=new THREE.Group();g.position.set(x+.5,y+.5,z+.5);wetRoot.add(g);
  addBox(g,.92,.07,.92,0,.43,0,dirtM);
  const rng=ivyRng(natureSeedAt(x,y,z,v.r)^777),n=3+(rng.next()*4|0);
  for(let i=0;i<n;i++){const px=rng.range(-.36,.36),pz=rng.range(-.36,.36),sz=rng.range(.05,.13),py=.46+rng.range(0,.04);
   addBox(g,sz,sz*.65,sz*rng.range(.7,1.2),px,py,pz,rng.next()<.45?rockM:aggM)}
 });
}
window.buildMergedBasins=buildMergedBasins;
window.buildWetMossBeds=buildWetMossBeds;
})();
/*BASINS_END*/
"""

if "/*BASINS*/" in s:
    import re
    s = re.sub(r"/\*BASINS\*/.*?/\*BASINS_END\*/", BASIN_JS.strip(), s, count=1, flags=re.S)
else:
    marker = "/*MOSS_NETS_END*/"
    if marker not in s:
        raise SystemExit("MOSS_NETS_END missing")
    s = s.replace(marker, marker + "\n\n" + BASIN_JS, 1)

# refresh: skip 17/24 instances, call basin builders
old_ref = "if(v.t!==5&&v.t!==63&&v.t!==64&&v.t!==15&&v.t!==14&&v.t!==41&&v.t!==42&&v.t!==65&&v.t!==66&&v.t!==67&&!(v.t>=43&&v.t<=46)){meshes[v.t].forEach(m=>{m.setMatrixAt(n[v.t],D.matrix);m.setColorAt(n[v.t],cc)});n[v.t]++;}"
new_ref = "if(v.t!==5&&v.t!==63&&v.t!==64&&v.t!==17&&v.t!==24&&v.t!==15&&v.t!==14&&v.t!==41&&v.t!==42&&v.t!==65&&v.t!==66&&v.t!==67&&!(v.t>=43&&v.t<=46)){meshes[v.t].forEach(m=>{m.setMatrixAt(n[v.t],D.matrix);m.setColorAt(n[v.t],cc)});n[v.t]++;}"
if old_ref not in s:
    raise SystemExit("refresh skip not found")
s = s.replace(old_ref, new_ref, 1)

old_build = "lampPos=lamps;lightsSync();buildMossSurfaceNets();buildRockSurfaceNets();buildAdaptiveTrees();buildAdaptiveIvy();"
new_build = "lampPos=lamps;lightsSync();buildMossSurfaceNets();buildRockSurfaceNets();buildMergedBasins();buildWetMossBeds();buildAdaptiveTrees();buildAdaptiveIvy();"
if old_build not in s:
    raise SystemExit("refresh build chain not found")
s = s.replace(old_build, new_build, 1)

HTML.write_text(s, encoding="utf-8")
print("patched basins + wet moss into HTML")
