// Faceted surface nets for Moss (t==5). Face-connected moss blends; rigid cells stay cubic.
static constexpr float MossStep=.25f,MossBlend=.42f,MossRelief=.23f;
static const int MossDir6[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
static const int MossCorners[8][3]={{0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}};
static const int MossEdges[12][2]={{0,1},{2,3},{4,5},{6,7},{0,2},{1,3},{4,6},{5,7},{0,4},{1,5},{2,6},{3,7}};
static const int MossFacesC[6][4]={{0,1,3,2},{4,5,7,6},{0,1,5,4},{2,3,7,6},{0,2,6,4},{1,3,7,5}};
static const int MossFacesE[6][4]={{0,5,1,4},{2,7,3,6},{0,9,2,8},{1,11,3,10},{4,10,6,8},{5,11,7,9}};
inline int mossPack(int x,int y,int z){return (x<<10)|(z<<5)|y;}
inline void mossUnpack(int k,int& x,int& y,int& z){x=k>>10;z=(k>>5)&31;y=k&31;}
bool mossRigidType(int t){
 if(t<0||t==5)return false;
 if(t==13||t==14||t==15||t==16||t==25||t==26||t==27||t==37||t==40||t==41||t==42)return false;
 if(t>=43&&t<=46)return false;
 return true;
}
bool mossRigidAt(int x,int y,int z){
 auto c=get(x,y,z);if(c.t>=0&&mossRigidType(c.t))return true;
 int bx,by,bz;if(!footprintRoot(x,y,z,bx,by,bz))return false;return mossRigidType(get(bx,by,bz).t);
}
float mossSharpBox(float px,float py,float pz,float cx,float cy,float cz,float hx,float hy,float hz){
 float qx=fabsf(px-cx)-hx,qy=fabsf(py-cy)-hy,qz=fabsf(pz-cz)-hz;
 return sqrtf(std::max(0.f,qx)*std::max(0.f,qx)+std::max(0.f,qy)*std::max(0.f,qy)+std::max(0.f,qz)*std::max(0.f,qz))+std::min(std::max(qx,std::max(qy,qz)),0.f);
}
float mossRoundBox(float px,float py,float pz,int x,int y,int z){
 const float bevel=.105f,half=.5f-bevel;
 float qx=fabsf(px-(x+.5f))-half,qy=fabsf(py-(y+.5f))-half,qz=fabsf(pz-(z+.5f))-half;
 return sqrtf(std::max(0.f,qx)*std::max(0.f,qx)+std::max(0.f,qy)*std::max(0.f,qy)+std::max(0.f,qz)*std::max(0.f,qz))+std::min(std::max(qx,std::max(qy,qz)),0.f)-bevel;
}
float mossSmoothMin(float a,float b,float k){
 if(k<.001f)return std::min(a,b);
 float h=std::max(k-fabsf(a-b),0.f)/k;
 return std::min(a,b)-h*h*k*.25f;
}
void mossDeform(float& px,float& py,float& pz){
 if(MossRelief<.001f)return;
 float ridge=sinf(px*2.35f+pz*.81f)*.58f+cosf(pz*3.7f-px*1.31f)*.33f+sinf(px*5.3f+pz*4.6f)*.20f;
 float swell=sinf(px*.81f-pz*.67f)*cosf(pz*.93f+px*.35f)*.52f;
 float height=(ridge+swell)*MossRelief;
 float fy=py-floorf(py);
 float topWeight=std::max(0.f,std::min(1.f,(fy-.16f)/.70f));
 px+=.14f*MossRelief*sinf(py*3.2f+pz*2.1f)*topWeight;
 py-=height*topWeight;
 pz+=.14f*MossRelief*cosf(py*2.9f+px*1.9f)*topWeight;
}
struct MossCell{int x,y,z;};
void mossEmitIsland(const std::vector<MossCell>& group){
 std::unordered_set<int> occupied;
 for(auto c:group)occupied.insert(mossPack(c.x,c.y,c.z));
 struct Contact{float cx,cy,cz,hx,hy,hz;};
 std::vector<Contact> contacts;std::unordered_set<int> localConcrete;
 for(auto c:group)for(auto& d:MossDir6){
  int nx=c.x+d[0],ny=c.y+d[1],nz=c.z+d[2];if(!mossRigidAt(nx,ny,nz))continue;
  localConcrete.insert(mossPack(nx,ny,nz));
  float h=.19f,cx=c.x+.5f,cy=c.y+.5f,cz=c.z+.5f;
  contacts.push_back({cx+d[0]*(.5f-h/2),cy+d[1]*(.5f-h/2),cz+d[2]*(.5f-h/2),d[0]?h/2:.5f,d[1]?h/2:.5f,d[2]?h/2:.5f});
 }
 int lo[3]={1000,1000,1000},hi[3]={-1000,-1000,-1000};
 for(auto c:group){lo[0]=std::min(lo[0],c.x);lo[1]=std::min(lo[1],c.y);lo[2]=std::min(lo[2],c.z);hi[0]=std::max(hi[0],c.x);hi[1]=std::max(hi[1],c.y);hi[2]=std::max(hi[2],c.z);}
 const float pad=1.f;float x0=(float)lo[0]-pad,y0=(float)lo[1]-pad,z0=(float)lo[2]-pad;
 int snx=(int)ceilf((hi[0]-lo[0]+1+2*pad)/MossStep)+1;
 int sny=(int)ceilf((hi[1]-lo[1]+1+2*pad)/MossStep)+1;
 int snz=(int)ceilf((hi[2]-lo[2]+1+2*pad)/MossStep)+1;
 if(snx*sny*snz>280000)return;
 auto sidx=[&](int x,int y,int z){return (z*sny+y)*snx+x;};
 std::vector<float> samples((size_t)snx*sny*snz);
 auto field=[&](float px,float py,float pz){
  float rawX=px,rawY=py,rawZ=pz;mossDeform(px,py,pz);
  int ax=(int)floorf(px),ay=(int)floorf(py),az=(int)floorf(pz);
  struct N{int x,y,z;float d;};std::vector<N> nearby;
  for(int z=az-1;z<=az+1;z++)for(int y=ay-1;y<=ay+1;y++)for(int x=ax-1;x<=ax+1;x++)
   if(occupied.count(mossPack(x,y,z)))nearby.push_back({x,y,z,mossRoundBox(px,py,pz,x,y,z)});
  if(nearby.empty()&&contacts.empty())return 2.f;
  std::sort(nearby.begin(),nearby.end(),[](const N&a,const N&b){return a.d<b.d;});
  float value=nearby.empty()?2.f:nearby[0].d;
  if(!nearby.empty()){auto owner=nearby[0];
   for(size_t i=1;i<nearby.size();i++){int man=abs(owner.x-nearby[i].x)+abs(owner.y-nearby[i].y)+abs(owner.z-nearby[i].z);
    value=man==1?mossSmoothMin(value,nearby[i].d,MossBlend):std::min(value,nearby[i].d);}
  }
  for(auto& c:contacts){if(fabsf(rawX-c.cx)>c.hx+.55f||fabsf(rawY-c.cy)>c.hy+.55f||fabsf(rawZ-c.cz)>c.hz+.55f)continue;
   value=std::min(value,mossSharpBox(rawX,rawY,rawZ,c.cx,c.cy,c.cz,c.hx,c.hy,c.hz));}
  for(int rk:localConcrete){int x,y,z;mossUnpack(rk,x,y,z);
   if(fabsf(rawX-(x+.5f))>1||fabsf(rawY-(y+.5f))>1||fabsf(rawZ-(z+.5f))>1)continue;
   value=std::max(value,-mossSharpBox(rawX,rawY,rawZ,x+.5f,y+.5f,z+.5f,.5f,.5f,.5f));}
  return value;
 };
 for(int z=0;z<snz;z++)for(int y=0;y<sny;y++)for(int x=0;x<snx;x++)samples[sidx(x,y,z)]=field(x0+x*MossStep,y0+y*MossStep,z0+z*MossStep);
 int cx=snx-1,cy=sny-1,cz=snz-1;auto cidx=[&](int x,int y,int z){return (z*cy+y)*cx+x;};
 std::vector<int> edgeVerts((size_t)cx*cy*cz*12,-1);
 std::vector<V> positions;
 for(int z=0;z<cz;z++)for(int y=0;y<cy;y++)for(int x=0;x<cx;x++){
  float values[8];int mask=0;for(int c=0;c<8;c++){values[c]=samples[sidx(x+MossCorners[c][0],y+MossCorners[c][1],z+MossCorners[c][2])];if(values[c]<0)mask|=1<<c;}
  if(!mask||mask==255)continue;
  bool active[12];for(int e=0;e<12;e++)active[e]=(values[MossEdges[e][0]]<0)!=(values[MossEdges[e][1]]<0);
  int parent[12];for(int i=0;i<12;i++)parent[i]=i;
  auto root=[&](int i){while(parent[i]!=i){parent[i]=parent[parent[i]];i=parent[i];}return i;};
  auto link=[&](int a,int b){parent[root(a)]=root(b);};
  for(int f=0;f<6;f++){
   int crossed[4],cn=0;for(int i=0;i<4;i++)if(active[MossFacesE[f][i]])crossed[cn++]=MossFacesE[f][i];
   if(cn==2){link(crossed[0],crossed[1]);continue;}
   if(cn==4){float avg=0;for(int i=0;i<4;i++)avg+=values[MossFacesC[f][i]];avg*=.25f;bool inside=avg<0;
    for(int i=0;i<4;i++)if((values[MossFacesC[f][i]]<0)!=inside)link(MossFacesE[f][(i+3)%4],MossFacesE[f][i]);}
  }
  int seen[12]{};
  for(int e=0;e<12;e++)if(active[e]&&!seen[e]){
   int r=root(e);float vx=0,vy=0,vz=0;int n=0;std::vector<int> crossings;
   for(int j=0;j<12;j++)if(active[j]&&root(j)==r){seen[j]=1;crossings.push_back(j);
    int a=MossEdges[j][0],b=MossEdges[j][1];float va=values[a],vb=values[b],t=va/(va-vb);
    vx+=MossCorners[a][0]+(MossCorners[b][0]-MossCorners[a][0])*t;
    vy+=MossCorners[a][1]+(MossCorners[b][1]-MossCorners[a][1])*t;
    vz+=MossCorners[a][2]+(MossCorners[b][2]-MossCorners[a][2])*t;n++;}
   int id=(int)positions.size();positions.push_back({x0+(x+vx/n)*MossStep,y0+(y+vy/n)*MossStep,z0+(z+vz/n)*MossStep});
   for(int j:crossings)edgeVerts[cidx(x,y,z)*12+j]=id;
  }
 }
 auto ev=[&](int x,int y,int z,int edge)->int{if(x<0||y<0||z<0||x>=cx||y>=cy||z>=cz)return -1;return edgeVerts[cidx(x,y,z)*12+edge];};
 auto addQuad=[&](int a,int b,int c,int d,bool front){
  if(a<0||b<0||c<0||d<0)return;
  V A=positions[a],B=positions[b],C=positions[c],D=positions[d];
  origin={};turn=0;ambient=1;
  if(front){tri(A,B,C,1);tri(A,C,D,1);}else{tri(A,C,B,1);tri(A,D,C,1);}
 };
 for(int z=1;z<snz-1;z++)for(int y=1;y<sny-1;y++)for(int x=0;x<snx-1;x++){
  float a=samples[sidx(x,y,z)],b=samples[sidx(x+1,y,z)];if((a<0)==(b<0))continue;
  addQuad(ev(x,y-1,z-1,3),ev(x,y,z-1,2),ev(x,y,z,0),ev(x,y-1,z,1),a<0);
 }
 for(int z=1;z<snz-1;z++)for(int y=0;y<sny-1;y++)for(int x=1;x<snx-1;x++){
  float a=samples[sidx(x,y,z)],b=samples[sidx(x,y+1,z)];if((a<0)==(b<0))continue;
  addQuad(ev(x-1,y,z-1,7),ev(x-1,y,z,5),ev(x,y,z,4),ev(x,y,z-1,6),a<0);
 }
 for(int z=0;z<snz-1;z++)for(int y=1;y<sny-1;y++)for(int x=1;x<snx-1;x++){
  float a=samples[sidx(x,y,z)],b=samples[sidx(x,y,z+1)];if((a<0)==(b<0))continue;
  addQuad(ev(x-1,y-1,z,11),ev(x,y-1,z,10),ev(x,y,z,8),ev(x-1,y,z,9),a<0);
 }
}
void emitMossSurfaceNets(){
 std::unordered_set<int> todo;
 for(int x=0;x<world.n;x++)for(int z=0;z<world.n;z++)for(int y=0;y<20;y++)if(get(x,y,z).t==5)todo.insert(mossPack(x,y,z));
 while(!todo.empty()){
  int first=*todo.begin();todo.erase(todo.begin());
  int sx,sy,sz;mossUnpack(first,sx,sy,sz);
  std::vector<MossCell> group={{sx,sy,sz}},queue={{sx,sy,sz}};
  for(size_t i=0;i<queue.size();i++){auto c=queue[i];
   for(auto& d:MossDir6){int nk=mossPack(c.x+d[0],c.y+d[1],c.z+d[2]);
    if(todo.erase(nk)){MossCell n{c.x+d[0],c.y+d[1],c.z+d[2]};group.push_back(n);queue.push_back(n);}}
  }
  mossEmitIsland(group);
 }
}
