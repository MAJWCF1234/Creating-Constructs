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

void spherePart(V center,float radius,int material,bool hemisphere,int slices,int rings){
 float low=hemisphere?0.f:-PI/2.f;
 for(int j=0;j<rings;j++)for(int i=0;i<slices;i++){
  float u=low+(PI/2-low)*j/rings,v=low+(PI/2-low)*(j+1)/rings,a=i*2*PI/slices,b=(i+1)*2*PI/slices;
  auto p=[&](float th,float ph){return center+V{cosf(th)*cosf(ph),sinf(ph),sinf(th)*cosf(ph)}*radius;};
  if(!hemisphere&&j==0)tri(p(b,u),p(a,v),p(b,v),material);
  else if(j==rings-1)tri(p(b,u),p(a,u),p(a,v),material);
  else quad(p(b,u),p(a,u),p(a,v),p(b,v),material);
 }
}
void roundVault(){
 box(1,.5f,1,0,-.25f);
 for(int i=0;i<32;i++){
  float a=PI*i/32,b=PI*(i+1)/32;
  V p{.5f*cosf(a),.5f*sinf(a),-.5f},q{.5f*cosf(b),.5f*sinf(b),-.5f};
  V P{p.x,p.y,.5f},Q{q.x,q.y,.5f};quad(p,q,Q,P,0);
  tri({0,0,-.5f},q,p,0);tri({0,0,.5f},P,Q,0);
 }
}
void taperedObelisk(){
 for(int i=0;i<4;i++){
  float a=PI/4+PI*.5f*i,b=a+PI*.5f;
  V p{.3f*cosf(a),-.5f,.3f*sinf(a)},q{.3f*cosf(b),-.5f,.3f*sinf(b)};
  V P{.14f*cosf(a),.5f,.14f*sinf(a)},Q{.14f*cosf(b),.5f,.14f*sinf(b)};
  quad(q,p,P,Q,0);tri({0,.5f,0},Q,P,0);tri({0,-.5f,0},p,q,0);
 }
}

#include "../generated/nature_models.inc"
#include "../generated/nature_textures.inc"
bool bakeNatureMask=false;
bool natureIsBark(int material){int layer=material-22;return layer>=0&&layer<NatureTextureCount&&natureBarkLayer[layer];}
void natureVoxel(V p){int x=(int)floorf(p.x),y=(int)floorf(p.y),z=(int)floorf(p.z);if(!inb(x,y,z))return;
 int fx=std::clamp((int)((p.x-x)*4),0,3),fy=std::clamp((int)((p.y-y)*4),0,3),fz=std::clamp((int)((p.z-z)*4),0,3),bit=fx+fz*4+fy*16;
 gpu.natureMask[idx(x,y,z)*2+bit/32]|=1u<<(bit%32);
}
void stampNature(V a,V b,V c,const NatureVertex& A,const NatureVertex& B,const NatureVertex& C){
 if(!bakeNatureMask||ghostMode)return;
 int steps=std::min(32,std::max(1,(int)ceilf(std::max(len(b-a),std::max(len(c-a),len(c-b)))/.11f)));
 for(int i=0;i<=steps;i++)for(int j=0;j<=steps-i;j++){float u=i/(float)steps,v=j/(float)steps,w=1-u-v;
  float tx=A.u*w+B.u*u+C.u*v,ty=A.v*w+B.v*u+C.v*v;int ix=((int)floorf(tx*128)%128+128)%128,iy=((int)floorf(ty*128)%128+128)%128;
  if(naturePixels[A.material-22][(iy*128+ix)*4+3]>=128)natureVoxel(a*w+b*u+c*v);
 }
}
template<size_t Count>void natureModel(const NatureVertex(&mesh)[Count],V base,float scale,float angle,bool shadow=true,float width=1,bool hanging=false){
 float ca=cosf(angle),sa=sinf(angle);auto rotate=[&](V p){return V{p.x*ca+p.z*sa,p.y,-p.x*sa+p.z*ca};};
 for(size_t i=0;i<Count;i+=3){V pos[3],normal[3];for(int j=0;j<3;j++){float flip=hanging?-1.f:1.f;pos[j]=transform(base+rotate(V{mesh[i+j].p.x*width*flip,mesh[i+j].p.y*flip,mesh[i+j].p.z*width})*scale);V n=mesh[i+j].n;n.x*=flip;n.y*=flip;normal[j]=transformNormal(rotate(n));}
  // Suppress foliage buried in adjacent authored structure, preserving its openings.
  if(!natureIsBark(mesh[i].material)&&!ghostMode&&solidAt((pos[0]+pos[1]+pos[2])*(1.f/3)))continue;
  if(shadow)stampNature(pos[0],pos[1],pos[2],mesh[i],mesh[i+1],mesh[i+2]);
  for(int j=0;j<3;j++){const auto& v=mesh[i+j];vertices.push_back({pos[j],normal[j],(float)(ghostMode?10:v.material),ambient,v.u,v.v});}
 }
}
uint32_t natureSeed(){return (uint32_t)((int)(origin.x*2)*73856093u)^((int)(origin.z*2)*19349663u)^((int)(origin.y*2)*83492791u)^(uint32_t)(turn*2654435761u);}
int natureTreeVariant(uint32_t seed){IvyRandom rng{seed+59};return std::min(7,(int)(rng.next()*8));}
void natureTree(){
 uint32_t seed=natureSeed();IvyRandom rng{seed+59};
 float scale=rng.range(.64f,.77f),angle=rng.range(0,2*PI);
 switch(natureTreeVariant(seed)){
 case 0:case 1:natureModel(nature_tree_5,{0,-.5f,0},scale,angle,true,1.25f);break;
 case 2:case 3:natureModel(nature_tree_6,{0,-.5f,0},scale,angle,true,1.25f);break;
 case 4:case 5:natureModel(nature_tree_7,{0,-.5f,0},scale*.86f,angle,true,1.15f);break;
 case 6:natureModel(nature_tree_8,{0,-.5f,0},scale*.9f,angle,true,1.15f);break;
 default:natureModel(nature_tree_9,{0,-.5f,0},scale*1.15f,angle,true,1.25f);break;
 }
}
void natureBirch(){
 uint32_t seed=natureSeed();IvyRandom rng{seed+71};
 float scale=rng.range(.52f,.64f),angle=rng.range(0,2*PI);
 if(rng.next()<.5f)natureModel(nature_birch_1,{0,-.5f,0},scale,angle,true,1.1f);
 else natureModel(nature_birch_2,{0,-.5f,0},scale*1.15f,angle,true,1.15f);
}
void naturePine(){
 uint32_t seed=natureSeed();IvyRandom rng{seed+83};
 float scale=rng.range(.52f,.64f),angle=rng.range(0,2*PI);
 if(rng.next()<.5f)natureModel(nature_pine_1,{0,-.5f,0},scale,angle,true,1.15f);
 else natureModel(nature_pine_2,{0,-.5f,0},scale,angle,true,1.15f);
}
void natureLog(){IvyRandom rng{natureSeed()+91};natureModel(nature_log,{0,-.35f,0},rng.range(.85f,1.05f),rng.range(0,2*PI),true,1.f);}
void natureStump(){IvyRandom rng{natureSeed()+97};natureModel(nature_stump,{0,-.5f,0},rng.range(.9f,1.15f),rng.range(0,2*PI),true,1.f);}
void natureMushroom(){IvyRandom rng{natureSeed()+103};float s=rng.range(.55f,.85f),a=rng.range(0,2*PI);
 if(rng.next()<.5f)natureModel(nature_mushroom_1,{0,-.5f,0},s,a,false);else natureModel(nature_mushroom_2,{0,-.5f,0},s,a,false);}
void natureFern(int variant,V root,float scale,float angle){
 switch(variant%3){case 0:natureModel(nature_bracken,root,scale,angle,false);break;case 1:natureModel(nature_bracken_2,root,scale,angle,false);break;default:natureModel(nature_bracken_3,root,scale,angle,false);break;}
}
void natureGrass(int variant,V root,float scale,float angle,bool tall){
 if(tall){if(variant&1)natureModel(nature_tallgrass_2,root,scale,angle,false);else natureModel(nature_tallgrass,root,scale,angle,false);}
 else {if(variant&1)natureModel(nature_meadow_2,root,scale,angle,false);else natureModel(nature_meadow,root,scale,angle,false);}
}
void natureVariantMesh(int variant){
 V root{0,-.49f,0};switch(variant){
 case 0:natureModel(nature_bracken,root,1,0,false);break;case 1:natureModel(nature_bracken_2,root,1,0,false);break;case 2:natureModel(nature_bracken_3,root,1,0,false);break;
 case 3:natureModel(nature_meadow,root,1,0,false);break;case 4:natureModel(nature_meadow_2,root,1,0,false);break;case 5:natureModel(nature_tallgrass,root,1,0,false);break;case 6:natureModel(nature_tallgrass_2,root,1,0,false);break;
 case 7:natureModel(nature_dogbane,root,1,0,false);break;case 8:natureModel(nature_dogbane_2,root,1,0,false);break;case 9:natureModel(nature_clover,root,1,0,false);break;case 10:natureModel(nature_aster,root,1,0,false);break;case 11:natureModel(nature_goldenrod,root,1,0,false);break;case 12:natureModel(nature_shrub_1,root,1,0,false);break;case 13:natureModel(nature_shrub_2,root,1,0,false);break;case 14:natureModel(nature_shrub_3,root,1,0,false);break;case 15:natureModel(nature_birch_1,root,.6f,0,false);break;case 16:natureModel(nature_birch_2,root,.6f,0,false);break;case 17:natureModel(nature_pine_1,root,.6f,0,false);break;case 18:natureModel(nature_pine_2,root,.6f,0,false);break;case 19:natureModel(nature_mushroom_1,root,.7f,0,false);break;default:natureModel(nature_mushroom_2,root,.7f,0,false);break;}
}
void natureUnderstory(float height,bool planter){
 IvyRandom rng{natureSeed()^319u};
 float spread=planter?.29f:.44f;
 // Every tile gets a stable plant community; nearby tiles no longer repeat one recipe.
 int community=std::min(5,(int)(rng.next()*6));float fertility=rng.range(.38f,1.f);
 for(int i=0;i<(planter?4:9);i++){if(i>0&&rng.next()>fertility)continue;
  V root{rng.range(-spread,spread),height,rng.range(-spread,spread)};float a=rng.range(0,2*PI),s=rng.range(.72f,1.17f);
  int variant=std::min(5,(int)(rng.next()*6));
  if(i==0)natureFern(variant,root,s*(planter?.48f:.68f),a);
  else if(i==1)natureGrass(variant,root,s,a,false);
  else if(i==3||i==6){
   float h=s*(planter?.48f:.72f);
   switch(community){
   case 1:if(variant&1)natureModel(nature_dogbane,root,h,a,false);else natureModel(nature_dogbane_2,root,h,a,false);break;
   case 2:natureModel(nature_aster,root,h,a,false);break;
   case 3:natureModel(nature_goldenrod,root,h*.8f,a,false);break;
   default:natureGrass(variant,root,h,a,true);break;
   }
  }else if(community==4)natureModel(nature_clover,root,s*(planter?.48f:.68f),a,false);
  else if(community==5)natureFern(variant,root,s*(planter?.42f:.6f),a);
  else natureGrass(variant,root,s,a,false);
 }
}
// Water-edge / flooded moss plant set — reeds, lily pads, emergent fringe (reuses pack meshes).
void natureWaterUnderstory(float height){
 IvyRandom rng{natureSeed()^521u};
 int count=3+(int)(rng.next()*5);
 for(int i=0;i<count;i++){
  V root{rng.range(-.42f,.42f),height,rng.range(-.42f,.42f)};
  float a=rng.range(0,2*PI),s=rng.range(.75f,1.25f);int variant=std::min(5,(int)(rng.next()*6));
  int kind=(int)(rng.next()*4);
  if(kind==0)natureGrass(variant,root,s*rng.range(1.15f,1.55f),a,true); // reed cluster
  else if(kind==1)natureModel(nature_clover,{root.x,root.y-rng.range(.02f,.08f),root.z},s*rng.range(.95f,1.35f),a,false); // lily pad
  else if(kind==2){if(variant&1)natureModel(nature_dogbane,root,s*.7f,a,false);else natureModel(nature_dogbane_2,root,s*.7f,a,false);}
  else natureFern(variant,root,s*.55f,a);
 }
}

// Site vegetation follows actual surfaces instead of adding floating concrete shelves.
bool natureSupport(V start,V direction,float reach,V& result){
 bool before=solidAt(transform(start));if(before)return false;
 for(float d=.035f;d<=reach;d+=.035f){V p=start+direction*d;if(!solidAt(transform(p)))continue;V inside=p,outside=p-direction*.035f;
  for(int i=0;i<7;i++){V mid=(inside+outside)*.5f;if(solidAt(transform(mid)))inside=mid;else outside=mid;}result=outside-direction*.012f;return true;
 }return false;
}
float natureFloor(float x,float z,bool slope=false){V surface;if(natureSupport({x,-.43f,z},{0,-1,0},2.35f,surface)){
 V p=transform(surface);int cx=(int)floorf(p.x),cy=(int)floorf(p.y-.03f),cz=(int)floorf(p.z);float fx=p.x-cx,fz=p.z-cz;
 if(get(cx,cy,cz).t==17){float left=get(cx-1,cy,cz).t==17?0.f:.18f,right=get(cx+1,cy,cz).t==17?1.f:.82f,back=get(cx,cy,cz-1).t==17?0.f:.18f,front=get(cx,cy,cz+1).t==17?1.f:.82f;if(fx>left&&fx<right&&fz>back&&fz<front)return cy+.392f-origin.y;}
 return surface.y;
 }return slope?-z:-.485f;}
void natureFreeLeaf(V root,V axis,V N,float size,IvyRandom& rng){
 N=norm(N);axis=norm(axis-N*dot(axis,N));V across=norm(cross(axis,N));V center=root+axis*(size*.43f)+N*(size*.05f);
 if(!ivyReserveLeaf(transform(center),size))return;
 const float shape[8][2]={{0,0},{-.36f,.18f},{-.5f,.44f},{-.19f,.57f},{0,1},{.23f,.57f},{.5f,.42f},{.35f,.17f}};V edge[8];
 for(int i=0;i<8;i++)edge[i]=root+axis*(shape[i][1]*size)+across*(shape[i][0]*size*.75f);
 float previous=ambient;ambient*=rng.range(.82f,1.06f);for(int i=0;i<8;i++)tri(center,edge[i],edge[(i+1)%8],20);ambient=previous;
}
void natureHangingFern(int variant,V root,float size,float angle){
 switch(variant%3){case 0:natureModel(nature_bracken,root,size,angle,false,1,true);break;case 1:natureModel(nature_bracken_2,root,size,angle,false,1,true);break;default:natureModel(nature_bracken_3,root,size,angle,false,1,true);break;}
}
void natureHanging(int type){
 IvyRandom rng{natureSeed()^(uint32_t)(type*73)};bool continuation=get((int)floorf(origin.x),(int)floorf(origin.y)+1,(int)floorf(origin.z)).t==type;
 bool supported=false;V sample;for(float x:{-.3f,0.f,.3f})for(float z:{-.3f,0.f,.3f})supported=supported||natureSupport({x,.43f,z},{0,1,0},1.6f,sample);
 for(int i=0;i<4;i++){
  V root{rng.range(-.38f,.38f),.43f,rng.range(-.36f,.36f)},support;
  if(i==0){root.x=0;root.z=0;}
  if(!continuation&&natureSupport(root,{0,1,0},1.6f,support))root=support;
  else {if(supported&&!continuation)continue;root.y=.48f;}
  if(i<3&&!continuation)natureHangingFern((int)(rng.next()*3),root,rng.range(.55f,.82f),rng.range(0,2*PI));
  float length=rng.range(.65f,1.08f),phase=rng.range(0,2*PI);V previous=root;
  for(int j=1;j<=8;j++){float t=j/8.f;V p=root+V{sinf(t*3+phase)*.06f*t,-length*t,cosf(t*4+phase)*.055f*t};if(solidAt(transform(p)))break;
   tube(previous,p,.006f,21);float side=j%2?1.f:-1.f;
   natureFreeLeaf(p,norm(V{side*.65f,-.65f,.12f}),V{rng.range(-.4f,.4f),.15f,1},rng.range(.11f,.18f),rng);previous=p;
  }
 }
}
void natureGreenWall(){
 IvyRandom rng{natureSeed()^0x51u};
 for(int i=0;i<6;i++){
  float x=-.43f+i*.172f;V previous{};bool connected=false;
  for(int j=0;j<6;j++){
   V p{x+rng.range(-.045f,.045f),-.45f+j*.17f,-.43f},surface;
   if(natureSupport(p,{0,0,-1},1.35f,surface))p=surface;else p.z=-.46f;
   p.z+=.025f;if(connected)tube(previous,p,.006f,21);
   natureFreeLeaf(p,norm(V{j%2?.65f:-.65f,.7f,.08f}),V{rng.range(-.2f,.2f),rng.range(-.15f,.15f),1},rng.range(.14f,.21f),rng);previous=p;connected=true;
  }
 }
}
void natureSurfaceCover(int type){
 bool slope=type==88;const int divisions=4;
 float heights[5][5];for(int x=0;x<=divisions;x++)for(int z=0;z<=divisions;z++)heights[x][z]=natureFloor(-.49f+x*.245f,-.49f+z*.245f,slope);
 for(int ix=0;ix<divisions;ix++)for(int iz=0;iz<divisions;iz++){
  float x=-.49f+ix*.245f,z=-.49f+iz*.245f,x1=x+.245f,z1=z+.245f;
  quad({x,heights[ix][iz],z},{x,heights[ix][iz+1],z1},{x1,heights[ix+1][iz+1],z1},{x1,heights[ix+1][iz],z},1);
 }
 IvyRandom rng{natureSeed()^(uint32_t)(type*97)};
 for(int i=0;i<7;i++){
  float x=rng.range(-.38f,.38f),z=rng.range(-.38f,.38f);V root{x,natureFloor(x,z,slope)+.012f,z};float scale=rng.range(.72f,1.02f),angle=rng.range(0,2*PI);
  if(i<2)natureFern((int)(rng.next()*3),root,scale*.58f,angle);
  else if(type==91&&i==2)natureModel(nature_clover,root,scale*.72f,angle,false);
  else natureGrass((int)(rng.next()*6),root,scale,angle,i==4||i==6);
 }
}
void natureCascade(){
 bool above=faceNeighbor(77,0,1),below=faceNeighbor(77,0,-1);V face;
 float z=-.46f;if(natureSupport({0,0,-.42f},{0,0,-1},1.2f,face))z=face.z+.012f;
 // One continuous sheet per cell; stacked cells meet exactly at their boundaries.
 float left=faceNeighbor(77,-1,0)?-.5f:-.49f,right=faceNeighbor(77,1,0)?.5f:.49f,bottom=-.5f;
 auto belowCell=get((int)floorf(origin.x),(int)floorf(origin.y)-1,(int)floorf(origin.z));if(belowCell.t==40)bottom=-.65f;else if(belowCell.t==24)bottom=-1.18f;
 size_t first=transparent.size();quad({left,.5f,z},{right,.5f,z},{right,bottom,z},{left,bottom,z},9);
 for(size_t i=first;i<transparent.size();i++)transparent[i].u=2; // flow, not a pond's volume
 if(!above)quad({-.49f,.495f,z},{-.49f,.495f,z+.12f},{.49f,.495f,z+.12f},{.49f,.495f,z},9);
 if(!below){IvyRandom rng{natureSeed()^177u};for(float x:{-.39f,.39f}){V root;if(natureSupport({x,-.47f,.15f},{0,-1,0},.35f,root))natureGrass((int)(rng.next()*6),root,.7f,rng.range(0,2*PI),true);}}
}
