// Sparse hanging ivy over connected vertical runs — concept-art drapes, not a wall mess.
struct IvyRandom {
 uint32_t state;
 float next(){state=state*747796405u+2891336453u;uint32_t v=((state>>((state>>28u)+4u))^state)*277803737u;v=(v>>22u)^v;return (v&0x00ffffffu)/16777216.f;}
 float range(float a,float b){return a+(b-a)*next();}
};
bool sameIvy(int x,int y,int z,int r){auto c=get(x,y,z);return c.t==15&&c.r==r;}
std::array<bool,WorldCells> ivyGenerated{},ivyPatch{};
std::array<std::vector<V>,4096> ivyLeafBins;
bool ivyPatchActive=false,ivyAttachmentFound=false;
int ivyLeavesAccepted=0,ivyLeavesSkipped=0;
void ivyBeginRefresh(){ivyGenerated.fill(false);ivyPatch.fill(false);for(auto& b:ivyLeafBins)b.clear();ivyPatchActive=false;ivyLeavesAccepted=ivyLeavesSkipped=0;}
uint32_t ivyBin(int x,int y,int z){return ((uint32_t)x*73856093u^(uint32_t)y*19349663u^(uint32_t)z*83492791u)&4095u;}
bool ivyReserveLeaf(V p,float size){
 if(ghostMode)return true;
 const float bin=.14f;int x=(int)floorf(p.x/bin),y=(int)floorf(p.y/bin),z=(int)floorf(p.z/bin);
 float spacing=.105f+size*.20f; // roomier — fewer overlapping leaves
 for(int dx=-1;dx<=1;dx++)for(int dy=-1;dy<=1;dy++)for(int dz=-1;dz<=1;dz++)for(V q:ivyLeafBins[ivyBin(x+dx,y+dy,z+dz)])if(dot(p-q,p-q)<spacing*spacing){ivyLeavesSkipped++;return false;}
 ivyLeafBins[ivyBin(x,y,z)].push_back(p);ivyLeavesAccepted++;return true;
}
bool ivyOnPatch(V local){
 if(!ivyPatchActive||ghostMode)return true;
 V p=transform({local.x,local.y,0});int x=(int)floorf(p.x),y=(int)floorf(p.y),z=(int)floorf(p.z);
 return inb(x,y,z)&&ivyPatch[idx(x,y,z)];
}
V ivyAttached(V local){
 ivyAttachmentFound=false;V p=transform(local),N=transformNormal({0,0,1}),best=p;float score=1.45f*1.45f;
 int px=(int)floorf(p.x),py=(int)floorf(p.y),pz=(int)floorf(p.z);
 for(int dx=-2;dx<=2;dx++)for(int dy=-1;dy<=1;dy++)for(int dz=-2;dz<=2;dz++){
  int x=px+dx,y=py+dy,z=pz+dz;auto c=get(x,y,z);
  if(c.t<0||c.t==11||c.t==13||c.t==14||c.t==15||c.t==16||c.t==25||c.t==26||c.t==27||c.t==41||c.t==42||c.t==65||c.t==66||c.t==67)continue;
  V q{std::clamp(p.x,x+.008f,x+.992f),std::clamp(p.y,y+.008f,y+.992f),std::clamp(p.z,z+.008f,z+.992f)};
  if(N.x>.5f)q.x=x+1.f;if(N.x<-.5f)q.x=(float)x;if(N.z>.5f)q.z=z+1.f;if(N.z<-.5f)q.z=(float)z;
  // Find the actual collision surface for partial/new architectural pieces.
  if(c.t!=0&&c.t!=3&&c.t!=5&&c.t!=18&&c.t!=19&&c.t!=20){
   V outside=q+N*.025f,inside=outside;bool found=false;
   for(int i=0;i<18;i++){inside=outside-N*(i*.0625f);if(solidAt(inside)){found=true;break;}}
   if(!found)continue;V lo=inside,hi=inside+N*.0625f;
   for(int i=0;i<5;i++){V mid=(lo+hi)*.5f;if(solidAt(mid))lo=mid;else hi=mid;}q=hi;
  }
  float outward=dot(p-q,N);if(outward<-.12f)continue;
  float d=dot(p-q,p-q);if(d>=score)continue;score=d;ivyAttachmentFound=true;
  best=q+N*std::clamp(outward,.045f,.15f);
 }
 V result=best-origin;for(int i=0;i<turn;i++)result={-result.z,result.y,result.x};return result;
}
void ivyLeaf(V root,V axis,float size,IvyRandom& rng){
 root=ivyAttached(root);if(!ivyAttachmentFound&&!ghostMode)return;
 V N=norm(V{rng.range(-.5f,.5f),rng.range(-.3f,.45f),1});
 axis=norm(axis-N*dot(axis,N));V across=norm(cross(axis,N));
 float width=size*rng.range(.62f,.93f);V center=root+axis*(size*.43f)+N*(size*.10f);
 if(!ivyReserveLeaf(transform(center),size))return;
 constexpr float edge[8][2]={{0,0},{-.36f,.18f},{-.50f,.44f},{-.19f,.57f},{0,1},{.23f,.57f},{.5f,.42f},{.35f,.17f}};
 V outline[8];for(int i=0;i<8;i++)outline[i]=root+axis*(edge[i][1]*size)+across*(edge[i][0]*width);
 float saved=ambient;ambient=rng.range(.75f,1.05f);
 for(int i=0;i<8;i++)tri(center,outline[i],outline[(i+1)%8],20);
 ambient=saved;
}
void ivyBranch(V start,V finish,float phase,IvyRandom& rng,bool fineBranch=false){
 V delta=finish-start;int steps=std::max(3,(int)ceilf(len(delta)/.145f));
 auto curve=[&](float t){float envelope=sinf(t*PI);return start+delta*t+V{sinf(t*4.2f+phase)*.03f*envelope,0,.02f*envelope};};
 V prev=curve(0);float saved=ambient;ambient=.88f;
 for(int j=1;j<=steps;j++){
  float t=j/(float)steps;V point=curve(t);if(!ivyOnPatch(point)&&j>2)break;
  V pa=ivyAttached(prev);bool prevSupported=ivyAttachmentFound;V pb=ivyAttached(point);if((!prevSupported||!ivyAttachmentFound)&&!ghostMode)break;
  tube(pa,pb,fineBranch?.002f:.0032f,21);
  // sparse leaves — only every other segment, low chance
  if(j<steps&&rng.next()>(fineBranch?.25f:.16f)){
   float side=(j&2)?-1.f:1.f;
   V direction=norm(V{side*rng.range(.5f,1.f),rng.range(-.7f,.1f),rng.range(.2f,.55f)});
   float size=rng.range(fineBranch?.05f:.07f,fineBranch?.1f:.13f);
   ivyLeaf(point+V{0,0,.015f},direction,size,rng);
  }
  prev=point;
 }
 ambient=saved;
}
void ivyGrowth(){
 int x=(int)floorf(origin.x),y=(int)floorf(origin.y),z=(int)floorf(origin.z);
 if(ghostMode){
  IvyRandom rng{917u};V a{0,.42f,-.39f},b{.05f,-.2f,-.32f};
  tube(a,b,.003f,21);ivyLeaf(a+V{0,-.15f,.02f},{-.6f,-.5f,.25f},.1f,rng);
  ivyLeaf(b,{.6f,-.45f,.25f},.09f,rng);return;
 }
 if(!inb(x,y,z)||ivyGenerated[idx(x,y,z)])return;
 V along=transformNormal({1,0,0});int ax=(int)along.x,az=(int)along.z;
 std::vector<std::array<int,3>> nodes;nodes.push_back({x,y,z});ivyGenerated[idx(x,y,z)]=true;ivyPatch.fill(false);ivyPatch[idx(x,y,z)]=true;
 for(size_t head=0;head<nodes.size();head++){
  auto q=nodes[head];
  for(int du=-1;du<=1;du++)for(int dv=-1;dv<=1;dv++)if(du||dv){
   int nx=q[0]+ax*du,ny=q[1]+dv,nz=q[2]+az*du;
   if(sameIvy(nx,ny,nz,turn)&&!ivyGenerated[idx(nx,ny,nz)]){ivyGenerated[idx(nx,ny,nz)]=true;ivyPatch[idx(nx,ny,nz)]=true;nodes.push_back({nx,ny,nz});}
  }
 }
 std::sort(nodes.begin(),nodes.end(),[](const auto& a,const auto& b){return a[1]!=b[1]?a[1]>b[1]:idx(a[0],a[1],a[2])<idx(b[0],b[1],b[2]);});
 ivyPatchActive=true;
 for(auto q:nodes){
  if(sameIvy(q[0],q[1]+1,q[2],turn))continue; // only grow from top of a column
  float u=(q[0]-x)*(float)ax+(q[2]-z)*(float)az,v=(float)(q[1]-y);
  int drop=1;while(sameIvy(q[0],q[1]-drop,q[2],turn))drop++;
  IvyRandom rng{0x9135ab29u^(uint32_t)q[0]*73856093u^(uint32_t)q[1]*19349663u^(uint32_t)q[2]*83492791u^(uint32_t)turn*2654435761u};
  bool left=sameIvy(q[0]-ax,q[1],q[2]-az,turn),right=sameIvy(q[0]+ax,q[1],q[2]+az,turn);
  // small crown clump at the ledge
  V crown{u+rng.range(-.18f,.18f),v+rng.range(.44f,.48f),rng.range(-.38f,-.32f)};
  int leaves=left||right?5:7;
  for(int j=0;j<leaves;j++){
   V p=crown+V{rng.range(-.1f,.1f),rng.range(-.04f,.04f),rng.range(0,.04f)};
   ivyLeaf(p,norm(V{rng.range(-1.f,1.f),rng.range(-.6f,.15f),.35f}),rng.range(.08f,.13f),rng);
  }
  // one main hanging stem (two only on isolated columns)
  int stems=(left&&right)?1:2;
  for(int i=0;i<stems;i++){
   float px=u+(stems==1?rng.range(-.12f,.12f):(i==0?rng.range(-.28f,-.05f):rng.range(.08f,.28f)));
   float length=drop*rng.range(.88f,.98f);
   V a{px,v+.46f,rng.range(-.40f,-.35f)};
   V b{px+rng.range(-.1f,.1f),std::max(v+.5f-length,.04f-origin.y),rng.range(-.34f,-.28f)};
   ivyBranch(a,b,rng.range(0,PI*2),rng,false);
   // Branches share the same global spacing budget as neighboring stems.
   int spurs=drop>=2?std::min(3,1+drop/2):0;
   for(int spur=0;spur<spurs;spur++){
    float t=(spur+rng.range(.35f,.75f))/(spurs+.25f);V root=a+(b-a)*t;root.z+=.02f;
    float side=rng.next()<.5f?-1.f:1.f;
    V tip=root+V{side*rng.range(.22f,.44f),-rng.range(.18f,.36f),.02f};
    if(ivyOnPatch(root))ivyBranch(root,tip,rng.range(0,6.f),rng,true);
   }
  }
  // thin lateral join when neighbors share an edge
  if(right&&rng.next()>.35f){
   V a{u+.12f,v+.44f,-.36f},b{u+.95f,v+rng.range(.35f,.44f),-.34f};
   ivyBranch(a,b,rng.range(0,6.28f),rng,true);
  }
 }
 ivyPatchActive=false;
}
