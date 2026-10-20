/*PIPE_SYSTEM*/
// Face-mounted 3×3 pipe router for Creating Constructs (walls / floor / ceiling).
const PIPE_PITCH=1/3,PIPE_HALF=PIPE_PITCH/2,PIPE_R=.03,PIPE_STANDOFF=.055;
const PIPE_KIND={43:'pipe',44:'valve',45:'gauge',46:'outlet'};
function isPipeTool(t){return t>=43&&t<=46}
const pipeNodes=new Map();
const pipeMeshes=new Map();
const pipeRoot=new THREE.Group();S.add(pipeRoot);
const pipeMat=new THREE.MeshStandardMaterial({color:0x8f98a2,metalness:.92,roughness:.25});
const pipeIron=new THREE.MeshStandardMaterial({color:0x22262d,metalness:.78,roughness:.45});
const pipeBrass=new THREE.MeshStandardMaterial({color:0xc8963e,metalness:.88,roughness:.28});
const pipeBolt=new THREE.MeshStandardMaterial({color:0xdde3eb,metalness:.95,roughness:.2});
const pipeValveRed=new THREE.MeshStandardMaterial({color:0xc52828,metalness:.35,roughness:.3});
let pipeGaugeDial;(function(){const c=document.createElement('canvas');c.width=c.height=128;const g=c.getContext('2d');
g.fillStyle='#f8fafc';g.fillRect(0,0,128,128);g.beginPath();g.arc(64,64,58,0,Math.PI*2);g.lineWidth=5;g.strokeStyle='#1e293b';g.stroke();
for(let i=0;i<=10;i++){const a=.75*Math.PI+(i/10)*1.5*Math.PI;g.beginPath();g.moveTo(64+Math.cos(a)*54,64+Math.sin(a)*54);g.lineTo(64+Math.cos(a)*44,64+Math.sin(a)*44);g.lineWidth=i%2?2:3;g.strokeStyle=i>7?'#dc2626':'#0f172a';g.stroke()}
g.beginPath();g.moveTo(64,64);g.lineTo(96,44);g.lineWidth=2;g.strokeStyle='#dc2626';g.stroke();g.beginPath();g.arc(64,64,4,0,Math.PI*2);g.fillStyle='#0f172a';g.fill();
pipeGaugeDial=new THREE.MeshBasicMaterial({map:new THREE.CanvasTexture(c)})})();
const _pg=new Map();
const pCyl=(...a)=>{const k='c'+a;if(!_pg.has(k))_pg.set(k,new THREE.CylinderGeometry(...a));return _pg.get(k)};
const pBox=(...a)=>{const k='b'+a;if(!_pg.has(k))_pg.set(k,new THREE.BoxGeometry(...a));return _pg.get(k)};
const pTor=(...a)=>{const k='t'+a;if(!_pg.has(k))_pg.set(k,new THREE.TorusGeometry(...a));return _pg.get(k)};
const pSph=(...a)=>{const k='s'+a;if(!_pg.has(k))_pg.set(k,new THREE.SphereGeometry(...a));return _pg.get(k)};
const pCir=(...a)=>{const k='r'+a;if(!_pg.has(k))_pg.set(k,new THREE.CircleGeometry(...a));return _pg.get(k)};
function pipeAdd(g,geo,mat,x=0,y=0,z=0,rz=0){const m=new THREE.Mesh(geo,mat);m.position.set(x,y,z);m.rotation.z=rz;m.castShadow=true;g.add(m);return m}
function pipeAnchor(){const g=new THREE.Group();pipeAdd(g,pBox(.08,.08,.006),pipeIron,0,0,-PIPE_STANDOFF+.003);
 const stem=pipeAdd(g,pCyl(.008,.008,PIPE_STANDOFF-.012,10),pipeIron,0,0,-PIPE_STANDOFF/2);stem.rotation.x=Math.PI/2;
 pipeAdd(g,pTor(PIPE_R+.005,.006,8,20),pipeIron);
 for(const x of[-.027,.027])for(const y of[-.027,.027]){const b=pipeAdd(g,pCyl(.006,.006,.008,6),pipeBolt,x,y,-PIPE_STANDOFF+.008);b.rotation.x=Math.PI/2}
 return g}
function pipeFlange(axis,sign){const g=new THREE.Group();pipeAdd(g,pCyl(.049,.049,.012,20),pipeIron);pipeAdd(g,pCyl(PIPE_R+.004,PIPE_R+.004,.014,16),pipeBrass);
 for(const x of[-.031,.031])for(const z of[-.031,.031])pipeAdd(g,pCyl(.0055,.0055,.009,6),pipeBolt,x,0,z);
 if(axis==='x'){g.rotation.z=Math.PI/2;g.position.x=sign*(PIPE_HALF-.006)}else g.position.y=sign*(PIPE_HALF-.006);return g}
function pipeRun(g,axis,sign=0){const len=sign?PIPE_HALF:PIPE_PITCH;pipeAdd(g,pCyl(PIPE_R,PIPE_R,len,20),pipeMat,axis==='x'?sign*PIPE_HALF/2:0,axis==='y'?sign*PIPE_HALF/2:0,0,axis==='x'?Math.PI/2:0)}
function buildPipeMesh(c,anchored){const root=new THREE.Group();if(anchored)root.add(pipeAnchor());
 let ports=Object.entries(c).filter(([,v])=>v).map(([d])=>d);if(!ports.length)ports=['east','west'];
 const H=ports.filter(d=>d==='east'||d==='west'),V=ports.filter(d=>d==='north'||d==='south');
 if(ports.length===2&&H.length===1&&V.length===1){const x=H[0]==='east'?1:-1,y=V[0]==='north'?1:-1;
  const ang=x===1?(y===1?Math.PI:Math.PI/2):(y===1?-Math.PI/2:0);
  pipeAdd(root,pTor(PIPE_HALF,PIPE_R,12,24,Math.PI/2),pipeMat,x*PIPE_HALF,y*PIPE_HALF,0,ang)}
 else{if(H.length===2)pipeRun(root,'x');else if(H.length===1)pipeRun(root,'x',H[0]==='east'?1:-1);
  if(V.length===2)pipeRun(root,'y');else if(V.length===1)pipeRun(root,'y',V[0]==='north'?1:-1);
  if(H.length&&V.length)pipeAdd(root,pSph(PIPE_R,12,10),pipeMat)}
 for(const d of ports)root.add(pipeFlange(d==='east'||d==='west'?'x':'y',(d==='east'||d==='north')?1:-1));
 if(ports.length===1)pipeAdd(root,pCyl(.042,.042,.014,20),pipeIron,0,0,0,H.length?Math.PI/2:0);return root}
function buildFloorOutlet(){const g=new THREE.Group();g.add(pipeAnchor());pipeRun(g,'y');g.add(pipeFlange('y',1));
 const lip=-PIPE_HALF;pipeAdd(g,pCyl(.043,.043,.046,20),pipeIron,0,lip+.023,0);pipeAdd(g,pCyl(.052,.052,.016,24),pipeIron,0,lip+.009,0);
 const rim=pipeAdd(g,pTor(.049,.005,8,24),pipeBrass,0,lip+.019,0);rim.rotation.x=Math.PI/2;return g}
function fittingBase(rot,anchored){const root=new THREE.Group();if(anchored)root.add(pipeAnchor());
 const conduit=new THREE.Group();pipeRun(conduit,'x');conduit.add(pipeFlange('x',-1),pipeFlange('x',1));conduit.rotation.z=rot*Math.PI/2;root.add(conduit);return root}
function buildValveMesh(rot,anchored){const g=fittingBase(rot,anchored);pipeAdd(g,pBox(.07,.06,.06),pipeIron);
 pipeAdd(g,pCyl(.008,.008,.07,12),pipeBrass,0,0,.045).rotation.x=Math.PI/2;pipeAdd(g,pTor(.044,.007,10,28),pipeValveRed,0,0,.08);
 pipeAdd(g,pCyl(.003,.003,.085,8),pipeIron,0,0,.08);pipeAdd(g,pCyl(.003,.003,.085,8),pipeIron,0,0,.08,Math.PI/2);return g}
function buildGaugeMesh(rot,anchored){const g=fittingBase(rot,anchored);const vert=rot%2!==0,x=vert?.105:0,y=vert?0:.105;
 const stem=pipeAdd(g,pCyl(.009,.009,.095,12),pipeBrass,x/2,y/2,.036);if(vert)stem.rotation.z=-Math.PI/2;
 pipeAdd(g,pCyl(.046,.046,.024,24),pipeIron,x,y,.055).rotation.x=Math.PI/2;pipeAdd(g,pCir(.039,32),pipeGaugeDial,x,y,.069);return g}

// Face on solid cell: 0=+X 1=-X 2=+Y 3=-Y 4=+Z 5=-Z
const PIPE_N=[[1,0,0],[-1,0,0],[0,1,0],[0,-1,0],[0,0,1],[0,0,-1]];
const PIPE_U=[[0,0,-1],[0,0,1],[1,0,0],[1,0,0],[1,0,0],[-1,0,0]];
const PIPE_V=[[0,1,0],[0,1,0],[0,0,-1],[0,0,1],[0,1,0],[0,1,0]];
const pipeDirs={north:[0,1],south:[0,-1],east:[1,0],west:[-1,0]};
const pipeOpp={north:'south',south:'north',east:'west',west:'east'};
function pipeKey(n){return n.cx+','+n.cy+','+n.cz+','+n.face+','+n.u+','+n.v}
function pipeAt(n){return pipeNodes.get(pipeKey(n))}
function pipeCenter(n){const N=PIPE_N[n.face],U=PIPE_U[n.face],V=PIPE_V[n.face],ou=(n.u-1)*PIPE_PITCH,ov=(n.v-1)*PIPE_PITCH;
 return new THREE.Vector3(n.cx+.5+N[0]*.5+U[0]*ou+V[0]*ov+N[0]*PIPE_STANDOFF,n.cy+.5+N[1]*.5+U[1]*ou+V[1]*ov+N[1]*PIPE_STANDOFF,n.cz+.5+N[2]*.5+U[2]*ou+V[2]*ov+N[2]*PIPE_STANDOFF)}
function pipeQuat(face){return new THREE.Quaternion().setFromRotationMatrix(new THREE.Matrix4().makeBasis(new THREE.Vector3(...PIPE_U[face]),new THREE.Vector3(...PIPE_V[face]),new THREE.Vector3(...PIPE_N[face])))}
function pipeStep(n,du,dv){let {cx,cy,cz,face,u,v}=n;u+=du;v+=dv;const U=PIPE_U[face],V=PIPE_V[face];
 while(u>2){u-=3;cx+=U[0];cy+=U[1];cz+=U[2]}while(u<0){u+=3;cx-=U[0];cy-=U[1];cz-=U[2]}
 while(v>2){v-=3;cx+=V[0];cy+=V[1];cz+=V[2]}while(v<0){v+=3;cx-=V[0];cy-=V[1];cz-=V[2]}
 if(!inb(cx,cy,cz)||!cells.has(key(cx,cy,cz)))return null;return {cx,cy,cz,face,u,v}}
function pipeNeighbor(n,d){const [du,dv]=pipeDirs[d],p=pipeStep(n,du,dv);return p?pipeAt(p):null}
function pipeAllowed(n,d){if(n.kind==='outlet')return d==='north';return n.kind==='pipe'||((n.rot%2===0)?(d==='east'||d==='west'):(d==='north'||d==='south'))}
function pipeConns(n){const c={north:false,south:false,east:false,west:false};
 for(const d of Object.keys(pipeDirs)){const m=pipeNeighbor(n,d);c[d]=!!m&&pipeAllowed(n,d)&&pipeAllowed(m,pipeOpp[d])}return c}
function pipeFitRot(probe,fallback=0){let h=0,v=0;for(const d of Object.keys(pipeDirs)){const m=pipeNeighbor(probe,d);
 if(!m||(m.kind!=='pipe'&&!m.auto&&!pipeAllowed(m,pipeOpp[d])))continue;(d==='east'||d==='west')?h++:v++}
 return v>h?1:h>v?0:(fallback%2)}
function pipeCanOutlet(n){return n.face!==2&&n.face!==3&&n.cy===0&&n.v===0}
function pipeShouldAnchor(n,c){if(n.kind!=='pipe')return true;const count=Object.values(c).filter(Boolean).length;if(count!==2)return true;
 if(c.east&&c.west)return n.u===1;if(c.north&&c.south)return n.v===1;return true}
function pipeRenderOne(k){const old=pipeMeshes.get(k);if(old){pipeRoot.remove(old);pipeMeshes.delete(k)}
 const n=pipeNodes.get(k);if(!n)return;const c=pipeConns(n);let mesh;
 if(n.kind==='pipe')mesh=buildPipeMesh(c,pipeShouldAnchor(n,c));
 else if(n.kind==='outlet')mesh=buildFloorOutlet();
 else if(n.kind==='valve')mesh=buildValveMesh(n.rot,true);
 else mesh=buildGaugeMesh(n.rot,true);
 mesh.position.copy(pipeCenter(n));mesh.quaternion.copy(pipeQuat(n.face));pipeRoot.add(mesh);pipeMeshes.set(k,mesh)}
function pipeSettle(n){if(!n)return;const cand=[n];for(const d of Object.keys(pipeDirs)){const m=pipeNeighbor(n,d);if(m)cand.push(m)}
 for(const node of cand)if(node&&node.kind!=='pipe'&&node.auto){const p=pipeFitRot(node,node.rot);if(node.rot%2!==p)node.rot=p}
 const touched=new Set();for(const node of cand)if(node){touched.add(pipeKey(node));
  for(const d of Object.keys(pipeDirs)){const [du,dv]=pipeDirs[d],p=pipeStep(node,du,dv);if(p)touched.add(pipeKey(p))}}
 touched.forEach(pipeRenderOne)}
function pipeSet(cell,kind,rot=0,auto=false){if(kind==='outlet'&&!pipeCanOutlet(cell))return false;
 const k=pipeKey(cell),prev=pipeNodes.get(k);
 if(prev&&prev.kind===kind&&(kind==='pipe'||kind==='outlet'||(prev.rot===rot&&!!prev.auto===!!auto)))return false;
 if(kind==='pipe'&&prev&&prev.kind!=='pipe'&&prev.kind!=='outlet')return false;
 const item={cx:cell.cx,cy:cell.cy,cz:cell.cz,face:cell.face,u:cell.u,v:cell.v,kind,rot,auto:(kind==='valve'||kind==='gauge')&&auto};
 pipeNodes.set(k,item);pipeSettle(item);return true}
function pipeDel(cell){const k=pipeKey(cell),old=pipeNodes.get(k);if(!old)return false;pipeNodes.delete(k);pipeSettle(old);return true}
function pipeClearAll(){for(const m of pipeMeshes.values())pipeRoot.remove(m);pipeMeshes.clear();pipeNodes.clear()}
function pipeRebuild(){for(const m of pipeMeshes.values())pipeRoot.remove(m);pipeMeshes.clear();for(const k of pipeNodes.keys())pipeRenderOne(k)}
function pipeSerialize(){return[...pipeNodes.values()].map(n=>({cx:n.cx,cy:n.cy,cz:n.cz,face:n.face,u:n.u,v:n.v,kind:n.kind,rot:n.rot,auto:!!n.auto}))}
function pipeLoad(arr){pipeClearAll();if(!Array.isArray(arr))return;
 for(const n of arr){if(!n||!['pipe','valve','gauge','outlet'].includes(n.kind))continue;
  if(!inb(n.cx|0,n.cy|0,n.cz|0)||(n.u|0)<0||(n.u|0)>2||(n.v|0)<0||(n.v|0)>2||(n.face|0)<0||(n.face|0)>5)continue;
  pipeNodes.set(pipeKey(n),{cx:n.cx|0,cy:n.cy|0,cz:n.cz|0,face:n.face|0,u:n.u|0,v:n.v|0,kind:n.kind,rot:((n.rot|0)%4+4)%4,auto:n.auto!==false&&(n.kind==='valve'||n.kind==='gauge')})}
 for(const n of pipeNodes.values())if(n.auto)n.rot=pipeFitRot(n,n.rot);pipeRebuild()}
function pipeHitFromPick(h){if(!h||!h.rem||!h.add)return null;
 const rem=h.rem,add=h.add,dx=add[0]-rem[0],dy=add[1]-rem[1],dz=add[2]-rem[2];
 let face=-1;if(dx===1)face=0;else if(dx===-1)face=1;else if(dy===1)face=2;else if(dy===-1)face=3;else if(dz===1)face=4;else if(dz===-1)face=5;else return null;
 const N=PIPE_N[face],U=PIPE_U[face],V=PIPE_V[face];
 const hp=h.hp||[rem[0]+.5+N[0]*.5,rem[1]+.5+N[1]*.5,rem[2]+.5+N[2]*.5];
 const fx=rem[0]+.5+N[0]*.5,fy=rem[1]+.5+N[1]*.5,fz=rem[2]+.5+N[2]*.5;
 const lx=hp[0]-fx,ly=hp[1]-fy,lz=hp[2]-fz;
 const su=lx*U[0]+ly*U[1]+lz*U[2],sv=lx*V[0]+ly*V[1]+lz*V[2];
 return{cx:rem[0],cy:rem[1],cz:rem[2],face,u:Math.max(0,Math.min(2,Math.round(su/PIPE_PITCH+1))),v:Math.max(0,Math.min(2,Math.round(sv/PIPE_PITCH+1)))}}
let pipeManual=false;
function pipePaint(cell,toolType,rotateExisting=true){if(!cell)return false;const kind=PIPE_KIND[toolType];if(!kind)return false;
 if(kind==='pipe')return pipeSet(cell,'pipe');
 if(kind==='outlet'){if(!pipeCanOutlet(cell))return false;return pipeSet(cell,'outlet')}
 const old=pipeAt(cell);
 if(old&&old.kind===kind&&rotateExisting){pipeManual=true;return pipeSet(cell,kind,(old.rot+1)%4,false)}
 const useRot=pipeManual?(rot%2):pipeFitRot({...cell,kind,rot:0,auto:true},0);
 return pipeSet(cell,kind,useRot,!pipeManual)}
function pipeFlat(n){const U=PIPE_U[n.face],V=PIPE_V[n.face];
 return[(n.cx*U[0]+n.cy*U[1]+n.cz*U[2])*3+n.u,(n.cx*V[0]+n.cy*V[1]+n.cz*V[2])*3+n.v]}
function pipeFromFlat(face,fu,fv){const U=PIPE_U[face],V=PIPE_V[face];
 // recover cell by dividing — walk from origin cell along axes is hard; use: cell offset = floor(fu/3) along U
 let ou=Math.floor(fu/3),ov=Math.floor(fv/3),u=((fu%3)+3)%3,v=((fv%3)+3)%3;
 // We need a reference: store absolute by reconstructing from a known seed is fragile.
 // Instead encode absolute U/V cell index from world axes components.
 // For axis-aligned U/V, one component of U is ±1.
 const ua=U[0]||U[1]||U[2],va=V[0]||V[1]||V[2];
 let cx=0,cy=0,cz=0;
 if(U[0])cx=ou*Math.sign(U[0]);else if(U[1])cy=ou*Math.sign(U[1]);else cz=ou*Math.sign(U[2]);
 if(V[0])cx+=ov*Math.sign(V[0]);else if(V[1])cy+=ov*Math.sign(V[1]);else cz+=ov*Math.sign(V[2]);
 // Flat coords are relative to world origin; for negative axes, floor division already handled via sign.
 // Fix for negative: Math.floor(-1/3)= -1, and (-1%3+3)%3=2 — good.
 if(fu<0&&u!==0){/* already ok via ((fu%3)+3)%3 */}
 if(!inb(cx,cy,cz))return null;
 return{cx,cy,cz,face,u,v}}
function pipeDrawLine(a,b,toolType){if(!a||!b||a.face!==b.face)return;
 const [x0,y0]=pipeFlat(a),[x1,y1]=pipeFlat(b),dx=x1-x0,dy=y1-y0,steps=Math.max(Math.abs(dx),Math.abs(dy));
 if(steps>300)return;let prev=null;
 for(let t=0;t<=steps;t++){const xi=Math.round(x0+dx*t/steps),yi=Math.round(y0+dy*t/steps);
  // Map flat index back relative to `a`
  const [ax,ay]=pipeFlat(a);let gu=xi-ax,gv=yi-ay,cur={...a};
  while(gu>0){cur=pipeStep(cur,1,0)||cur;gu--}while(gu<0){cur=pipeStep(cur,-1,0)||cur;gu++}
  while(gv>0){cur=pipeStep(cur,0,1)||cur;gv--}while(gv<0){cur=pipeStep(cur,0,-1)||cur;gv++}
  if(prev&&cur.u!==prev.u&&cur.v!==prev.v){const mid=pipeStep(prev,Math.sign(cur.u-prev.u)||0,0);if(mid)pipePaint(mid,toolType,false)}
  pipePaint(cur,toolType,false);prev=cur}}
/*PIPE_SYSTEM_END*/
