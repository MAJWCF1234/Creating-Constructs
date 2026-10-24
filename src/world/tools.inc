// Build tools: box fill / erase, region select + stamp, finish painting, symmetry, build plane,
// atmosphere cycle, walker torch, and in-game construct generators. Everything edits the authored
// world through put/canPlace so footprints, saves and Share JSON stay valid.
int tool=0,mirrorMode=0,buildPlane=-1,stampRot=0;bool boxArmed=false;int boxAx=0,boxAy=0,boxAz=0;
const char* toolNames[]={"Brush","Box","Select","Paint","Stamp"};
const char* mirrorNames[]={"Mirror: off","Mirror: X","Mirror: Z","Mirror: X+Z"};
struct ClipCell{int8_t dx,dy,dz,t,r;uint16_t m;uint8_t d;};std::vector<ClipCell> clipboard;int clipW=0,clipH=0,clipD=0;
bool dayCycle=false,turntable=false,torch=false,torchLit=false;float dayPhase=.3f;uint32_t generatorSeed=1234;
void invalidate();void setMood(int m);
typedef std::array<int,3> CellPos;
std::vector<CellPos> footprintCells(int x,int y,int z,int t,int r){std::vector<CellPos> cells;eachFootprintCell(x,y,z,t,r,[&](int cx,int cy,int cz){cells.push_back({cx,cy,cz});});std::sort(cells.begin(),cells.end());return cells;}
// Mirrored or rotated multi-cell pieces move their anchor corner: pick the root whose footprint covers exactly these cells.
bool rootForCells(std::vector<CellPos> cells,int t,int r,int& ox,int& oy,int& oz){std::sort(cells.begin(),cells.end());for(auto c:cells)if(footprintCells(c[0],c[1],c[2],t,r)==cells){ox=c[0];oy=c[1];oz=c[2];return true;}return false;}
int mirrorRotation(int r,bool mx,bool mz){if(mx)r=(4-r)%4;if(mz)r=(6-r)%4;return r;}
bool placeAt(int x,int y,int z,int t,int r,int recipe){if(!canPlace(x,y,z,t,r))return false;put(x,y,z,t,r);if(recipe)world.surfaces[idx(x,y,z)]=(uint16_t)recipe;placed[idx(x,y,z)]=true;return true;}
void removeAt(int x,int y,int z){auto gone=get(x,y,z);if(gone.t<0)return;world.pipes.erase(std::remove_if(world.pipes.begin(),world.pipes.end(),[&](const PipeNode& p){return footprintCovers(x,y,z,gone.t,gone.r,p.cx,p.cy,p.cz);}),world.pipes.end());put(x,y,z,-1);}
template<class Fn>void eachMirror(Fn&& fn){if(mirrorMode&1)fn(true,false);if(mirrorMode&2)fn(false,true);if(mirrorMode==3)fn(true,true);}
CellPos mirrorCell(CellPos c,bool mx,bool mz){return {mx?world.n-1-c[0]:c[0],c[1],mz?world.n-1-c[2]:c[2]};}
void mirrorPlace(int x,int y,int z,int t,int r,int recipe){eachMirror([&](bool mx,bool mz){std::vector<CellPos> cells;for(auto c:footprintCells(x,y,z,t,r))cells.push_back(mirrorCell(c,mx,mz));int ox,oy,oz,mr=mirrorRotation(r,mx,mz);if(rootForCells(cells,t,mr,ox,oy,oz)&&!(ox==x&&oy==y&&oz==z))placeAt(ox,oy,oz,t,mr,recipe);});}
void mirrorRemove(int x,int y,int z,int t,int r){eachMirror([&](bool mx,bool mz){for(auto c:footprintCells(x,y,z,t,r)){CellPos m=mirrorCell(c,mx,mz);int ox,oy,oz;if(footprintRoot(m[0],m[1],m[2],ox,oy,oz))removeAt(ox,oy,oz);}});}
void paintAt(int x,int y,int z){if(get(x,y,z).t<0)return;int index=idx(x,y,z),recipe=surfaceRecipe();if(recipe)world.surfaces[index]=(uint16_t)recipe;else world.surfaces.erase(index);}
void paintCell(int x,int y,int z){auto c=get(x,y,z);if(c.t<0)return;paintAt(x,y,z);eachMirror([&](bool mx,bool mz){for(auto m:footprintCells(x,y,z,c.t,c.r)){CellPos q=mirrorCell(m,mx,mz);int ox,oy,oz;if(footprintRoot(q[0],q[1],q[2],ox,oy,oz))paintAt(ox,oy,oz);}});}
bool toolCursor(bool remove,int& x,int& y,int& z){if(remove){if(!hasRem)return false;x=(int)hitRem.x;y=(int)hitRem.y;z=(int)hitRem.z;}else{if(!hasAdd)return false;x=(int)hitAdd.x;y=(int)hitAdd.y;z=(int)hitAdd.z;}return inb(x,y,z);}
struct Box{int x0,y0,z0,x1,y1,z1;};
Box boxFrom(int ax,int ay,int az,int bx,int by,int bz){return {std::min(ax,bx),std::min(ay,by),std::min(az,bz),std::max(ax,bx),std::max(ay,by),std::max(az,bz)};}
int boxFill(Box b,int t,int r,int recipe){int n=0;for(int y=b.y0;y<=b.y1;y++)for(int x=b.x0;x<=b.x1;x++)for(int z=b.z0;z<=b.z1;z++)if(placeAt(x,y,z,t,r,recipe)){n++;mirrorPlace(x,y,z,t,r,recipe);}return n;}
int boxErase(Box b){int n=0;for(int y=b.y1;y>=b.y0;y--)for(int x=b.x0;x<=b.x1;x++)for(int z=b.z0;z<=b.z1;z++){int ox,oy,oz;if(!footprintRoot(x,y,z,ox,oy,oz))continue;auto c=get(ox,oy,oz);removeAt(ox,oy,oz);mirrorRemove(ox,oy,oz,c.t,c.r);n++;}return n;}
void copyRegion(Box b){clipboard.clear();clipW=b.x1-b.x0+1;clipH=b.y1-b.y0+1;clipD=b.z1-b.z0+1;
 for(int x=b.x0;x<=b.x1;x++)for(int y=b.y0;y<=b.y1;y++)for(int z=b.z0;z<=b.z1;z++){auto c=get(x,y,z);if(c.t<0||c.t==16||isPipeTool(c.t))continue;clipboard.push_back({(int8_t)(x-b.x0),(int8_t)(y-b.y0),(int8_t)(z-b.z0),c.t,c.r,(uint16_t)surfaceAt(idx(x,y,z)),(uint8_t)(world.doors.count(idx(x,y,z))?1:0)});}}
// Quarter turns about +Y; the turned footprint is shifted back so its min corner stays the stamp anchor.
CellPos stampOffset(int dx,int dy,int dz,int k){for(int i=0;i<k;i++){int nx=dz;dz=-dx;dx=nx;}if(k==1)dz+=clipW-1;if(k==2){dx+=clipW-1;dz+=clipD-1;}if(k==3)dx+=clipD-1;return {dx,dy,dz};}
template<class Fn>void eachStampPiece(int bx,int by,int bz,Fn&& fn){
 for(auto& c:clipboard){int r=(c.r+stampRot)%4;std::vector<CellPos> cells;for(auto f:footprintCells(c.dx,c.dy,c.dz,c.t,c.r)){CellPos o=stampOffset(f[0],f[1],f[2],stampRot);cells.push_back({bx+o[0],by+o[1],bz+o[2]});}int ox,oy,oz;if(rootForCells(cells,c.t,r,ox,oy,oz))fn(ox,oy,oz,(int)c.t,r,(int)c.m,(int)c.d);}
}
int stampAt(int bx,int by,int bz){int n=0;eachStampPiece(bx,by,bz,[&](int x,int y,int z,int t,int r,int m,int d){if(!placeAt(x,y,z,t,r,m))return;n++;if(d&&t==96)world.doors[idx(x,y,z)]=1;mirrorPlace(x,y,z,t,r,m);});return n;}
// Returns true when the active tool consumed the click; the brush falls through to the classic edit path.
bool toolEdit(bool remove){
 if(tool==0)return false;
 if(tool==3){if(hasRem){paintCell((int)hitRem.x,(int)hitRem.y,(int)hitRem.z);dirty=true;save();}return true;}
 if(tool==4){if(remove||clipboard.empty()){tool=0;boxArmed=false;invalidate();return true;}
  int bx,by,bz;if(toolCursor(false,bx,by,bz)){remember();if(stampAt(bx,by,bz)){audioEdit(false,clipboard[0].t,hitAdd+V{.5f,.5f,.5f});dirty=true;save();}}return true;}
 int x,y,z;if(!toolCursor(remove,x,y,z))return true;
 if(!boxArmed){boxArmed=true;boxAx=x;boxAy=y;boxAz=z;audioEvent(A_click);return true;}
 boxArmed=false;Box b=boxFrom(boxAx,boxAy,boxAz,x,y,z);
 if(tool==2){copyRegion(b);if(!clipboard.empty()){tool=4;stampRot=0;audioEvent(A_click);}else audioEvent(A_back);invalidate();return true;}
 remember();int n=remove?boxErase(b):boxFill(b,selected,rotation,surfaceRecipe());
 if(n){audioEdit(remove,remove?0:selected,V{(b.x0+b.x1+1)*.5f,(b.y0+b.y1+1)*.5f,(b.z0+b.z1+1)*.5f});dirty=true;save();}
 return true;}
// Ray / build-plane crossing between two march samples: the plane claims the click unless a piece already sits there.
bool buildPlaneHit(V a,V b){
 if(buildPlane<0||tool==3)return false;float plane=(float)buildPlane;if((a.y-plane)*(b.y-plane)>0||a.y==b.y)return false;
 float s=(plane-a.y)/(b.y-a.y);V q=a+(b-a)*s;int cx=(int)floorf(q.x),cz=(int)floorf(q.z);if(cx<0||cz<0||cx>=world.n||cz>=world.n)return false;
 int ox,oy,oz;if(footprintRoot(cx,buildPlane,cz,ox,oy,oz)){hit=hasRem=true;hasAdd=false;hitRem={(float)ox,(float)oy,(float)oz};}else{hit=hasAdd=true;hasRem=false;hitAdd={(float)cx,plane,(float)cz};}
 hitPoint=q;return true;}
void toolGuides(){
 float n=(float)world.n;
 if(buildPlane>=0){float y=buildPlane+.006f;for(int i=0;i<=world.n;i++){box(.012f,.012f,n,(float)i,y,n*.5f,10);box(n,.012f,.012f,n*.5f,y,(float)i,10);}}
 if(mirrorMode&1)box(.03f,.6f,n,n*.5f,.3f,n*.5f,10);if(mirrorMode&2)box(n,.6f,.03f,n*.5f,.3f,n*.5f,10);
}
void toolPreview(bool remove){
 std::string next;char s[128];
 if(tool==3)next=hasRem?std::string("Paint ")+surfaceNames[chosenSurface]+" onto this piece (drag to sweep)":"Paint: click a piece. Original material clears a finish";
 else if(tool==4){if(hasAdd){int bx=(int)hitAdd.x,by=(int)hitAdd.y,bz=(int)hitAdd.z,ok=0,total=0;
   eachStampPiece(bx,by,bz,[&](int x,int y,int z,int t,int r,int,int){bool valid=canPlace(x,y,z,t,r);total++;ok+=valid;origin={x+.5f,y+.5f,z+.5f};turn=r;ghostMaterial=valid?45:44;ghostMode=true;piece(t);ghostMode=false;});
   ghostMaterial=10;turn=0;origin={};sprintf_s(s,"Stamp %d of %d pieces fit. R turns, click places, Esc leaves",ok,total);next=s;}else next="Stamp: aim at the plot";}
 else if(boxArmed){int x,y,z;if(toolCursor(remove,x,y,z)){Box b=boxFrom(boxAx,boxAy,boxAz,x,y,z);previewFrame({(b.x0+b.x1+1)*.5f,(b.y0+b.y1+1)*.5f,(b.z0+b.z1+1)*.5f},b.x1-b.x0+1.02f,b.y1-b.y0+1.02f,b.z1-b.z0+1.02f,remove?44:45);
   sprintf_s(s,"%s %d x %d x %d: click to %s",tool==2?"Select":"Box",b.x1-b.x0+1,b.y1-b.y0+1,b.z1-b.z0+1,tool==2?"copy":remove?"erase":"fill");next=s;}else next="Box: aim at the far corner";}
 else next=tool==2?"Select: click the first corner of the region to copy":"Box: click the first corner. Shift or X erases the volume";
 if(next!=placementStatus){placementStatus=next;if(topPanel)InvalidateRect(topPanel,nullptr,FALSE);}
}
std::string toolSummary(){std::string s=std::to_string(pieces)+" pieces";if(tool)s+=std::string(" | ")+toolNames[tool];if(mirrorMode)s+=std::string(" | ")+mirrorNames[mirrorMode];if(buildPlane>=0)s+=" | Plane "+std::to_string(buildPlane);return s;}
void setTool(int next){tool=next;boxArmed=false;if(tool==4&&clipboard.empty())tool=0;if(tool==3)category=5;placementStatus.clear();}
// One full day lasts four minutes; the sun tracks a tilted arc and night follows dusk.
void advanceDayCycle(float dt){if(!dayCycle)return;dayPhase=fmodf(dayPhase+dt/240.f,1.f);float s=sinf(dayPhase*2*PI);sunAngle=fmodf(dayPhase*360.f+60.f,360.f);sunHeight=4+86*std::max(0.f,s);night=s<-.03f;}
void toggleDayCycle(){dayCycle=!dayCycle;if(dayCycle)dayPhase=night?.75f:asinf(std::clamp((sunHeight-4)/86.f,0.f,1.f))/(2*PI);}
// The walker's torch borrows the last fixture slot while lit; releasing it rebuilds the fixture list.
void applyTorch(){bool lit=walk&&torch&&night;if(lit){V p=eye+forward*.8f+V{0,-.3f,0};lampKinds[15]=13;gpu.lamps[15]={p.x,p.y,p.z,4.5f};torchLit=true;}else if(torchLit){torchLit=false;dirty=true;}}

struct GenRng{uint32_t s;uint32_t next(){s=s*1664525u+1013904223u;return s>>8;}int range(int a,int b){return b<=a?a:a+(int)(next()%(uint32_t)(b-a+1));}bool chance(int percent){return (int)(next()%100)<percent;}};
constexpr int GeneratorCount=5;
const char* generatorNames[]={"Brutalist tower","Monastery cloister","Terraced hillside","Ruined colonnade","Megastructure deck"};
const char* generatorNotes[]={"Setback core, board-form skin, ribbon windows and cantilevers","Arcaded walk around a courtyard pool with birches and planters","Stepped aggregate terraces, stairs, moss, pines and a still basin","Weathered columns, half walls, ivy, logs and boulders","Pilotis under a waffle deck with lightwells, cores and bridges"};
void generateWorld(int kind,uint32_t seed){
 remember();GenRng rng{seed*2654435761u+(uint32_t)kind*97u+1u};world.c.fill(Cell{});world.cables.clear();world.pipes.clear();world.surfaces.clear();world.doors.clear();resetDoorMotion();if(world.n<12)world.n=12;
 int n=world.n,c=n/2;
 auto G=[&](int x,int y,int z,int t,int r=0,int m=0){if(!canPlace(x,y,z,t,r))return false;put(x,y,z,t,r);if(m)world.surfaces[idx(x,y,z)]=(uint16_t)m;return true;};
 auto column=[&](int x,int z,int y0,int y1,int t,int r=0){for(int y=y0;y<=y1;y++)G(x,y,z,t,r);};
 auto fillRect=[&](int x0,int z0,int x1,int z1,int y,int t,int r=0,int m=0){for(int x=x0;x<=x1;x++)for(int z=z0;z<=z1;z++)G(x,y,z,t,r,m);};
 auto ring=[&](int x0,int z0,int x1,int z1,int y,int t,bool outward){for(int x=x0;x<=x1;x++)for(int z=z0;z<=z1;z++)if(x==x0||x==x1||z==z0||z==z1)G(x,y,z,t,outward?(z==z0?0:x==x0?1:z==z1?2:3):0);};
 auto facing=[&](int x,int z,int x0,int z0,int x1,int z1){return z==z0?0:x==x0?1:z==z1?2:3;}; // wall-flush local -Z toward the outside
 switch(kind){
 case 0:{int half=std::clamp(n/6,2,4),x0=c-half,x1=c+half-1,z0=c-half,z1=c+half-1,top=std::min(18,10+rng.range(0,6));
  fillRect(std::max(0,x0-2),std::max(0,z0-2),std::min(n-1,x1+2),std::min(n-1,z1+2),0,34);
  for(int y=0;y<top;y++){if(y>0&&y%5==0&&x1-x0>2){x0++;x1--;z0++;z1--;}bool ribbon=y%3==2&&y>1;
   for(int x=x0;x<=x1;x++)for(int z=z0;z<=z1;z++){bool edge=x==x0||x==x1||z==z0||z==z1,corner=(x==x0||x==x1)&&(z==z0||z==z1);
    if(!edge){if(y%5==0&&y)G(x,y,z,34);continue;}
    if(y==0&&!corner){G(x,y,z,56);continue;}
    if(ribbon&&!corner)G(x,y,z,33,facing(x,z,x0,z0,x1,z1));else G(x,y,z,47);}
   if(y%5==4&&y<top-1)for(int x=x0;x<=x1;x++){G(x,y,z0-1,48,2);G(x,y,z1+1,48,0);}
   if(y%5==2&&y>2)for(int z=z0;z<=z1;z++){G(x0-1,y,z,48,3);G(x1+1,y,z,48,1);}}
  column(c,c,1,top-1,60);ring(x0,z0,x1,z1,top,11,true);fillRect(x0+1,z0+1,x1-1,z1-1,top,35);G(c,top+1,c,79);
  for(int i=0;i<n;i++){int x=rng.range(0,n-1),z=rng.range(0,n-1);if(get(x,0,z).t<0)G(x,0,z,rng.chance(50)?76:rng.chance(50)?5:17);}
  for(int i=0;i<4;i++){int x=rng.range(1,n-2),z=rng.range(1,n-2);if(get(x,0,z).t<0)G(x,0,z,rng.chance(50)?14:41);}
  setMood(4);break;}
 case 1:{int x0=1,z0=1,x1=n-2,z1=n-2;G(c-1,0,z0,23,0);
  for(int x=x0;x<=x1;x++)for(int z=z0;z<=z1;z++){bool outer=x==x0||x==x1||z==z0||z==z1;int r=facing(x,z,x0,z0,x1,z1);
   bool inside=x>=x0+2&&x<=x1-2&&z>=z0+2&&z<=z1-2,innerEdge=inside&&(x==x0+2||x==x1-2||z==z0+2||z==z1-2),walkway=!outer&&!inside;
   if(outer){bool corner=(x==x0||x==x1)&&(z==z0||z==z1);G(x,0,z,47);G(x,1,z,!corner&&(x+z)%3==0?32:47,r);G(x,2,z,47);G(x,3,z,1);}
   else if(walkway){G(x,3,z,1);if((x+z)%4==0)G(x,2,z,37);}
   else if(innerEdge){bool corner=(x==x0+2||x==x1-2)&&(z==z0+2||z==z1-2);if(corner||(x+z)%2==0)column(x,z,0,2,7);else G(x,2,z,10,(z==z0+2||z==z1-2)?0:1);G(x,3,z,1);}
   else{bool pool=x>=c-1&&x<=c&&z>=c-1&&z<=c;if(pool)G(x,0,z,24);else if((x==x0+3||x==x1-3)&&(z==z0+3||z==z1-3))G(x,0,z,rng.chance(50)?41:14);else if((x==x0+3||x==x1-3)&&z%3==0)G(x,0,z,39,x==x0+3?3:1);else if(rng.chance(12))G(x,0,z,17);else G(x,0,z,(x+z)%2?76:5);}}
  G(c-1,0,z0-1,26);G(c+2,0,z0-1,26);setMood(2);break;}
 case 2:{int steps=std::clamp(n/4,3,6);auto height=[&](int x,int z){if(x<0||z<0||x>=n||z>=n)return -1;float d=(x+z*.6f)/(n*1.6f);return std::clamp((int)((1-d)*steps*1.6f)-1,0,steps+2);};
  for(int x=0;x<n;x++)for(int z=0;z<n;z++){int h=height(x,z);for(int y=0;y<h;y++)G(x,y,z,y==h-1?61:0);
   if(h==0){if(x>=n-5&&z>=n-5&&x<n-1&&z<n-1)G(x,0,z,40);else if(rng.chance(55))G(x,0,z,rng.chance(60)?5:76);continue;}
   int t=rng.range(0,9);if(t<4)G(x,h,z,76);else if(t<6)G(x,h,z,5);else if(t==6)G(x,h,z,86);else if(t==7&&rng.chance(45))G(x,h,z,42);else if(t==8&&rng.chance(35))G(x,h,z,rng.chance(50)?64:63);else if(t==9&&rng.chance(25))G(x,h,z,rng.chance(50)?65:66,rng.range(0,3));}
  for(int x=1;x<n;x++)for(int z=0;z<n;z++){int h=height(x,z);if(height(x-1,z)==h+1&&rng.chance(22)){removeAt(x,h,z);put(x,h,z,6,1);}else if(height(x,z-1)==h+1&&rng.chance(14)){removeAt(x,h,z);put(x,h,z,6,0);}}
  setMood(1);break;}
 case 3:{int x0=2,z0=2,x1=n-3,z1=n-3;
  for(int x=x0;x<=x1;x+=2)for(int z=z0;z<=z1;z+=3)if(rng.chance(78)){int h=rng.range(1,5);column(x,z,0,h-1,7);if(h==5&&rng.chance(65))G(x+1,5,z,12,0);if(h>=3&&rng.chance(30))G(x,h,z,5);}
  for(int x=x0;x<=x1;x++){int h=rng.range(0,3);for(int y=0;y<h;y++)G(x,y,z0-1,y==h-1&&rng.chance(50)?74:rng.chance(70)?47:0);if(h&&rng.chance(60))G(x,0,z0,15,0);if(h>1&&rng.chance(35))G(x,1,z0,15,0);}
  for(int z=z0;z<=z1;z++){int h=rng.range(0,2);for(int y=0;y<h;y++)G(x1+1,y,z,y==h-1&&rng.chance(50)?74:47);if(h&&rng.chance(50))G(x1,0,z,15,3);}
  for(int i=0;i<n*2;i++){int x=rng.range(0,n-1),z=rng.range(0,n-1);if(get(x,0,z).t>=0)continue;int t=rng.range(0,11);G(x,0,z,t<5?5:t<8?76:t==8?64:t==9?63:t==10?65:67,rng.range(0,3));}
  for(int i=0;i<5;i++){int x=rng.chance(50)?rng.range(0,1):rng.range(n-2,n-1),z=rng.range(0,n-1);if(get(x,0,z).t<0)G(x,0,z,rng.chance(50)?14:41);}
  setMood(2);break;}
 default:{int pitch=4;
  for(int x=1;x<n-1;x+=pitch)for(int z=1;z<n-1;z+=pitch)G(x,0,z,56);
  fillRect(0,0,n-1,n-1,2,50);
  for(int x=3;x<n-4;x+=pitch*2)for(int z=3;z<n-4;z+=pitch*2)if(rng.chance(60)){for(int dx=0;dx<2;dx++)for(int dz=0;dz<2;dz++)removeAt(x+dx,2,z+dz);G(x,2,z,58);}
  for(int x=1;x<n-1;x+=pitch*2)for(int z=1;z<n-1;z+=pitch*2)if(rng.chance(60)){int top=rng.range(5,9);column(x,z,3,top,60);}
  // Inhabited bridges join neighboring cores at deck level 5: two tunnel segments with floor plates between.
  for(int x=1;x<n-1;x+=pitch*2)for(int z=1;z<n-1;z+=pitch*2){if(get(x,3,z).t!=60)continue;
   if(x+pitch*2<n-1&&get(x+pitch*2,3,z).t==60){G(x+1,5,z,59,0);for(int i=3;i<=5;i++)G(x+i,5,z,34);G(x+6,5,z,59,0);}
   if(z+pitch*2<n-1&&get(x,3,z+pitch*2).t==60){G(x,5,z+1,59,3);for(int i=3;i<=5;i++)G(x,5,z+i,34);G(x,5,z+6,59,3);}}
  ring(0,0,n-1,n-1,3,11,true);
  for(int x=2;x<n-2;x+=pitch)for(int z=2;z<n-2;z+=pitch)if(get(x,3,z).t<0&&rng.chance(45))G(x,3,z,rng.chance(50)?13:80);
  for(int i=0;i<n*2;i++){int x=rng.range(0,n-1),z=rng.range(0,n-1);if(get(x,0,z).t<0)G(x,0,z,rng.chance(45)?5:rng.chance(50)?76:40);}
  setMood(0);break;}
 }
 target={n*.5f,2,n*.5f};hasNamedWalkSpawn=false;dirty=true;save();
}
