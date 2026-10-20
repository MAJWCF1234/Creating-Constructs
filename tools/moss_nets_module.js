/*MOSS_NETS*/
// Faceted surface nets for Moss (t=5). Face-connected moss blends; rigid neighbors
// stay cubic — moss meets them on flat contact faces (piled earth, not warped concrete).
(function(){
const DIR6=[[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]];
const CORNERS=[[0,0,0],[1,0,0],[0,1,0],[1,1,0],[0,0,1],[1,0,1],[0,1,1],[1,1,1]];
const EDGES=[[0,1],[2,3],[4,5],[6,7],[0,2],[1,3],[4,6],[5,7],[0,4],[1,5],[2,6],[3,7]];
const FACES=[
 {c:[0,1,3,2],e:[0,5,1,4]},{c:[4,5,7,6],e:[2,7,3,6]},
 {c:[0,1,5,4],e:[0,9,2,8]},{c:[2,3,7,6],e:[1,11,3,10]},
 {c:[0,2,6,4],e:[4,10,6,8]},{c:[1,3,7,5],e:[5,11,7,9]}
];
const STEP=.25,BLEND=.42,RELIEF=.23;
let mossNetMesh=null;
function mossRigid(t){
 if(t==null||t<0||t===5)return false;
 // Non-mass / soft / tools — do not form flat concrete contacts
 if(t===13||t===14||t===15||t===16||t===25||t===26||t===27||t===37||t===40||t===41||t===42)return false;
 if(t>=43&&t<=46)return false;
 return true;
}
function sharpBox(px,py,pz,cx,cy,cz,hx,hy,hz){
 const qx=Math.abs(px-cx)-hx,qy=Math.abs(py-cy)-hy,qz=Math.abs(pz-cz)-hz;
 return Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0);
}
function roundBox(px,py,pz,x,y,z){
 const bevel=.105,half=.5-bevel;
 const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
 return Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
}
function smoothMin(a,b,k){
 if(k<.001)return Math.min(a,b);
 const h=Math.max(k-Math.abs(a-b),0)/k;
 return Math.min(a,b)-h*h*k*.25;
}
function deformPoint(px,py,pz){
 if(RELIEF<.001)return [px,py,pz];
 const ridge=Math.sin(px*2.35+pz*.81)*.58+Math.cos(pz*3.7-px*1.31)*.33+Math.sin(px*5.3+pz*4.6)*.20;
 const swell=Math.sin(px*.81-pz*.67)*Math.cos(pz*.93+px*.35)*.52;
 const height=(ridge+swell)*RELIEF;
 const topWeight=Math.max(0,Math.min(1,(py-Math.floor(py)-.16)/.70));
 return [px+.14*RELIEF*Math.sin(py*3.2+pz*2.1)*topWeight,py-height*topWeight,pz+.14*RELIEF*Math.cos(py*2.9+px*1.9)*topWeight];
}
function mossIslands(keys){
 const todo=new Set(keys),islands=[];
 while(todo.size){
  const first=todo.values().next().value;todo.delete(first);
  const [fx,fy,fz]=first.split(',').map(Number),group=[[fx,fy,fz]],queue=[[fx,fy,fz]];
  for(let i=0;i<queue.length;i++){
   const [x,y,z]=queue[i];
   for(const [dx,dy,dz] of DIR6){const nk=`${x+dx},${y+dy},${z+dz}`;
    if(todo.delete(nk)){group.push([x+dx,y+dy,z+dz]);queue.push([x+dx,y+dy,z+dz]);}}
  }
  islands.push(group);
 }
 return islands;
}
function isRigidAt(x,y,z){
 const v=cells.get(key(x,y,z));if(v&&mossRigid(v.t))return true;
 const root=footprintRoot(x,y,z);if(!root)return false;return mossRigid(root[3].t);
}
function buildIsland(group,positions,indices){
 const occupied=new Set(group.map(([x,y,z])=>`${x},${y},${z}`));
 const contacts=[],localConcrete=new Set();
 for(const [x,y,z] of group)for(const [dx,dy,dz] of DIR6){
  const nx=x+dx,ny=y+dy,nz=z+dz;if(!isRigidAt(nx,ny,nz))continue;
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
 if(nx*ny*nz>280000)return; // safety cap
 const sidx=(x,y,z)=>(z*ny+y)*nx+x;
 const samples=new Float32Array(nx*ny*nz);
 function field(px,py,pz){
  const rawX=px,rawY=py,rawZ=pz;
  const dpt=deformPoint(px,py,pz);px=dpt[0];py=dpt[1];pz=dpt[2];
  const ax=Math.floor(px),ay=Math.floor(py),az=Math.floor(pz);
  const nearby=[];
  for(let z=az-1;z<=az+1;z++)for(let y=ay-1;y<=ay+1;y++)for(let x=ax-1;x<=ax+1;x++){
   if(occupied.has(`${x},${y},${z}`))nearby.push({x,y,z,d:roundBox(px,py,pz,x,y,z)});
  }
  if(!nearby.length&&!contacts.length)return 2;
  nearby.sort((a,b)=>a.d-b.d);
  let value=nearby.length?nearby[0].d:2;
  if(nearby.length){const owner=nearby[0];
   for(let i=1;i<nearby.length;i++){
    const other=nearby[i],man=Math.abs(owner.x-other.x)+Math.abs(owner.y-other.y)+Math.abs(owner.z-other.z);
    value=man===1?smoothMin(value,other.d,BLEND):Math.min(value,other.d);
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
function buildMossSurfaceNets(){
 if(mossNetMesh){S.remove(mossNetMesh);mossNetMesh.geometry.dispose();mossNetMesh=null;}
 const keys=[];cells.forEach((v,k)=>{if(v.t===5){const y=k&31,z=(k>>5)&31,x=k>>10;keys.push(`${x},${y},${z}`);}});
 if(!keys.length)return;
 const positions=[],indices=[];
 for(const group of mossIslands(keys))buildIsland(group,positions,indices);
 if(!indices.length)return;
 const geo=new THREE.BufferGeometry();
 geo.setAttribute('position',new THREE.Float32BufferAttribute(positions,3));
 geo.setIndex(indices);geo.computeVertexNormals();
 const mat=moss.clone();mat.flatShading=true;mat.side=THREE.DoubleSide;foliageWrap(mat);
 mossNetMesh=new THREE.Mesh(geo,mat);mossNetMesh.castShadow=true;mossNetMesh.receiveShadow=true;S.add(mossNetMesh);
}
window.buildMossSurfaceNets=buildMossSurfaceNets;
})();
/*MOSS_NETS_END*/
