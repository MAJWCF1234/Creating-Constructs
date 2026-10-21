// Face-mounted 3×3 pipe overlay (HTML SurfaceWorks-style). Nodes live in world.pipes.
#define pipeNodes world.pipes
constexpr float PipePitch=1.f/3.f,PipeHalf=PipePitch*.5f,PipeR=.03f,PipeStandoff=.055f;
const V PipeN[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
const V PipeU[6]={{0,0,-1},{0,0,1},{1,0,0},{1,0,0},{1,0,0},{-1,0,0}};
const V PipeV[6]={{0,1,0},{0,1,0},{0,0,-1},{0,0,1},{0,1,0},{0,1,0}};
bool isPipeTool(int t){return t>=43&&t<=46;}
int pipeKindFromType(int t){return t-43;}
V pipeCenter(const PipeNode& n){
 V N=PipeN[n.face],U=PipeU[n.face],Vdir=PipeV[n.face];float ou=(n.u-1)*PipePitch,ov=(n.v-1)*PipePitch;
 return V{n.cx+.5f,n.cy+.5f,n.cz+.5f}+N*.5f+U*ou+Vdir*ov+N*PipeStandoff;
}
int findPipe(int cx,int cy,int cz,int face,int u,int v){
 for(int i=0;i<(int)pipeNodes.size();i++){auto& n=pipeNodes[i];if(n.cx==cx&&n.cy==cy&&n.cz==cz&&n.face==face&&n.u==u&&n.v==v)return i;}return -1;
}
bool pipeStep(PipeNode n,int du,int dv,PipeNode& out){
 out=n;out.u=(int8_t)(out.u+du);out.v=(int8_t)(out.v+dv);V U=PipeU[n.face],Vdir=PipeV[n.face];
 while(out.u>2){out.u-=3;out.cx+=(int8_t)U.x;out.cy+=(int8_t)U.y;out.cz+=(int8_t)U.z;}
 while(out.u<0){out.u+=3;out.cx-=(int8_t)U.x;out.cy-=(int8_t)U.y;out.cz-=(int8_t)U.z;}
 while(out.v>2){out.v-=3;out.cx+=(int8_t)Vdir.x;out.cy+=(int8_t)Vdir.y;out.cz+=(int8_t)Vdir.z;}
 while(out.v<0){out.v+=3;out.cx-=(int8_t)Vdir.x;out.cy-=(int8_t)Vdir.y;out.cz-=(int8_t)Vdir.z;}
 return inb(out.cx,out.cy,out.cz)&&get(out.cx,out.cy,out.cz).t>=0;
}
bool pipeAllowed(const PipeNode& n,int dir){ // 0N 1S 2E 3W
 if(n.kind==3)return dir==0;
 if(n.kind==0)return true;
 bool horiz=n.rot%2==0;return horiz?(dir==2||dir==3):(dir==0||dir==1);
}

// --- Oriented primitives in face local frame (U,V,N) at world origin O ---
V pipeW(V O,V U,V Vd,V N,float u,float v,float n){return O+U*u+Vd*v+N*n;}
void pipeTube(V a,V b,float r,int m){if(len(b-a)<1e-5f)return;tube(a,b,r,m);}
void pipeLocalBox(V O,V U,V Vd,V N,float wu,float wv,float wn,float u,float v,float n,int m){
 auto P=[&](float a,float b,float c){return pipeW(O,U,Vd,N,u+a,v+b,n+c);};
 float hu=wu*.5f,hv=wv*.5f,hn=wn*.5f;
 quad(P(-hu,-hv,-hn),P( hu,-hv,-hn),P( hu, hv,-hn),P(-hu, hv,-hn),m);
 quad(P(-hu,-hv, hn),P(-hu, hv, hn),P( hu, hv, hn),P( hu,-hv, hn),m);
 quad(P(-hu,-hv,-hn),P(-hu,-hv, hn),P( hu,-hv, hn),P( hu,-hv,-hn),m);
 quad(P(-hu, hv,-hn),P( hu, hv,-hn),P( hu, hv, hn),P(-hu, hv, hn),m);
 quad(P(-hu,-hv,-hn),P(-hu, hv,-hn),P(-hu, hv, hn),P(-hu,-hv, hn),m);
 quad(P( hu,-hv,-hn),P( hu,-hv, hn),P( hu, hv, hn),P( hu, hv,-hn),m);
}
void pipeDisc(V O,V U,V Vd,V N,float u,float v,float n,float r,int m,int segs=16){
 V c=pipeW(O,U,Vd,N,u,v,n);
 for(int i=0;i<segs;i++){float a0=i*2*PI/segs,a1=(i+1)*2*PI/segs;
  tri(c,c+U*(cosf(a0)*r)+Vd*(sinf(a0)*r),c+U*(cosf(a1)*r)+Vd*(sinf(a1)*r),m);}
}
void pipeRing(V O,V U,V Vd,V N,float u,float v,float n,float R,float thick,int m,int segs=16){
 // Flat torus in UV plane (wall-parallel), extruded slightly along N.
 V c=pipeW(O,U,Vd,N,u,v,n);float hn=thick*.5f;
 for(int i=0;i<segs;i++){float a0=i*2*PI/segs,a1=(i+1)*2*PI/segs;
  V r0=U*cosf(a0)+Vd*sinf(a0),r1=U*cosf(a1)+Vd*sinf(a1);
  V i0=c+r0*(R-thick),i1=c+r1*(R-thick),o0=c+r0*(R+thick),o1=c+r1*(R+thick);
  V nOff=N*hn;
  quad(i0+nOff,o0+nOff,o1+nOff,i1+nOff,m);quad(i0-nOff,i1-nOff,o1-nOff,o0-nOff,m);
  quad(o0-nOff,o0+nOff,o1+nOff,o1-nOff,m);quad(i0+nOff,i0-nOff,i1-nOff,i1+nOff,m);}
}
void pipeAnchor(V O,V U,V Vd,V N){
 // Wall plate + stem + clamp collar + bolts (matches HTML pipeAnchor).
 pipeLocalBox(O,U,Vd,N,.08f,.08f,.006f,0,0,-PipeStandoff+.003f,3);
 pipeTube(pipeW(O,U,Vd,N,0,0,-PipeStandoff+.006f),pipeW(O,U,Vd,N,0,0,-.006f),.008f,3);
 pipeRing(O,U,Vd,N,0,0,0,PipeR+.005f,.006f,3,12);
 for(float x:{-.027f,.027f})for(float y:{-.027f,.027f})
  pipeTube(pipeW(O,U,Vd,N,x,y,-PipeStandoff+.004f),pipeW(O,U,Vd,N,x,y,-PipeStandoff+.012f),.006f,8);
}
void pipeFlange(V O,V U,V Vd,V N,bool alongU,float sign){
 float pos=sign*(PipeHalf-.006f);
 V a,b,c;
 if(alongU){a=pipeW(O,U,Vd,N,pos-.006f,0,0);b=pipeW(O,U,Vd,N,pos+.006f,0,0);c=pipeW(O,U,Vd,N,pos,0,0);}
 else{a=pipeW(O,U,Vd,N,0,pos-.006f,0);b=pipeW(O,U,Vd,N,0,pos+.006f,0);c=pipeW(O,U,Vd,N,0,pos,0);}
 pipeTube(a,b,.049f,3);pipeTube(a,b,PipeR+.004f,8);
 // Bolt heads on flange face (in the plane perpendicular to the run).
 V X=alongU?Vd:U,Y=N;
 for(float s:{-.031f,.031f})for(float t:{-.031f,.031f}){
  V p=c+X*s+Y*t;pipeTube(p-N*.004f,p+N*.005f,.0055f,8);}
}
void pipeRunAxis(V O,V U,V Vd,V N,bool alongU,float sign,int m=8){
 // sign=0 full pitch; ±1 half-run toward that port.
 float len=sign?PipeHalf:PipePitch;float mid=sign?sign*PipeHalf*.5f:0;
 if(alongU)pipeTube(pipeW(O,U,Vd,N,mid-len*.5f,0,0),pipeW(O,U,Vd,N,mid+len*.5f,0,0),PipeR,m);
 else pipeTube(pipeW(O,U,Vd,N,0,mid-len*.5f,0),pipeW(O,U,Vd,N,0,mid+len*.5f,0),PipeR,m);
}
void pipeElbow(V O,V U,V Vd,V N,int xSign,int ySign){
 // Quarter-circle elbow; ports land on cell edges (HTML torus).
 V C=pipeW(O,U,Vd,N,xSign*PipeHalf,ySign*PipeHalf,0);
 // Arc from the inward U side to the inward V side.
 float a0=(xSign>0?(ySign>0?PI:PI*.5f):(ySign>0?-PI*.5f:0.f));
 const int segs=10;
 for(int i=0;i<segs;i++){float t0=a0+i*(PI*.5f)/segs,t1=a0+(i+1)*(PI*.5f)/segs;
  // Three.js torus in XY with rot.z=a0: points at (R cos θ, R sin θ) after rotation.
  // Match HTML: torus at corner, arc π/2.
  V p0=C+U*(cosf(t0)*PipeHalf)+Vd*(sinf(t0)*PipeHalf);
  V p1=C+U*(cosf(t1)*PipeHalf)+Vd*(sinf(t1)*PipeHalf);
  // Shift so the arc sits on the inner corner (toward cell center).
  // HTML torusGeometry places the tube centerline at major radius from mesh origin.
  // With position (xH,yH) and the start angle above, the free ends hit the axes:
  // Verify: for x=1,y=1,a0=π: θ=π → (-H,0) from C → world (0,H); θ=3π/2 → (0,-H) from C → (H,0). ✓
  pipeTube(p0,p1,PipeR,8);}
}
void pipeCap(V O,V U,V Vd,V N,bool alongU){
 if(alongU)pipeTube(pipeW(O,U,Vd,N,-.007f,0,0),pipeW(O,U,Vd,N,.007f,0,0),.042f,3);
 else pipeTube(pipeW(O,U,Vd,N,0,-.007f,0),pipeW(O,U,Vd,N,0,.007f,0),.042f,3);
}
bool pipeShouldAnchor(const PipeNode& n,bool north,bool south,bool east,bool west){
 if(n.kind!=0)return true;
 int count=(north?1:0)+(south?1:0)+(east?1:0)+(west?1:0);
 if(count!=2)return true;
 if(east&&west)return n.u==1;
 if(north&&south)return n.v==1;
 return true;
}
void pipeEmitValve(V O,V U,V Vd,V N,int rot){
 bool horiz=rot%2==0;
 if(horiz){pipeRunAxis(O,U,Vd,N,true,0);pipeFlange(O,U,Vd,N,true,-1);pipeFlange(O,U,Vd,N,true,1);}
 else{pipeRunAxis(O,U,Vd,N,false,0);pipeFlange(O,U,Vd,N,false,-1);pipeFlange(O,U,Vd,N,false,1);}
 pipeAnchor(O,U,Vd,N);
 pipeLocalBox(O,U,Vd,N,.07f,.06f,.06f,0,0,0,3);
 // Stem toward camera (along N) + red handwheel in UV plane.
 pipeTube(pipeW(O,U,Vd,N,0,0,.01f),pipeW(O,U,Vd,N,0,0,.08f),.008f,8);
 pipeRing(O,U,Vd,N,0,0,.08f,.044f,.007f,7,14); // brick ≈ valve red
 pipeTube(pipeW(O,U,Vd,N,-.042f,0,.08f),pipeW(O,U,Vd,N,.042f,0,.08f),.003f,3);
 pipeTube(pipeW(O,U,Vd,N,0,-.042f,.08f),pipeW(O,U,Vd,N,0,.042f,.08f),.003f,3);
}
void pipeEmitGauge(V O,V U,V Vd,V N,int rot){
 bool vert=rot%2!=0;
 if(!vert){pipeRunAxis(O,U,Vd,N,true,0);pipeFlange(O,U,Vd,N,true,-1);pipeFlange(O,U,Vd,N,true,1);}
 else{pipeRunAxis(O,U,Vd,N,false,0);pipeFlange(O,U,Vd,N,false,-1);pipeFlange(O,U,Vd,N,false,1);}
 pipeAnchor(O,U,Vd,N);
 // Stem leaves pipe perpendicular to run; dial faces out (+N) and stays upright in UV.
 float x=vert?.105f:0,y=vert?0:.105f;
 pipeTube(pipeW(O,U,Vd,N,0,0,.036f),pipeW(O,U,Vd,N,x,y,.036f),.009f,8);
 pipeTube(pipeW(O,U,Vd,N,x,y,.036f),pipeW(O,U,Vd,N,x,y,.069f),.046f,3);
 pipeDisc(O,U,Vd,N,x,y,.069f,.039f,8,20);
 // Needle
 V dial=pipeW(O,U,Vd,N,x,y,.071f);
 pipeTube(dial,dial+U*.025f+Vd*.018f,.002f,7);
}
void pipeEmitOutlet(V O,V U,V Vd,V N){
 pipeAnchor(O,U,Vd,N);
 pipeRunAxis(O,U,Vd,N,false,0); // full vertical local-V run
 pipeFlange(O,U,Vd,N,false,1);  // uphill port only
 float lip=-PipeHalf;
 pipeTube(pipeW(O,U,Vd,N,0,lip+.001f,0),pipeW(O,U,Vd,N,0,lip+.046f,0),.043f,3);
 pipeTube(pipeW(O,U,Vd,N,0,lip+.001f,0),pipeW(O,U,Vd,N,0,lip+.017f,0),.052f,3);
 pipeRing(O,U,Vd,N,0,lip+.019f,0,.049f,.005f,8,16);
}
void pipeEmitStraight(V O,V U,V Vd,V N,bool north,bool south,bool east,bool west,bool anchored){
 if(anchored)pipeAnchor(O,U,Vd,N);
 int ports=(north?1:0)+(south?1:0)+(east?1:0)+(west?1:0);
 if(!ports){east=west=true;ports=2;}
 bool H=east||west,Vports=north||south;
 if(ports==2&&H&&Vports&&!(east&&west)&&!(north&&south)){
  int xs=east?1:-1,ys=north?1:-1;pipeElbow(O,U,Vd,N,xs,ys);
 }else{
  if(east&&west)pipeRunAxis(O,U,Vd,N,true,0);
  else if(east)pipeRunAxis(O,U,Vd,N,true,1);
  else if(west)pipeRunAxis(O,U,Vd,N,true,-1);
  if(north&&south)pipeRunAxis(O,U,Vd,N,false,0);
  else if(north)pipeRunAxis(O,U,Vd,N,false,1);
  else if(south)pipeRunAxis(O,U,Vd,N,false,-1);
  if(H&&Vports){ // tee / cross join blob
   for(int i=0;i<8;i++){float a=i*2*PI/8,b=(i+1)*2*PI/8;
    V c=O;tri(c,c+U*(cosf(a)*PipeR)+Vd*(sinf(a)*PipeR),c+U*(cosf(b)*PipeR)+Vd*(sinf(b)*PipeR),8);}}
 }
 if(east)pipeFlange(O,U,Vd,N,true,1);if(west)pipeFlange(O,U,Vd,N,true,-1);
 if(north)pipeFlange(O,U,Vd,N,false,1);if(south)pipeFlange(O,U,Vd,N,false,-1);
 if(ports==1)pipeCap(O,U,Vd,N,east||west);
}
int pipeFitRot(const PipeNode& probe,int fallback);
void settlePipes(){for(auto& n:pipeNodes)if(n.autoAlign&&(n.kind==1||n.kind==2))n.rot=(int8_t)pipeFitRot(n,n.rot);}
void renderPipes(){
 settlePipes();
 for(auto& n:pipeNodes){
  V O=pipeCenter(n),U=PipeU[n.face],Vd=PipeV[n.face],N=PipeN[n.face];
  bool north=false,south=false,east=false,west=false;PipeNode q;
  const int du[4]={0,0,1,-1},dv[4]={1,-1,0,0};
  bool* flags[4]={&north,&south,&east,&west};
  for(int d=0;d<4;d++){if(!pipeStep(n,du[d],dv[d],q))continue;int j=findPipe(q.cx,q.cy,q.cz,q.face,q.u,q.v);if(j<0)continue;
   if(pipeAllowed(n,d)&&pipeAllowed(pipeNodes[j],d^1))*flags[d]=true;}
  if(n.kind==0)pipeEmitStraight(O,U,Vd,N,north,south,east,west,pipeShouldAnchor(n,north,south,east,west));
  else if(n.kind==1)pipeEmitValve(O,U,Vd,N,n.rot);
  else if(n.kind==2)pipeEmitGauge(O,U,Vd,N,n.rot);
  else pipeEmitOutlet(O,U,Vd,N);
 }
}
bool pipeHitCell(PipeNode& out){
 // Prefer add→rem face (exterior mount). Mount on the occupied cell under the cursor,
 // not footprint root — multi-cell pieces otherwise put pipes inside the volume.
 if(!hasRem)return false;
 int cx=(int)hitRem.x,cy=(int)hitRem.y,cz=(int)hitRem.z;
 // Refine to the voxel the ray actually entered when footprint root differs
 {int ix=(int)floorf(hitPoint.x),iy=(int)floorf(hitPoint.y),iz=(int)floorf(hitPoint.z);
  if(inb(ix,iy,iz)&&get(ix,iy,iz).t>=0){cx=ix;cy=iy;cz=iz;}}
 int face=-1;
 if(hasAdd){
  V d=hitAdd-V{(float)cx,(float)cy,(float)cz};
  // Only accept cardinal adjacency; otherwise fall back (avoids 2-wide footprint diagonals)
  if(fabsf(d.x)+fabsf(d.y)+fabsf(d.z)<1.1f){
   if(d.x>0)face=0;else if(d.x<0)face=1;else if(d.y>0)face=2;else if(d.y<0)face=3;else if(d.z>0)face=4;else if(d.z<0)face=5;
  }
 }
 if(face<0){
  V d=hitPoint-(V{(float)cx,(float)cy,(float)cz}+V{.5f,.5f,.5f});
  float ax=fabsf(d.x),ay=fabsf(d.y),az=fabsf(d.z);
  if(ax>=ay&&ax>=az)face=d.x>=0?0:1;
  else if(ay>=ax&&ay>=az)face=d.y>=0?2:3;
  else face=d.z>=0?4:5;
 }
 V N=PipeN[face],U=PipeU[face],Vdir=PipeV[face];
 V faceC=V{(float)cx,(float)cy,(float)cz}+V{.5f,.5f,.5f}+N*.5f;
 V local=hitPoint-faceC;
 float su=local.x*U.x+local.y*U.y+local.z*U.z,sv=local.x*Vdir.x+local.y*Vdir.y+local.z*Vdir.z;
 int u=(int)std::round(su/PipePitch+1.f);u=std::clamp(u,0,2);
 int v=(int)std::round(sv/PipePitch+1.f);v=std::clamp(v,0,2);
 out={(int8_t)cx,(int8_t)cy,(int8_t)cz,(int8_t)face,(int8_t)u,(int8_t)v,0,0,0};
 return true;
}
bool pipeManual=false;
bool pipeStrokeActive=false;
PipeNode pipeStrokeLast{};
bool pipeSameCell(const PipeNode& a,const PipeNode& b){return a.cx==b.cx&&a.cy==b.cy&&a.cz==b.cz&&a.face==b.face&&a.u==b.u&&a.v==b.v;}
int pipeFitRot(const PipeNode& probe,int fallback=0){
 int h=0,v=0;PipeNode q;const int du[4]={0,0,1,-1},dv[4]={1,-1,0,0};
 for(int d=0;d<4;d++){if(!pipeStep(probe,du[d],dv[d],q))continue;int j=findPipe(q.cx,q.cy,q.cz,q.face,q.u,q.v);if(j<0)continue;
  auto& m=pipeNodes[j];if(m.kind!=0&&!m.autoAlign&&!pipeAllowed(m,d^1))continue;(d==2||d==3)?h++:v++;}
 return v>h?1:h>v?0:(fallback%2);
}
void pipePlaceOne(PipeNode cell,int kind,bool rotateExisting){
 bool remove=wantErase();
 int idx=findPipe(cell.cx,cell.cy,cell.cz,cell.face,cell.u,cell.v);
 if(remove){if(idx>=0){PipeNode gone=pipeNodes[idx];pipeNodes.erase(pipeNodes.begin()+idx);audioEdit(true,43,pipeCenter(gone));dirty=true;}}
 else{
  if(kind==3&&!(cell.face!=2&&cell.face!=3&&cell.cy==0&&cell.v==0))return;
  if(idx>=0){
   auto& n=pipeNodes[idx];
   if(n.kind==kind&&(kind==1||kind==2)&&rotateExisting){n.rot=(n.rot+1)&3;n.autoAlign=0;pipeManual=true;}
   else if(n.kind!=0&&kind==0)return;
   else if(n.kind==kind)return;
   else{n.kind=(int8_t)kind;n.rot=(int8_t)(pipeManual&&(kind==1||kind==2)?rotation%2:pipeFitRot(n,0));n.autoAlign=(int8_t)((kind==1||kind==2)&&!pipeManual);}
  }else{
   cell.kind=(int8_t)kind;
   cell.rot=(int8_t)(pipeManual&&(kind==1||kind==2)?rotation%2:((kind==1||kind==2)?pipeFitRot(cell,0):0));
   cell.autoAlign=(int8_t)((kind==1||kind==2)&&!pipeManual);
   if(pipeNodes.size()<8000)pipeNodes.push_back(cell);
  }
  audioEdit(false,43,pipeCenter(cell));dirty=true;
 }
}
void pipeDrawLine(PipeNode a,PipeNode b,int kind){
 if(a.face!=b.face){pipePlaceOne(b,kind,false);return;}
 PipeNode cur=a;int guard=0;
 while(!pipeSameCell(cur,b)&&guard++<400){
  PipeNode next=cur;int du=0,dv=0;
  V ca=pipeCenter(cur),cb=pipeCenter(b),delta=cb-ca;
  V U=PipeU[cur.face],Vdir=PipeV[cur.face];
  float su=delta.x*U.x+delta.y*U.y+delta.z*U.z,sv=delta.x*Vdir.x+delta.y*Vdir.y+delta.z*Vdir.z;
  if(fabsf(su)>=fabsf(sv)&&fabsf(su)>1e-4f)du=su>0?1:-1;else if(fabsf(sv)>1e-4f)dv=sv>0?1:-1;else break;
  if(!pipeStep(cur,du,dv,next))break;
  pipePlaceOne(next,kind,false);cur=next;
 }
}
void pipeEdit(){
 PipeNode cell;if(!pipeHitCell(cell))return;
 int kind=pipeKindFromType(selected);
 bool first=!pipeStrokeActive;
 if(first){pipeStrokeActive=true;pipeStrokeLast=cell;pipePlaceOne(cell,kind,true);save();return;}
 if(pipeSameCell(cell,pipeStrokeLast))return;
 if(kind==0)pipeDrawLine(pipeStrokeLast,cell,kind);
 else pipePlaceOne(cell,kind,false);
 pipeStrokeLast=cell;save();
}
void pipeEndStroke(){if(pipeStrokeActive){pipeStrokeActive=false;if(dirty)save();}}
