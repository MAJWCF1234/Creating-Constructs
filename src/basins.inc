// Merged planters (17) / pools (24): face-connected cells share one rim + continuous bed.
// Wet moss beds: dirt + stone pebbles when water sits on or against moss.
bool isBasinType(int t){return t==17||t==24;}
bool isWaterPiece(int t){return t==40||t==24;}
bool mossWetAt(int x,int y,int z){
 if(get(x,y+1,z).t==40)return true;
 for(auto& d:NetDir6){
  int t=get(x+d[0],y+d[1],z+d[2]).t;
  if(t==40||t==24)return true;
 }
 return false;
}
void emitWetMossBed(){
 box(.92f,.07f,.92f,0,.43f,0,39);
 IvyRandom rng{natureSeed()^777u};
 int pebbles=3+(int)(rng.next()*4);
 for(int i=0;i<pebbles;i++){
  float px=rng.range(-.36f,.36f),pz=rng.range(-.36f,.36f),s=rng.range(.05f,.13f);
  float py=.46f+rng.range(0.f,.04f);
  // mat 38 rock / mat 39 dirt — never 19 (aggregate vs ivy billboard clash)
  if(rng.next()<.55f)box(s,s*.65f,s*rng.range(.7f,1.2f),px,py,pz,38);
  else box(s*1.15f,s*.5f,s*.9f,px,py,pz,39);
 }
}
// Fill stays inside the rim; open sides extend to the cell edge so neighbors abut cleanly.
void basinFillBounds(int x,int y,int z,const std::unordered_set<int>& occ,float rim,float& x0,float& x1,float& z0,float& z1){
 bool px=!occ.count(netPack(x+1,y,z)),nx=!occ.count(netPack(x-1,y,z));
 bool pz=!occ.count(netPack(x,y,z+1)),nz=!occ.count(netPack(x,y,z-1));
 x0=nx?-rim:-.5f;x1=px?rim:.5f;z0=nz?-rim:-.5f;z1=pz?rim:.5f;
}
void emitBasinIsland(const std::vector<NetCell>& group,int type){
 std::unordered_set<int> occ;
 for(auto c:group)occ.insert(netPack(c.x,c.y,c.z));
 // Wall occupies [.38,.50] on rim faces; keep fill clearly inside so water never draws through walls.
 const float wallH=.5f,wallT=.12f,rim=.5f-wallT-.06f; // ≈.32
 for(auto c:group){
  origin={c.x+.5f,c.y+.5f,c.z+.5f};turn=0;ambient=1;activeSurface=surfaceAt(idx(c.x,c.y,c.z));
  // Exact 1×1 floor tiles abut — no overlap / z-fight
  box(1.f,.08f,1.f,0,-.46f,0,0);
  float x0,x1,z0,z1;basinFillBounds(c.x,c.y,c.z,occ,rim,x0,x1,z0,z1);
  float cx=(x0+x1)*.5f,cz=(z0+z1)*.5f,w=x1-x0,d=z1-z0;
  if(type==17){
   box(w,.36f,d,cx,-.30f,cz,1); // soil inset from rim
  }else{
   // Pool water: TOP QUAD ONLY — never a box (side faces bled orange through rims)
   float wy=-.18f;
   if(w>.05f&&d>.05f)quad({x0,wy,z1},{x1,wy,z1},{x1,wy,z0},{x0,wy,z0},9);
  }
 }
 // Perimeter walls: FULL cell length so adjacent cells' walls meet (old .8 depth left 0.2 gaps).
 for(auto c:group){
  origin={c.x+.5f,c.y+.5f,c.z+.5f};turn=0;ambient=1;activeSurface=surfaceAt(idx(c.x,c.y,c.z));
  bool px=!occ.count(netPack(c.x+1,c.y,c.z)),nx=!occ.count(netPack(c.x-1,c.y,c.z));
  bool pz=!occ.count(netPack(c.x,c.y,c.z+1)),nz=!occ.count(netPack(c.x,c.y,c.z-1));
  float yo=-.25f;
  if(pz)box(1.f,wallH,wallT,0,yo,.5f-wallT*.5f);
  if(nz)box(1.f,wallH,wallT,0,yo,-.5f+wallT*.5f);
  if(nx)box(wallT,wallH,1.f,-.5f+wallT*.5f,yo,0);
  if(px)box(wallT,wallH,1.f,.5f-wallT*.5f,yo,0);
  // Outer corner posts seal the miter (slight overlap is fine for opaque concrete)
  float cxy=.5f-wallT*.35f,cs=wallT*1.15f;
  if(px&&pz)box(cs,wallH,cs,cxy,yo,cxy);
  if(px&&nz)box(cs,wallH,cs,cxy,yo,-cxy);
  if(nx&&pz)box(cs,wallH,cs,-cxy,yo,cxy);
  if(nx&&nz)box(cs,wallH,cs,-cxy,yo,-cxy);
 }
}
void emitBasinFamily(int type){
 std::unordered_set<int> todo;
 for(int x=0;x<world.n;x++)for(int z=0;z<world.n;z++)for(int y=0;y<20;y++)if(get(x,y,z).t==type)todo.insert(netPack(x,y,z));
 while(!todo.empty()){
  int first=*todo.begin();todo.erase(todo.begin());
  int sx,sy,sz;netUnpack(first,sx,sy,sz);
  std::vector<NetCell> group={{sx,sy,sz,type}},queue=group;
  for(size_t i=0;i<queue.size();i++){auto c=queue[i];
   for(auto& d:NetDir6){
    if(d[1]!=0)continue;
    int nx=c.x+d[0],ny=c.y+d[1],nz=c.z+d[2],nk=netPack(nx,ny,nz);
    if(todo.erase(nk)){NetCell n{nx,ny,nz,type};group.push_back(n);queue.push_back(n);}
   }
  }
  emitBasinIsland(group,type);
 }
}
void emitMergedBasins(){emitBasinFamily(17);emitBasinFamily(24);origin={};turn=0;activeSurface=0;}
