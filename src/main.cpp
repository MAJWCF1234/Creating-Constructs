#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <unordered_set>
#include "shaders.h"

constexpr int MaxPlot=32,WorldHeight=20,WorldCells=MaxPlot*MaxPlot*WorldHeight;
constexpr float PI=3.141592654f;
struct V {float x=0,y=0,z=0; V operator+(V b)const{return {x+b.x,y+b.y,z+b.z};} V operator-(V b)const{return {x-b.x,y-b.y,z-b.z};} V operator*(float f)const{return {x*f,y*f,z*f};}};
float dot(V a,V b){return a.x*b.x+a.y*b.y+a.z*b.z;} V cross(V a,V b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};} float len(V a){return sqrtf(dot(a,a));} V norm(V a){return a*(1/std::max(.00001f,len(a)));}
struct F4{float x,y,z,w;};
struct State{float vp[16];F4 eye,sun,sky,cfg,lamps[8],atmos,lighting;int cells[WorldCells];uint32_t natureMask[WorldCells*2];} gpu;
struct Cell{int8_t t=-1,r=0;};
struct Cable{V a,b;};
struct PipeNode{int8_t cx,cy,cz,face,u,v,kind,rot,autoAlign;};
struct World{int n=10;std::array<Cell,WorldCells> c;std::vector<Cable> cables;std::vector<PipeNode> pipes;};
World world;std::vector<World> history;
int idx(int x,int y,int z){return x*(MaxPlot*WorldHeight)+z*WorldHeight+y;} bool inb(int x,int y,int z){return x>=0&&z>=0&&y>=0&&x<world.n&&z<world.n&&y<20;}
Cell get(int x,int y,int z){return inb(x,y,z)?world.c[idx(x,y,z)]:Cell{};}
void put(int x,int y,int z,int t,int r=0){if(inb(x,y,z))world.c[idx(x,y,z)]={(int8_t)t,(int8_t)r};}
int footprintW(int t){return (t==23||t==51||t==57||t==58||t==59)?2:1;}
int footprintD(int t){return t==58?2:1;}
int footprintH(int t){return (t==23||t==51||t==56||t==58||t==59||t==60)?2:1;}
template<class Fn>void eachFootprintCell(int x,int y,int z,int t,int r,Fn&& fn){
 int w=footprintW(t),d=footprintD(t),h=footprintH(t);for(int ly=0;ly<h;ly++)for(int lz=0;lz<d;lz++)for(int lx=0;lx<w;lx++){int dx=lx,dz=lz;for(int i=0;i<r;i++){int nx=-dz;dz=dx;dx=nx;}fn(x+dx,y+ly,z+dz);}}
bool footprintCovers(int ax,int ay,int az,int t,int r,int x,int y,int z){bool hit=false;eachFootprintCell(ax,ay,az,t,r,[&](int cx,int cy,int cz){if(cx==x&&cy==y&&cz==z)hit=true;});return hit;}
bool footprintRoot(int x,int y,int z,int& ox,int& oy,int& oz){
 if(get(x,y,z).t>=0){ox=x;oy=y;oz=z;return true;}
 for(int ay=std::max(0,y-1);ay<=y;ay++)for(int ax=x-1;ax<=x+1;ax++)for(int az=z-1;az<=z+1;az++){
  if(!inb(ax,ay,az))continue;auto c=get(ax,ay,az);if(c.t<0||(footprintW(c.t)<=1&&footprintH(c.t)<=1&&footprintD(c.t)<=1))continue;
  if(footprintCovers(ax,ay,az,c.t,c.r,x,y,z)){ox=ax;oy=ay;oz=az;return true;}}
 return false;}
bool cellBlocked(int x,int y,int z){if(get(x,y,z).t>=0)return true;int ox,oy,oz;return footprintRoot(x,y,z,ox,oy,oz);}
bool canPlace(int x,int y,int z,int t,int r=0){bool ok=true;eachFootprintCell(x,y,z,t,r,[&](int cx,int cy,int cz){if(!inb(cx,cy,cz)||cellBlocked(cx,cy,cz))ok=false;});return ok;}
void remember(){if(history.size()>=80)history.erase(history.begin());history.push_back(world);}
std::wstring dataPath,logPath;FILE* logFile=nullptr;
void log(const char* s){if(logFile){fprintf(logFile,"%s\n",s);fflush(logFile);}}
void fail(const char* s){log(s);MessageBoxA(nullptr,s,"Creating Constructs",MB_ICONERROR);ExitProcess(1);}
void vkcheck(VkResult r,const char* s){if(r!=VK_SUCCESS){char b[256];sprintf_s(b,"%s (Vulkan result %d)",s,r);fail(b);}}
bool dirty=true,walk=false,night=false,rain=false,ao=true,erasing=false,drag=false,validate=false,smoke=false,ghostMode=false,hideUI=false,spaceHeld=false;
bool mouseInView=false;bool placed[WorldCells]{};float fieldOfView=60;
int selected=0,rotation=0,category=0,mood=0,pieces=0,frames=0,smokeStage=0;
float theta=.8f,phi=1.f,distanceCam=22,exposure=1.12f,fog=.38f,sunHeight=40,sunAngle=130,moodSat=0; // slightly lifted exposure
bool hasNamedWalkSpawn=false;V namedWalkSpawn;
V target{5,2,5},eye,forward,rightv,upv;float yaw=0,pitch=0,velY=0,hx=0,hz=0,wallRun=0,camRoll=0;V player{5,0,11.8f};bool grounded=false;
struct GrassBlade{V p;float rot,scale;};std::vector<GrassBlade> grass;
bool keys[256]{};int button=-1,mouseX=0,mouseY=0,lastX=0,lastY=0;
bool hit=false,hasAdd=false,hasRem=false;V hitAdd,hitRem,hitPoint,anchorA;bool pendingCable=false;
HWND mainWindow,viewWindow,topPanel,sidePanel,dockPanel,modalPanel,shareEdit;int W=1280,H=800,vw=1056,vh=690;HFONT font,fontBold,fontTitle;HBRUSH bgBrush;
#include "catalog.inc"
#include "audio.inc"
// Overcast / Noon / Golden hour / Blue hour / Concrete overcast — matched to HTML MOODS
const V moodTop[]={{.557f,.580f,.596f},{.310f,.525f,.776f},{.373f,.478f,.651f},{.110f,.165f,.314f},{.416f,.439f,.455f}};
const V moodHor[]={{.706f,.722f,.722f},{.749f,.831f,.894f},{.941f,.690f,.502f},{.498f,.561f,.690f},{.604f,.627f,.635f}};
const float moodSunI[]={2.4f,3.2f,3.4f,.9f,1.55f};
const float moodHi[]={.45f,.7f,.5f,.55f,.38f};
struct RectAction{RECT rect;int action;};std::vector<RectAction> actions;
void save(){FILE* f=nullptr;_wfopen_s(&f,dataPath.c_str(),L"wb");if(!f){log("Save failed");return;}uint32_t magic=0x334f4e4d,count=(uint32_t)world.cables.size(),pc=(uint32_t)world.pipes.size();fwrite(&magic,4,1,f);fwrite(&world.n,4,1,f);fwrite(world.c.data(),sizeof(Cell),WorldCells,f);fwrite(&count,4,1,f);if(count)fwrite(world.cables.data(),sizeof(Cable),count,f);fwrite(&pc,4,1,f);if(pc)fwrite(world.pipes.data(),sizeof(PipeNode),pc,f);fclose(f);}
bool readWorldCells(FILE* f,World& w,bool legacy){
 if(!legacy)return fread(w.c.data(),sizeof(Cell),WorldCells,f)==WorldCells;
 std::array<Cell,5120> old; if(fread(old.data(),sizeof(Cell),old.size(),f)!=old.size())return false;
 for(int x=0;x<16;x++)for(int z=0;z<16;z++)for(int y=0;y<20;y++)w.c[idx(x,y,z)]=old[x*320+z*20+y];return true;
}
bool load(){FILE* f=nullptr;_wfopen_s(&f,dataPath.c_str(),L"rb");if(!f)return false;World w;uint32_t magic=0,count=0;
 bool ok=fread(&magic,4,1,f)==1&&(magic==0x324f4e4d||magic==0x334f4e4d)&&fread(&w.n,4,1,f)==1&&w.n>=4&&w.n<=(magic==0x324f4e4d?16:MaxPlot)&&readWorldCells(f,w,magic==0x324f4e4d)&&fread(&count,4,1,f)==1&&count<=2048;
 if(ok){w.cables.resize(count);ok=!count||fread(w.cables.data(),sizeof(Cable),count,f)==count;for(auto c:w.c)if(c.t< -1||c.t>=PieceCount||c.r<0||c.r>3)ok=false;for(auto c:w.cables)if(!std::isfinite(len(c.a))||!std::isfinite(len(c.b))||len(c.a)>100||len(c.b)>100)ok=false;}
 if(ok){uint32_t pc=0;if(fread(&pc,4,1,f)==1&&pc<=8000){w.pipes.resize(pc);ok=!pc||fread(w.pipes.data(),sizeof(PipeNode),pc,f)==pc;}else w.pipes.clear();}
 fclose(f);if(ok)world=w;return ok;
}
#include "saves.inc"
void starter(){world=World{};world.n=10;world.pipes.clear();
 // Brutalist L-corridor + wave-2 megastructure accents
 for(int z=1;z<=6;z++){for(int y=0;y<4;y++){put(1,y,z,47);put(4,y,z,47);}put(2,0,z,47);put(3,0,z,47);put(2,3,z,48,2);put(3,3,z,48,2);}
 for(int x=1;x<=7;x++){for(int y=0;y<4;y++){put(x,y,7,47,1);if(x>=5)put(x,y,4,47,3);}put(x,0,5,47);put(x,0,6,47);if(x>=2&&x<=6)put(x,3,6,50);}
 for(int y=0;y<4;y++)put(1,y,4,54);
 put(1,0,2,55);put(4,3,3,52);put(2,0,1,53);put(3,0,1,53);
 put(7,0,5,51,1);put(7,0,6,-1);put(7,1,5,-1);put(7,1,6,-1);put(7,2,5,47,1);put(7,3,5,48,1); // portal 2×2 (r=1 spans +Z)
 put(2,0,3,5);put(3,0,5,5);put(5,0,6,5);put(6,1,6,14);put(0,0,3,17);
 put(8,0,4,56);put(8,1,4,-1); // pilotis
 put(0,0,6,57);put(1,0,6,-1); // megastep
 put(2,1,2,-1);put(3,1,2,-1);put(2,1,3,-1);put(3,1,3,-1);put(2,2,2,-1);put(3,2,2,-1);put(2,2,3,-1);put(3,2,3,-1);
 put(2,1,2,58); // lightwell 2×2×2
 put(5,2,5,59);put(6,2,5,-1);put(5,3,5,-1);put(6,3,5,-1); // bridge 2×2 walk tunnel
 put(8,0,7,60);put(8,1,7,-1); // tower core
 put(6,0,4,61); // aggregate
 put(2,2,4,62);put(3,2,4,62); // corbels
 world.pipes={{1,1,3,0,1,1,0,0,0},{1,2,3,0,1,1,0,0,0},{1,2,3,0,1,0,0,0,0},{1,2,3,0,1,2,1,1,1},{1,0,3,0,1,0,3,0,0}};
 mood=4;sunHeight=26;sunAngle=150;fog=.52f;moodSat=0;}

struct Vertex{V p,n;float m,a;float u=0,v=0;};std::vector<Vertex> vertices,transparent;V origin;int turn=0;float ambient=1;bool trans=false;
V transform(V p){for(int i=0;i<turn;i++)p={p.z,p.y,-p.x};return p+origin;}
// Local emitter centers match the visible fixture geometry, including rotation.
V lampEmitter(int type){
 V local=type==13?V{0,.4f,0}:type==26?V{0,.17f,0}:type==27?V{0,0,-.4f}:V{0,-.08f,0};
 return transform(local);
}

V transformNormal(V p){for(int i=0;i<turn;i++)p={p.z,p.y,-p.x};return p;}
void tri(V a,V b,V c,int m){V n=norm(cross(b-a,c-a));int mat=ghostMode?10:m;auto& v=(ghostMode||mat==2||mat==6||mat==9||mat==10||mat==14||mat==16||mat==17||mat==19)?transparent:vertices;v.push_back({transform(a),transformNormal(n),(float)mat,ambient});v.push_back({transform(b),transformNormal(n),(float)mat,ambient});v.push_back({transform(c),transformNormal(n),(float)mat,ambient});}
void triRaw(V a,V b,V c,V na,V nb,V nc,int m,float amb){auto& v=transparent;v.push_back({a,na,(float)m,amb});v.push_back({b,nb,(float)m,amb});v.push_back({c,nc,(float)m,amb});}
void quadRaw(V a,V b,V c,V d,V n,int m,float amb){triRaw(a,b,c,n,n,n,m,amb);triRaw(a,c,d,n,n,n,m,amb);}
// Crossed billboard; UV in normal.xy with V=0 at tips (texture top), V=1 at roots
void foliageBillboard(V c,float w,float h,float yaw,int mat,float amb=1){float ca=cosf(yaw),sa=sinf(yaw);V r{ca*w*.5f,0,sa*w*.5f},u{0,h,0};V bl=c-r,br=c+r,tl=bl+u,tr=br+u;
 triRaw(bl,br,tr,{0,1,0},{1,1,0},{1,0,0},mat,amb);triRaw(bl,tr,tl,{0,1,0},{1,0,0},{0,0,0},mat,amb);
 V r2{-sa*w*.5f,0,ca*w*.5f};bl=c-r2;br=c+r2;tl=bl+u;tr=br+u;
 triRaw(bl,br,tr,{0,1,0},{1,1,0},{1,0,0},mat,amb);triRaw(bl,tr,tl,{0,1,0},{1,0,0},{0,0,0},mat,amb);}
void quad(V a,V b,V c,V d,int m){tri(a,b,c,m);tri(a,c,d,m);}
void box(float w,float h,float d,float x=0,float y=0,float z=0,int m=0){float l=x-w/2,r=x+w/2,b=y-h/2,t=y+h/2,f=z-d/2,k=z+d/2;
quad({l,b,k},{r,b,k},{r,t,k},{l,t,k},m);quad({r,b,f},{l,b,f},{l,t,f},{r,t,f},m);quad({l,b,f},{l,b,k},{l,t,k},{l,t,f},m);quad({r,b,k},{r,b,f},{r,t,f},{r,t,k},m);quad({l,t,k},{r,t,k},{r,t,f},{l,t,f},m);quad({l,b,f},{r,b,f},{r,b,k},{l,b,k},m);}
void cylinder(float r,float h,float y,int m=0,int count=14){for(int i=0;i<count;i++){float a=i*2*PI/count,b=(i+1)*2*PI/count;V p{r*cosf(a),y-h/2,r*sinf(a)},q{r*cosf(b),y-h/2,r*sinf(b)},s{q.x,y+h/2,q.z},t{p.x,y+h/2,p.z};quad(q,p,t,s,m);tri({0,y+h/2,0},s,t,m);tri({0,y-h/2,0},p,q,m);}}
void leafBall(V c,float r){// leafy canopy shells (mat 16) — fern billboards were too small/broken
 for(int j=0;j<5;j++)for(int i=0;i<9;i++){float a=i*2*PI/9,b=(i+1)*2*PI/9,u=-PI/2+j*PI/5,v=u+PI/5;auto pt=[&](float th,float ph){return c+V{cosf(th)*cosf(ph),sinf(ph),sinf(th)*cosf(ph)}*r;};quad(pt(a,u),pt(b,u),pt(b,v),pt(a,v),16);}}
void tube(V a,V b,float radius,int m){V d=norm(b-a),u=norm(cross(d,fabsf(d.y)>.98f?V{1,0,0}:V{0,1,0})),v=cross(d,u);for(int i=0;i<5;i++){float x=i*2*PI/5,y=(i+1)*2*PI/5;V p=u*(cosf(x)*radius)+v*(sinf(x)*radius),q=u*(cosf(y)*radius)+v*(sinf(y)*radius);quad(a+p,b+p,b+q,a+q,m);}}
bool solidAt(V p);
#include "ivy.inc"
#include "shape_helpers.inc"
#include "nature.inc"
#include "pipes.inc"
void waterSurface(){
 int x=(int)floorf(origin.x),y=(int)floorf(origin.y),z=(int)floorf(origin.z);bool above=get(x,y+1,z).t==40;float top=above?.5f:.35f;
 if(!above)quad({-.5f,top,.5f},{.5f,top,.5f},{.5f,top,-.5f},{-.5f,top,-.5f},9);
 if(get(x+1,y,z).t!=40)quad({.5f,-.5f,.5f},{.5f,-.5f,-.5f},{.5f,top,-.5f},{.5f,top,.5f},9);
 if(get(x-1,y,z).t!=40)quad({-.5f,-.5f,-.5f},{-.5f,-.5f,.5f},{-.5f,top,.5f},{-.5f,top,-.5f},9);
 if(get(x,y,z+1).t!=40)quad({-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,top,.5f},{-.5f,top,.5f},9);
 if(get(x,y,z-1).t!=40)quad({.5f,-.5f,-.5f},{-.5f,-.5f,-.5f},{-.5f,top,-.5f},{.5f,top,-.5f},9);
}
void piece(int t){switch(t){
case 0:box(1,1,1);break;case 1:box(1,.5,1,0,-.25);break;case 2:box(.4,1,.4);break;case 3:box(1,1,1,0,0,0,2);break;
case 4:quad({-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,-.5f},{-.5f,.5f,-.5f},0);quad({.5f,-.5f,-.5f},{-.5f,-.5f,-.5f},{-.5f,.5f,-.5f},{.5f,.5f,-.5f},0);tri({-.5f,-.5f,-.5f},{-.5f,-.5f,.5f},{-.5f,.5f,-.5f},0);tri({.5f,-.5f,.5f},{.5f,-.5f,-.5f},{.5f,.5f,-.5f},0);quad({-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,-.5f,.5f},{-.5f,-.5f,.5f},0);break;
case 5:box(1,1,1,0,0,0,1);break;
case 6:for(int i=0;i<4;i++)box(1,.25f*(i+1),.25,0,-.5f+.125f*(i+1),.375f-.25f*i);break;
case 7:cylinder(.2,1,0);cylinder(.3,.12,-.44);cylinder(.3,.12,.44);break;
case 8:box(.15,1,.3,-.425);box(.15,1,.3,.425);box(.7,.2,.3,0,.4);box(.7,.25,.3,0,-.375);box(.7,.55,.04,0,.025,0,2);break;
case 9:box(1,1,.3);break;
case 10:{box(.25,1,1,-.375);box(.25,1,1,.375);for(int i=0;i<12;i++){float a=PI*i/12,b=PI*(i+1)/12;V p{.25f*cosf(a),.25f*sinf(a),-.5f},q{.25f*cosf(b),.25f*sinf(b),-.5f},P{p.x,.5f,-.5f},Q{q.x,.5f,-.5f};quad(p,q,Q,P,0);p.z=q.z=P.z=Q.z=.5f;quad(q,p,P,Q,0);quad({.25f*cosf(a),.25f*sinf(a),-.5f},{.25f*cosf(a),.25f*sinf(a),.5f},{.25f*cosf(b),.25f*sinf(b),.5f},{.25f*cosf(b),.25f*sinf(b),-.5f},0);}box(.5,.01,1,0,.495);break;}
case 11:box(1,.06,.06,0,.42,-.4,3);box(1,.04,.04,0,.2,-.4,3);for(float x:{-.47f,0.f,.47f})box(.06,.9,.06,x,-.05,-.4,3);break;
case 12:box(1,.3,.3,0,.35);break;
case 13:box(.6,.07,.07,0,.4,0,4);box(.66,.04,.14,0,.46,0,3);break;
case 14:natureTree();break;
case 15:ivyGrowth();break;
case 16:for(int i=0;i<12;i++){float a=i/12.f,b=(i+1)/12.f;tube({a-.5f,.4f-.8f*a*(1-a),0},{b-.5f,.4f-.8f*b*(1-b),0},.014,13);}break;
case 17:case 24:box(1,.5,.1,0,-.25,-.45);box(1,.5,.1,0,-.25,.45);box(.1,.5,.8,-.45,-.25);box(.1,.5,.8,.45,-.25);box(.8,.1,.8,0,-.45);box(.8,t==17?.42f:.02f,.8,0,t==17?-.29f:-.14f,0,t==17?1:9);break;
case 18:box(1,1,1,0,0,0,7);break;case 19:box(1,1,1,0,0,0,5);break;case 20:box(1,1,1,0,0,0,8);break;
case 21:for(int i=0;i<5;i++)box(.08,1,.5,-.4f+i*.2f,0,-.2);break;
case 22:for(int i=0;i<5;i++){box(.08,1,.25,-.4f+i*.2f);box(1,.08,.25,0,-.4f+i*.2f);}break;
case 23: // 2×2 doorway — jambs deepened toward interior (-Z)
 box(.3,2,.55,-.35,.5,-.15);box(.3,2,.55,1.35,.5,-.15);box(1.4,.18,.55,.5,1.41,-.15);break;
case 25:box(.06,1,.06,-.25,0,-.44,3);box(.06,1,.06,.25,0,-.44,3);for(int i=0;i<4;i++)box(.5,.04,.04,0,-.4f+i*.25f,-.44,3);break;
case 26:cylinder(.1,.6,-.2,3);cylinder(.09,.12,.17,4);break;
case 27:box(.2,.08,.06,0,.3,-.46,3);box(.2,.08,.06,0,-.3,-.46,3);box(.04,.6,.04,0,0,-.46,3);box(.1,.5,.1,0,0,-.4,4);break;
#include "modern_pieces.inc"
}}

#include "thumbnails.inc"
#include "moss_nets.inc"

VkInstance instance;VkPhysicalDevice physical;VkDevice device;VkSurfaceKHR surface;VkQueue queue;uint32_t family;
VkSwapchainKHR swapchain;VkFormat swapFormat;VkExtent2D extent;std::vector<VkImage> images;std::vector<VkImageView> views;std::vector<VkFramebuffer> framebuffers;
VkRenderPass renderPass;VkPipelineLayout pipelineLayout;VkPipeline pipeline,skyPipeline,waterPipeline;VkDescriptorSetLayout descriptorLayout;VkDescriptorPool descriptorPool;VkDescriptorSet descriptorSet;
VkCommandPool commandPool;VkCommandBuffer command;VkSemaphore acquired,rendered;std::vector<VkSemaphore> presentReady;VkFence fence;
VkBuffer vertexBuffer,stateBuffer;VkDeviceMemory vertexMemory,stateMemory,depthMemory;void* vertexMap;void* stateMap;
VkImage depthImage,msaaImage,texImage;VkImageView depthView,msaaView,texView;VkDeviceMemory msaaMemory,texMemory;VkSampler texSampler;
VkSampleCountFlagBits samples=VK_SAMPLE_COUNT_1_BIT;bool rebuildSwap=false;uint32_t vertexCount=0,staticCount=0,waterVertexCount=0;const size_t maxVertexBytes=128*1024*1024;
#include "texbake.inc"
VkDebugUtilsMessengerEXT debugMessenger;int validationErrors=0;std::string gpuName;
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,VkDebugUtilsMessageTypeFlagsEXT,const VkDebugUtilsMessengerCallbackDataEXT* data,void*){if(severity>=VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){log(data->pMessage);if(severity>=VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)validationErrors++;}return VK_FALSE;}
uint32_t memoryType(uint32_t bits,VkMemoryPropertyFlags flags){VkPhysicalDeviceMemoryProperties p;vkGetPhysicalDeviceMemoryProperties(physical,&p);for(uint32_t i=0;i<p.memoryTypeCount;i++)if((bits&(1u<<i))&&(p.memoryTypes[i].propertyFlags&flags)==flags)return i;fail("No compatible Vulkan memory type");return 0;}
void buffer(VkDeviceSize size,VkBufferUsageFlags usage,VkBuffer& b,VkDeviceMemory& memory,void** map){VkBufferCreateInfo ci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};ci.size=size;ci.usage=usage;vkcheck(vkCreateBuffer(device,&ci,nullptr,&b),"Create buffer");VkMemoryRequirements req;vkGetBufferMemoryRequirements(device,b,&req);VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.allocationSize=req.size;ai.memoryTypeIndex=memoryType(req.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);vkcheck(vkAllocateMemory(device,&ai,nullptr,&memory),"Allocate buffer");vkcheck(vkBindBufferMemory(device,b,memory,0),"Bind buffer");vkcheck(vkMapMemory(device,memory,0,size,0,map),"Map buffer");}
#include "foliage_trace.inc"
#include "foliage_tex.inc"
void uploadTextures(){
 constexpr int TextureLayers=9+NatureTextureCount;
 unsigned char layers[TextureLayers][128*128*4];
 bakeConcrete(layers[0]);bakeLeaf(layers[1],false);bakeBrick(layers[3]);bakeWood(layers[4]);bakeSteel(layers[5]);
 // Optimum vine/ivy (assets/optimum → foliage_tex.inc); PSX grass; ivy replaces fern layer for leaves
 memcpy(layers[2],vine_rgba,sizeof(vine_rgba));
 memcpy(layers[6],grass_rgba,sizeof(grass_rgba));
 memcpy(layers[7],ivy_rgba,sizeof(ivy_rgba));for(int i=0;i<NatureTextureCount;i++)memcpy(layers[8+i],naturePixels[i],128*128*4);
 bakeBoardForm(layers[8+NatureTextureCount]);
 VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};ii.imageType=VK_IMAGE_TYPE_2D;ii.format=VK_FORMAT_R8G8B8A8_UNORM;ii.extent={128,128,1};ii.mipLevels=1;ii.arrayLayers=TextureLayers;ii.samples=VK_SAMPLE_COUNT_1_BIT;ii.tiling=VK_IMAGE_TILING_OPTIMAL;ii.usage=VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
 vkcheck(vkCreateImage(device,&ii,nullptr,&texImage),"tex image");VkMemoryRequirements req;vkGetImageMemoryRequirements(device,texImage,&req);
 VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.allocationSize=req.size;ai.memoryTypeIndex=memoryType(req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
 vkcheck(vkAllocateMemory(device,&ai,nullptr,&texMemory),"tex mem");vkcheck(vkBindImageMemory(device,texImage,texMemory,0),"tex bind");
 VkBuffer staging;VkDeviceMemory stagingMem;void* stagingMap;buffer(128ull*128*4*TextureLayers,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,staging,stagingMem,&stagingMap);
 memcpy(stagingMap,layers,sizeof(layers));
 VkCommandBufferAllocateInfo ca{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ca.commandPool=commandPool;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;VkCommandBuffer cmd;vkcheck(vkAllocateCommandBuffers(device,&ca,&cmd),"tex cmd");
 VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};bi.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;vkBeginCommandBuffer(cmd,&bi);
 VkImageMemoryBarrier bar{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};bar.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED;bar.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;bar.srcQueueFamilyIndex=bar.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;bar.image=texImage;bar.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,TextureLayers};bar.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
 vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&bar);
 VkBufferImageCopy regions[TextureLayers]{};for(int i=0;i<TextureLayers;i++){regions[i].bufferOffset=(VkDeviceSize)i*128*128*4;regions[i].imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,(uint32_t)i,1};regions[i].imageExtent={128,128,1};}
 vkCmdCopyBufferToImage(cmd,staging,texImage,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,TextureLayers,regions);
 bar.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;bar.newLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;bar.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;bar.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
 vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,nullptr,0,nullptr,1,&bar);
 vkEndCommandBuffer(cmd);VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};si.commandBufferCount=1;si.pCommandBuffers=&cmd;vkQueueSubmit(queue,1,&si,VK_NULL_HANDLE);vkQueueWaitIdle(queue);
 vkFreeCommandBuffers(device,commandPool,1,&cmd);vkUnmapMemory(device,stagingMem);vkDestroyBuffer(device,staging,nullptr);vkFreeMemory(device,stagingMem,nullptr);
 VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};vi.image=texImage;vi.viewType=VK_IMAGE_VIEW_TYPE_2D_ARRAY;vi.format=VK_FORMAT_R8G8B8A8_UNORM;vi.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,TextureLayers};
 vkcheck(vkCreateImageView(device,&vi,nullptr,&texView),"tex view");
 VkSamplerCreateInfo sp{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};sp.magFilter=sp.minFilter=VK_FILTER_LINEAR;sp.addressModeU=sp.addressModeV=sp.addressModeW=VK_SAMPLER_ADDRESS_MODE_REPEAT;
 vkcheck(vkCreateSampler(device,&sp,nullptr,&texSampler),"tex sampler");
}
VkImageView imageView(VkImage im,VkFormat format,VkImageAspectFlags aspect){VkImageViewCreateInfo ci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};ci.image=im;ci.viewType=VK_IMAGE_VIEW_TYPE_2D;ci.format=format;ci.subresourceRange={aspect,0,1,0,1};VkImageView v;vkcheck(vkCreateImageView(device,&ci,nullptr,&v),"Image view");return v;}
VkShaderModule shader(const unsigned char* bytes,size_t size){VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};ci.codeSize=size;ci.pCode=reinterpret_cast<const uint32_t*>(bytes);VkShaderModule m;vkcheck(vkCreateShaderModule(device,&ci,nullptr,&m),"Shader module");return m;}
#include "postpass.inc"
void destroySwap(){for(auto semaphore:presentReady)vkDestroySemaphore(device,semaphore,nullptr);presentReady.clear();for(auto f:framebuffers)vkDestroyFramebuffer(device,f,nullptr);framebuffers.clear();destroySceneTargets();for(auto v:views)vkDestroyImageView(device,v,nullptr);views.clear();if(swapchain)vkDestroySwapchainKHR(device,swapchain,nullptr);}
void createSwap(){VkSurfaceCapabilitiesKHR cap;vkcheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical,surface,&cap),"Surface capabilities");extent=cap.currentExtent;if(extent.width==UINT32_MAX)extent={(uint32_t)vw,(uint32_t)vh};
VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};ci.surface=surface;ci.minImageCount=std::min(cap.maxImageCount?cap.maxImageCount:3u,std::max(2u,cap.minImageCount));ci.imageFormat=swapFormat;ci.imageColorSpace=VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;ci.imageExtent=extent;ci.imageArrayLayers=1;ci.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT;ci.preTransform=cap.currentTransform;ci.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;ci.presentMode=VK_PRESENT_MODE_FIFO_KHR;ci.clipped=VK_TRUE;vkcheck(vkCreateSwapchainKHR(device,&ci,nullptr,&swapchain),"Create swapchain");uint32_t n=0;vkGetSwapchainImagesKHR(device,swapchain,&n,nullptr);images.resize(n);vkGetSwapchainImagesKHR(device,swapchain,&n,images.data());presentReady.resize(n);VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};for(auto& semaphore:presentReady)vkcheck(vkCreateSemaphore(device,&semaphoreInfo,nullptr,&semaphore),"Per-image present semaphore");for(auto im:images)views.push_back(imageView(im,swapFormat,VK_IMAGE_ASPECT_COLOR_BIT));
createSceneTargets();
for(auto v:views){VkImageView attachments[]={v};VkFramebufferCreateInfo fi{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};fi.renderPass=postPass;fi.attachmentCount=1;fi.pAttachments=attachments;fi.width=extent.width;fi.height=extent.height;fi.layers=1;VkFramebuffer fb;vkcheck(vkCreateFramebuffer(device,&fi,nullptr,&fb),"post fb");framebuffers.push_back(fb);}rebuildSwap=false;}
VkPipeline makePipeline(bool sky,bool water=false){VkShaderModule vs=shader(sky?sky_vert:scene_vert,sky?sizeof(sky_vert):sizeof(scene_vert)),fs=shader(sky?sky_frag:water?water_frag:scene_frag,sky?sizeof(sky_frag):water?sizeof(water_frag):sizeof(scene_frag));VkPipelineShaderStageCreateInfo stages[2]{};for(int i=0;i<2;i++){stages[i].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;stages[i].stage=i?VK_SHADER_STAGE_FRAGMENT_BIT:VK_SHADER_STAGE_VERTEX_BIT;stages[i].module=i?fs:vs;stages[i].pName="main";}
VkVertexInputBindingDescription bind{0,sizeof(Vertex),VK_VERTEX_INPUT_RATE_VERTEX};VkVertexInputAttributeDescription attrs[]={{0,0,VK_FORMAT_R32G32B32_SFLOAT,0},{1,0,VK_FORMAT_R32G32B32_SFLOAT,12},{2,0,VK_FORMAT_R32G32_SFLOAT,24},{3,0,VK_FORMAT_R32G32_SFLOAT,32}};
VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};if(!sky){vi.vertexBindingDescriptionCount=1;vi.pVertexBindingDescriptions=&bind;vi.vertexAttributeDescriptionCount=4;vi.pVertexAttributeDescriptions=attrs;}
VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};ia.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};vp.viewportCount=vp.scissorCount=1;
VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};rs.polygonMode=VK_POLYGON_MODE_FILL;rs.cullMode=VK_CULL_MODE_NONE;rs.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE;rs.lineWidth=1;
VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};ms.rasterizationSamples=samples;
VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};ds.depthTestEnable=sky?VK_FALSE:VK_TRUE;ds.depthWriteEnable=sky?VK_FALSE:VK_TRUE;ds.depthCompareOp=VK_COMPARE_OP_LESS_OR_EQUAL;
VkPipelineColorBlendAttachmentState att{};att.colorWriteMask=15;att.blendEnable=VK_TRUE;att.srcColorBlendFactor=VK_BLEND_FACTOR_SRC_ALPHA;att.dstColorBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;att.colorBlendOp=VK_BLEND_OP_ADD;att.srcAlphaBlendFactor=VK_BLEND_FACTOR_ONE;att.dstAlphaBlendFactor=VK_BLEND_FACTOR_ZERO;att.alphaBlendOp=VK_BLEND_OP_ADD;
VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};cb.attachmentCount=1;cb.pAttachments=&att;
VkDynamicState dyn[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};VkPipelineDynamicStateCreateInfo dy{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};dy.dynamicStateCount=2;dy.pDynamicStates=dyn;
VkGraphicsPipelineCreateInfo pi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};pi.stageCount=2;pi.pStages=stages;pi.pVertexInputState=&vi;pi.pInputAssemblyState=&ia;pi.pViewportState=&vp;pi.pRasterizationState=&rs;pi.pMultisampleState=&ms;pi.pDepthStencilState=&ds;pi.pColorBlendState=&cb;pi.pDynamicState=&dy;pi.layout=pipelineLayout;pi.renderPass=scenePass;VkPipeline p;vkcheck(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&pi,nullptr,&p),"Graphics pipeline");vkDestroyShaderModule(device,vs,nullptr);vkDestroyShaderModule(device,fs,nullptr);return p;}
void initVulkan(){const char* exts[]={VK_KHR_SURFACE_EXTENSION_NAME,VK_KHR_WIN32_SURFACE_EXTENSION_NAME,VK_EXT_DEBUG_UTILS_EXTENSION_NAME};const char* layer="VK_LAYER_KHRONOS_validation";VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};app.pApplicationName="Creating Constructs";app.apiVersion=VK_API_VERSION_1_1;
VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};ci.pApplicationInfo=&app;ci.enabledExtensionCount=validate?3:2;ci.ppEnabledExtensionNames=exts;if(validate){ci.enabledLayerCount=1;ci.ppEnabledLayerNames=&layer;}vkcheck(vkCreateInstance(&ci,nullptr,&instance),"Create Vulkan instance. Install a Vulkan-capable graphics driver.");
if(validate){VkDebugUtilsMessengerCreateInfoEXT di{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};di.messageSeverity=VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT|VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;di.messageType=7;di.pfnUserCallback=debugCallback;auto fn=(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkCreateDebugUtilsMessengerEXT");if(fn)vkcheck(fn(instance,&di,nullptr,&debugMessenger),"Validation messenger");}
VkWin32SurfaceCreateInfoKHR si{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};si.hinstance=GetModuleHandle(nullptr);si.hwnd=viewWindow;vkcheck(vkCreateWin32SurfaceKHR(instance,&si,nullptr,&surface),"Win32 surface");uint32_t count=0;vkEnumeratePhysicalDevices(instance,&count,nullptr);if(!count)fail("No Vulkan graphics device found.");std::vector<VkPhysicalDevice> devs(count);vkEnumeratePhysicalDevices(instance,&count,devs.data());int score=-1;
for(auto pd:devs){VkPhysicalDeviceProperties prop;vkGetPhysicalDeviceProperties(pd,&prop);uint32_t nc=0;vkGetPhysicalDeviceQueueFamilyProperties(pd,&nc,nullptr);std::vector<VkQueueFamilyProperties> qs(nc);vkGetPhysicalDeviceQueueFamilyProperties(pd,&nc,qs.data());for(uint32_t j=0;j<nc;j++){VkBool32 support;vkGetPhysicalDeviceSurfaceSupportKHR(pd,j,surface,&support);int sc=prop.deviceType==VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU?2:1;if(support&&(qs[j].queueFlags&VK_QUEUE_GRAPHICS_BIT)&&sc>score){score=sc;physical=pd;family=j;gpuName=prop.deviceName;}}}if(score<0)fail("No Vulkan graphics queue supports this display.");log(gpuName.c_str());VkPhysicalDeviceProperties props;vkGetPhysicalDeviceProperties(physical,&props);if(!(props.limits.framebufferColorSampleCounts&props.limits.framebufferDepthSampleCounts&samples))samples=VK_SAMPLE_COUNT_1_BIT;
float priority=1;VkDeviceQueueCreateInfo qi{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};qi.queueFamilyIndex=family;qi.queueCount=1;qi.pQueuePriorities=&priority;const char* swapExt=VK_KHR_SWAPCHAIN_EXTENSION_NAME;VkDeviceCreateInfo di{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};di.queueCreateInfoCount=1;di.pQueueCreateInfos=&qi;di.enabledExtensionCount=1;di.ppEnabledExtensionNames=&swapExt;vkcheck(vkCreateDevice(physical,&di,nullptr,&device),"Create logical device");vkGetDeviceQueue(device,family,0,&queue);
uint32_t nf=0;vkGetPhysicalDeviceSurfaceFormatsKHR(physical,surface,&nf,nullptr);std::vector<VkSurfaceFormatKHR> formats(nf);vkGetPhysicalDeviceSurfaceFormatsKHR(physical,surface,&nf,formats.data());swapFormat=formats[0].format;for(auto f:formats)if(f.format==VK_FORMAT_B8G8R8A8_UNORM)swapFormat=f.format;
initPostPass();renderPass=scenePass;
VkDescriptorSetLayoutBinding bindings[3]={{0,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1,VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,nullptr},{1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_FRAGMENT_BIT,nullptr},{2,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1,VK_SHADER_STAGE_FRAGMENT_BIT,nullptr}};
VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};li.bindingCount=3;li.pBindings=bindings;vkcheck(vkCreateDescriptorSetLayout(device,&li,nullptr,&descriptorLayout),"Descriptor layout");VkPipelineLayoutCreateInfo pli{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};pli.setLayoutCount=1;pli.pSetLayouts=&descriptorLayout;vkcheck(vkCreatePipelineLayout(device,&pli,nullptr,&pipelineLayout),"Pipeline layout");
pipeline=makePipeline(false);skyPipeline=makePipeline(true);waterPipeline=makePipeline(false,true);buffer(maxVertexBytes,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,vertexBuffer,vertexMemory,&vertexMap);buffer(sizeof(State),VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,stateBuffer,stateMemory,&stateMap);buffer(foliageCapacity,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,foliageBuffer,foliageMemory,&foliageMap);memset(foliageMap,0,foliageCapacity);
VkDescriptorPoolSize ps[2]={{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,2},{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1}};VkDescriptorPoolCreateInfo dpi{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};dpi.maxSets=1;dpi.poolSizeCount=2;dpi.pPoolSizes=ps;vkcheck(vkCreateDescriptorPool(device,&dpi,nullptr,&descriptorPool),"Descriptor pool");
VkCommandPoolCreateInfo cp{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};cp.queueFamilyIndex=family;cp.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;vkcheck(vkCreateCommandPool(device,&cp,nullptr,&commandPool),"Command pool");
uploadTextures();
VkDescriptorSetAllocateInfo dai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};dai.descriptorPool=descriptorPool;dai.descriptorSetCount=1;dai.pSetLayouts=&descriptorLayout;vkcheck(vkAllocateDescriptorSets(device,&dai,&descriptorSet),"Descriptor set");
VkDescriptorBufferInfo bi{stateBuffer,0,sizeof(State)};VkDescriptorImageInfo ii{texSampler,texView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
VkWriteDescriptorSet wr[2]{};wr[0].sType=wr[1].sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;wr[0].dstSet=wr[1].dstSet=descriptorSet;wr[0].dstBinding=0;wr[0].descriptorCount=1;wr[0].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;wr[0].pBufferInfo=&bi;wr[1].dstBinding=1;wr[1].descriptorCount=1;wr[1].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;wr[1].pImageInfo=&ii;vkUpdateDescriptorSets(device,2,wr,0,nullptr);VkDescriptorBufferInfo fi{foliageBuffer,0,foliageCapacity};VkWriteDescriptorSet fw{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};fw.dstSet=descriptorSet;fw.dstBinding=2;fw.descriptorCount=1;fw.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;fw.pBufferInfo=&fi;vkUpdateDescriptorSets(device,1,&fw,0,nullptr);
VkCommandBufferAllocateInfo ca{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ca.commandPool=commandPool;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;vkcheck(vkAllocateCommandBuffers(device,&ca,&command),"Command buffer");VkSemaphoreCreateInfo se{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};vkcheck(vkCreateSemaphore(device,&se,nullptr,&acquired),"Acquire semaphore");vkcheck(vkCreateSemaphore(device,&se,nullptr,&rendered),"Present semaphore");VkFenceCreateInfo fe{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};fe.flags=VK_FENCE_CREATE_SIGNALED_BIT;vkcheck(vkCreateFence(device,&fe,nullptr,&fence),"Render fence");createSwap();log("Vulkan renderer initialized");}

#include "performance.inc"
#include "readback.inc"
#include "glass_readback.inc"
void refresh(){std::fill(std::begin(gpu.natureMask),std::end(gpu.natureMask),0u);bakeNatureMask=true;ivyBeginRefresh();vertices.clear();transparent.clear();grass.clear();pieces=0;origin={0,0,0};turn=0;ambient=1;
// HTML: 400x400 ground at y=-.01 color #777977, plot N×N at y=0 color #cfcdc5
box(400,.02,400,world.n*.5f,-.02f,world.n*.5f,11);
box((float)world.n,.02,(float)world.n,world.n*.5f,-.005f,world.n*.5f,12);
for(auto& l:gpu.lamps)l={0,0,0,0};int lamp=0;
for(int x=0;x<MaxPlot;x++)for(int z=0;z<MaxPlot;z++)for(int y=0;y<20;y++){auto c=get(x,y,z);gpu.cells[idx(x,y,z)]=c.t<0?0:1+c.t+PieceEncodingStride*c.r;if(c.t<0)continue;pieces++;int neighbors=0;for(V o:{V{1,0,0},V{-1,0,0},V{0,1,0},V{0,-1,0},V{0,0,1},V{0,0,-1}})if(get(x+(int)o.x,y+(int)o.y,z+(int)o.z).t>=0)neighbors++;
bool roof=get(x,y+1,z).t>=0;
ambient=std::max(.45f,(1.f-neighbors*.085f-(y==0?.06f:0.f)-(roof?.05f:0.f))*(.94f+((x*73+y*31+z*17)%11)/11.f*.12f));
origin={x+.5f,y+.5f,z+.5f};turn=c.r;
if(c.t==5){if(!roof)natureUnderstory(.51f,false);continue;} // moss mesh from surface nets
piece(c.t);if((c.t==13||c.t==26||c.t==27||c.t==37)&&lamp<8){V emitter=lampEmitter(c.t);gpu.lamps[lamp++]={emitter.x,emitter.y,emitter.z,1};}
if(c.t==17&&!roof)natureUnderstory(-.07f,true);
}
bakeNatureMask=false;origin={};turn=0;ambient=1;emitMossSurfaceNets();
renderPipes();vertices.insert(vertices.end(),transparent.begin(),transparent.end());transparent.clear();
std::vector<Vertex> waterVertices;size_t kept=0;for(size_t i=0;i+2<vertices.size();i+=3){if(vertices[i].m==9)waterVertices.insert(waterVertices.end(),vertices.begin()+i,vertices.begin()+i+3);else for(int j=0;j<3;j++)vertices[kept++]=vertices[i+j];}vertices.resize(kept);waterVertexCount=(uint32_t)waterVertices.size();vertices.insert(vertices.end(),waterVertices.begin(),waterVertices.end());
buildFoliageTrace();staticCount=(uint32_t)vertices.size();if(vertices.size()*sizeof(Vertex)>maxVertexBytes-1024*1024)fail("Geometry capacity exceeded");memcpy(vertexMap,vertices.data(),vertices.size()*sizeof(Vertex));dirty=false;if(topPanel)InvalidateRect(topPanel,nullptr,FALSE);if(dockPanel)InvalidateRect(dockPanel,nullptr,FALSE);}
bool solidAt(V p){int x=(int)floorf(p.x),y=(int)floorf(p.y),z=(int)floorf(p.z);int bx,by,bz;if(!footprintRoot(x,y,z,bx,by,bz))return false;auto c=get(bx,by,bz);V f={p.x-bx,p.y-by,p.z-bz};V d={f.x-.5f,f.y,f.z-.5f};for(int i=0;i<c.r;i++)d={-d.z,d.y,d.x};f={d.x+.5f,d.y,d.z+.5f};
switch(c.t){case 1:return f.y<.5;case 2:case 7:case 14:case 41:case 42:return fabsf(f.x-.5f)<.2f&&fabsf(f.z-.5f)<.2f;case 43:case 44:case 45:case 46:return false;case 4:return f.y<1-f.z;case 6:return f.y<ceilf((1-f.z)*4)/4;case 8:return fabsf(f.z-.5f)<.15f&&!(f.x>.15&&f.x<.85&&f.y>.25&&f.y<.8);case 9:return fabsf(f.z-.5f)<.15f;case 10:return !(fabsf(f.x-.5f)<.25f&&(f.y<.5f||(f.x-.5f)*(f.x-.5f)+(f.y-.5f)*(f.y-.5f)<.0625f));case 11:return f.z<.15;case 12:return f.y>.7&&fabsf(f.z-.5f)<.15;case 13:case 15:case 16:case 25:case 27:return false;case 17:return f.y<.5;case 21:return fabsf(f.z-.5f)<.25;case 22:return fabsf(f.z-.5f)<.15;case 23:return f.z>.02f&&f.z<.72f&&!(f.x>.3f&&f.x<1.7f&&f.y<1.82f); // deep interior-biased collider
case 24:return f.y<.1||(f.y<.5&&std::max(fabsf(f.x-.5f),fabsf(f.z-.5f))>.4);case 26:return fabsf(f.x-.5f)<.12&&fabsf(f.z-.5f)<.12&&f.y<.8;
case 28:return (f.x-.5f)*(f.x-.5f)+(f.z-.5f)*(f.z-.5f)+f.y*f.y<.25f;
case 29:return (f.x-.5f)*(f.x-.5f)+(f.z-.5f)*(f.z-.5f)<.25f;
case 30:return f.y<.5f+sqrtf(std::max(0.f,.25f-(f.x-.5f)*(f.x-.5f)));
case 31:return fabsf(f.z-.5f)<.05f;case 32:case 33:return fabsf(f.z-.5f)<.15f;
case 34:return f.y>.85f;case 35:case 37:case 40:return false;
case 36:return f.y<ceilf((1-f.z)*8)/8;
case 38:return fabsf(f.x-.5f)<.2f&&fabsf(f.z-.5f)<.2f;
case 39:return f.y<.35f&&fabsf(f.z-.5f)<.2f&&fabsf(f.x-.5f)<.45f;
case 48:return f.y>.72f;case 49:return f.z<.55f-.28f*f.y;case 50:return f.y>.86f;
case 51:return f.z>-.05f&&f.z<.7f&&!(f.x>.32f&&f.x<1.68f&&f.y<1.8f);
case 52:return fabsf(f.z-.5f)<.175f&&!(f.x>.1f&&f.x<.9f&&f.y>.55f&&f.y<.85f);
case 53:return f.y<.22f;case 54:return f.z<.18f||fabsf(f.x-.5f)>.32f;case 55:return f.y>.65f&&fabsf(f.z-.5f)<.35f;
case 56:return f.y>1.66f&&fabsf(f.x-.5f)<.28f&&fabsf(f.z-.5f)<.28f;
case 57:return f.y<.5f;
case 58:return f.x<.18f||f.x>1.82f||f.z<.18f||f.z>1.82f;
case 59:return f.y<.12f||fabsf(f.z-.5f)>.38f||f.y>1.85f; // standing clearance inside tunnel
case 60:return fabsf(f.x-.5f)>.36f||f.z>.86f||f.y>1.85f; // walk-in shaft for 1.6m player
case 62:return f.y>.05f&&f.z<.55f-.35f*f.y&&fabsf(f.x-.5f)<.3f;
default:return true;}}
bool freeAt(V p,float h){for(V o:{V{0,0,0},V{.28,0,0},V{-.28,0,0},V{0,0,.28},V{0,0,-.28}})for(float y:{.05f,h*.5f,h-.05f})if(solidAt(p+o+V{0,y,0}))return false;return true;}
// HTML wallDir — adjacent blocked cell while airborne
bool wallDir(float h,float& ox,float& oz){for(V o:{V{.45f,0,0},V{-.45f,0,0},V{0,0,.45f},V{0,0,-.45f}})if(!freeAt(player+o+V{0,.6f,0},h)){ox=o.x;oz=o.z;return true;}return false;}
void playerStep(float dt){V audioBefore=player;bool audioGrounded=grounded;float audioFall=velY;float h=keys['C']||keys[VK_CONTROL]?1.0f:1.6f;if(keys['Q'])yaw+=dt*1.8f;if(keys['E'])yaw-=dt*1.8f;
float f=(keys['W']||keys[VK_UP]?1.f:0)-(keys['S']||keys[VK_DOWN]?1.f:0),s=(keys['D']||keys[VK_RIGHT]?1.f:0)-(keys['A']||keys[VK_LEFT]?1.f:0);
float L=sqrtf(f*f+s*s);if(L<1e-4f)L=1;float sp=(keys['C']||keys[VK_CONTROL]?2.f:keys[VK_SHIFT]?7.5f:4.2f)*dt/L;
float sn=sinf(yaw),cs=cosf(yaw);float dx=(-sn*f+cs*s)*sp+hx*dt,dz=(-cs*f-sn*s)*sp+hz*dt;hx*=1-4*dt;hz*=1-4*dt;
for(V d:{V{dx,0,0},V{0,0,dz}}){V q=player+d;if(freeAt(q,h))player=q;else if(grounded&&freeAt(q+V{0,.6f,0},h)){player=q;player.y+=.6f;}else if(!grounded&&f>0&&velY<4&&freeAt(q+V{0,1.1f,0},h))velY=4.6f;}
player.x=std::clamp(player.x,-8.f,world.n+8.f);player.z=std::clamp(player.z,-8.f,world.n+8.f);
if(keys[VK_SPACE]&&grounded){velY=7.6f;grounded=false;audioEvent(A_jump,.65f);}
float wox=0,woz=0;bool wl=!grounded&&wallDir(h,wox,woz);bool wrun=false;
if(wl&&keys[VK_SPACE]&&!spaceHeld){audioEvent(A_jump,.7f,.94f);velY=7.4f;hx=-wox*13;hz=-woz*13;wallRun=0;}
else if(wl&&keys['W']&&wallRun<1.8f&&velY<3){velY=std::max(velY,-1.1f);wallRun+=dt;wrun=true;}
if(grounded)wallRun=0;spaceHeld=!!keys[VK_SPACE];
float side=wrun?(wox*cs-woz*sn)*-.012f:0;camRoll=camRoll*.92f+side;
auto cell=get((int)floorf(player.x),(int)floorf(player.y+.9f),(int)floorf(player.z));
velY=cell.t==25&&(keys['W']||keys[VK_SPACE])?3.2f:std::max(-25.f,velY-22*dt);
float ny=player.y+velY*dt;
if(ny<=0){player.y=0;velY=0;grounded=true;}
else if(freeAt({player.x,ny,player.z},h)){player.y=ny;grounded=false;}
else{if(velY<0){int n=0;while(n++<30&&player.y>0&&freeAt(player+V{0,-.02f,0},h))player.y-=.02f;grounded=true;}velY=0;}
if(grounded&&player.y>0&&freeAt(player+V{0,-.05f,0},h)&&velY==0)grounded=false;
audioFootstep(audioBefore,audioGrounded,audioFall);
eye=player+V{0,h-.12f,0};forward={-sinf(yaw)*cosf(pitch),sinf(pitch),-cosf(yaw)*cosf(pitch)};}
void camera(float dt){if(walk){for(int i=0;i<3;i++)playerStep(dt/3);}else{eye=target+V{sinf(phi)*sinf(theta),cosf(phi),sinf(phi)*cosf(theta)}*distanceCam;forward=norm(target-eye);camRoll=0;}
rightv=norm(cross(forward,{0,1,0}));upv=cross(rightv,forward);
if(walk&&fabsf(camRoll)>.0001f){float cr=cosf(camRoll),sr=sinf(camRoll);V nr=rightv*cr+upv*sr,nu=upv*cr-rightv*sr;rightv=nr;upv=nu;}
float aspect=extent.width/(float)extent.height,f=1/tanf((walk?fieldOfView:60.f)*PI/360),A=300.f/(300.f-.1f),B=-.1f*A;
V rows[]={rightv*(f/aspect),upv*(-f),forward*A,forward};float offsets[]={-dot(rows[0],eye),-dot(rows[1],eye),B-dot(rows[2],eye),-dot(forward,eye)};for(int r=0;r<4;r++){gpu.vp[r]=rows[r].x;gpu.vp[4+r]=rows[r].y;gpu.vp[8+r]=rows[r].z;gpu.vp[12+r]=offsets[r];}}
void pick(){hit=hasAdd=hasRem=false;if(walk)return;float f=tanf(PI/6),a=extent.width/(float)extent.height;V d=norm(forward+rightv*((mouseX/(float)vw*2-1)*f*a)+upv*((1-mouseY/(float)vh*2)*f));V prev{-999,-999,-999};for(float t=.1;t<120;t+=.035f){V p=eye+d*t,q{floorf(p.x),floorf(p.y),floorf(p.z)};if(q.x==prev.x&&q.y==prev.y&&q.z==prev.z)continue;if(q.y<0){if(inb((int)prev.x,(int)prev.y,(int)prev.z)){hit=hasAdd=true;hitAdd=prev;hitPoint=p;}return;}int ix=(int)q.x,iy=(int)q.y,iz=(int)q.z,ox,oy,oz;if(footprintRoot(ix,iy,iz,ox,oy,oz)&&!(drag&&placed[idx(ox,oy,oz)])){hit=hasRem=true;hitRem={(float)ox,(float)oy,(float)oz};hitPoint=p;hasAdd=inb((int)prev.x,(int)prev.y,(int)prev.z);hitAdd=prev;return;}prev=q;}}
V anchor(){if(hasRem&&hasAdd)return (hitAdd+hitRem)*.5f+V{.5,.5,.5};if(hasRem)return hitRem+V{.5,1,.5};return hitAdd+V{.5,.05,.5};}
void autoFacing(int& t,int& r){if(hasRem&&hasAdd){V d=hitRem-hitAdd;if(d.x||d.z){if(t==13)t=27;if(t==15||t==25||t==27)r=d.x<0?1:d.x>0?3:d.z>0?2:0;}}}
void edit(){if(!hit)return;bool remove=keys[VK_SHIFT]||keys['X']||erasing;if(selected==16){V a=anchor();if(remove){int best=-1;float bd=1.3;for(int i=0;i<(int)world.cables.size();i++){float d=std::min(len(world.cables[i].a-a),len(world.cables[i].b-a));if(d<bd){bd=d;best=i;}}if(best>=0){world.cables.erase(world.cables.begin()+best);audioEdit(true,16,a);dirty=true;save();}}else if(!pendingCable){anchorA=a;pendingCable=true;audioEvent(A_click);}else{if(len(a-anchorA)>.4&&world.cables.size()<2048){world.cables.push_back({anchorA,a});audioEdit(false,16,a);dirty=true;save();}pendingCable=false;}return;}
if(isPipeTool(selected)){pipeEdit();return;}
if(remove&&hasRem){audioEdit(true,get((int)hitRem.x,(int)hitRem.y,(int)hitRem.z).t,hitRem+V{.5f,.5f,.5f});put((int)hitRem.x,(int)hitRem.y,(int)hitRem.z,-1);dirty=true;save();}else if(!remove&&hasAdd){int t=selected,r=rotation;autoFacing(t,r);int ax=(int)hitAdd.x,ay=(int)hitAdd.y,az=(int)hitAdd.z;if(canPlace(ax,ay,az,t,r)){audioEdit(false,t,hitAdd+V{.5f,.5f,.5f});put(ax,ay,az,t,r);placed[idx(ax,ay,az)]=true;dirty=true;save();}}}
void toggleWalk(){walk=!walk;pendingCable=false;drag=false;ReleaseCapture();if(walk){player=hasNamedWalkSpawn&&freeAt(namedWalkSpawn,1.6f)?namedWalkSpawn:V{world.n/2.f,0,world.n+1.8f};velY=hx=hz=wallRun=camRoll=0;yaw=pitch=0;grounded=true;spaceHeld=false;}InvalidateRect(mainWindow,nullptr,FALSE);}
void raisePanels();
void dynamicGeometry(float time){vertices.clear();transparent.clear();origin={};turn=0;ambient=1;
float wd=rain?2.f:1.f;
for(auto c:world.cables){float L=len(c.b-c.a),slack=(.07f+.05f*sinf(time*1.4f))*L+.15f;float dx=c.b.x-c.a.x,dz=c.b.z-c.a.z,hl=hypotf(dx,dz);if(hl<1e-4f)hl=1;float nx=-dz/hl,nz=dx/hl;
auto p=[&](float s){float q=4*s*(1-s),w=sinf(time*2.1f*wd+s*7)*.05f*std::min(L,14.f)/5*wd*q;return V{c.a.x+dx*s+nx*w,c.a.y+(c.b.y-c.a.y)*s-slack*q+sinf(time*3.3f+s*5)*.025f*q,c.a.z+dz*s+nz*w};};
for(int i=0;i<16;i++)tube(p(i/16.f),p((i+1)/16.f),.028,13);}
// HTML PlaneGeometry(.5,.45) crossed + wind sway
for(auto& g:grass){float wx=sinf(time*2.2f+g.p.x*3.1f+g.p.z*2.3f)*.07f*g.scale,wz=cosf(time*1.7f+g.p.z*2.9f)*.05f*g.scale;
 float ww=.5f*g.scale,hh=.45f*g.scale,ca=cosf(g.rot),sa=sinf(g.rot);V base=g.p;
 auto cross=[&](float ax,float az){V r{ax*ww*.5f,0,az*ww*.5f};V bl=base-r,br=base+r;V tl=bl+V{wx,hh,wz},tr=br+V{wx,hh,wz};
  // V=1 roots / V=0 tips — baked PNG tips at top
  triRaw(bl,br,tr,{0,1,0},{1,1,0},{1,0,0},17,1.f);triRaw(bl,tr,tl,{0,1,0},{1,0,0},{0,0,0},17,1.f);};
 cross(ca,sa);cross(-sa,ca);}
// Emissive fixture geometry supplies bloom; no camera-facing glow discs.
if(rain){for(int i=0;i<1200;i++){float x=fmodf(i*13.713f,45)-15,z=fmodf(i*7.571f,45)-15,y=fmodf(i*.713f-time*22,23);if(y<0)y+=23;V a{x,y,z},b=a+V{-.04,.35,0};V side=rightv*.007f;quad(a-side,a+side,b+side,b-side,14);}}
bool remove=keys[VK_SHIFT]||keys['X']||erasing;if(!walk&&hit&&captureFile.empty()&&mouseInView){if(remove&&hasRem){origin=hitRem+V{.5,.5,.5};box(1.025,1.025,1.025,0,0,0,10);origin={};}else if(!remove&&hasAdd&&selected!=16&&!isPipeTool(selected)){int t=selected,r=rotation;autoFacing(t,r);origin=hitAdd+V{.5,.5,.5};turn=r;ghostMode=true;piece(t);ghostMode=false;turn=0;origin={};}if(pendingCable){V b=anchor();tube(anchorA,b,.024,10);}if(isPipeTool(selected)&&hasRem&&hasAdd){PipeNode g;if(pipeHitCell(g)){V c=pipeCenter(g);tube(c,c+PipeN[g.face]*(-.02f),.02f,10);}}}
vertices.insert(vertices.end(),transparent.begin(),transparent.end());size_t bytes=vertices.size()*sizeof(Vertex);if((staticCount+vertices.size())*sizeof(Vertex)>maxVertexBytes)fail("Dynamic geometry capacity exceeded");memcpy((char*)vertexMap+staticCount*sizeof(Vertex),vertices.data(),bytes);vertexCount=staticCount+(uint32_t)vertices.size();}
float cameraWaterDepth(V camera){
 int x=(int)floorf(camera.x),y=(int)floorf(camera.y),z=(int)floorf(camera.z);auto c=get(x,y,z);
 if(c.t==40){int top=y;while(top+1<WorldHeight&&get(x,top+1,z).t==40)top++;return std::max(0.f,top+.85f-camera.y);}
 if(c.t==24&&fabsf(camera.x-x-.5f)<.4f&&fabsf(camera.z-z-.5f)<.4f)return std::max(0.f,y+.37f-camera.y);return 0;
}
void render(float dt,float time){if(benchmarkRun)perfCpuStart=perfClock();if(vw<1||vh<1||IsIconic(mainWindow))return;if(rebuildSwap){vkcheck(vkDeviceWaitIdle(device),"Wait for resize");destroySwap();createSwap();raisePanels();}vkcheck(vkWaitForFences(device,1,&fence,VK_TRUE,UINT64_MAX),"Wait for frame");if(benchmarkRun)perfBuildStart=perfClock();if(dirty)refresh();camera(dt);pick();dynamicGeometry(time);
// Literally monolith.html sun() + MOODS (lines 236-246)
float el=sunHeight*PI/180,az=sunAngle*PI/180,low=night?0.f:std::max(0.f,1.f-el/.55f);
if(mood<0||mood>4)mood=0;float si=moodSunI[mood],hi=moodHi[mood];
float sunI=night?0.f:si*std::min(1.f,el*2.4f)*(rain?.55f:1.f);
gpu.eye={eye.x,eye.y,eye.z,time};
// sun direction from HTML: position relative to target, normalized
V sunDir={cosf(az)*cosf(el),sinf(el),sinf(az)*cosf(el)};
if(night){sunDir={-.35f,.85f,-.25f};sunI=.9f;} // moon.intensity night .9
gpu.sun={sunDir.x,sunDir.y,sunDir.z,sunI};
// hor = night?0x0a1020 : M.hor lerp 0xe9a878 by low*.35
V hor=night?V{.039f,.063f,.125f}:moodHor[mood];
hor=hor*(1.f-low*.35f)+V{.914f,.659f,.471f}*(low*.35f);
V top=night?V{.012f,.020f,.047f}:moodTop[mood]; // night 0x03050c
gpu.sky={hor.x,hor.y,hor.z,night?1.f:0.f};
gpu.atmos={top.x,top.y,top.z,night?.1f:(rain?moodSat*.4f:moodSat)}; // PQ.uniforms.sat
gpu.cfg={exposure, (rain?.012f:0.f)+fog*fog*.09f+.003f, ao?1.f:0.f, (float)world.n}; // FogExp2 density
for(int i=0;i<8;i++)gpu.lamps[i].w=(gpu.lamps[i].x||gpu.lamps[i].y||gpu.lamps[i].z)?(night?4.5f:0.f):0.f; // HTML day lamps off
gpu.lighting={night?.28f:hi*1.25f,rain?1.f:0.f,cameraWaterDepth(eye),0}; // hemi boost for brighter foliage/concrete
memcpy(stateMap,&gpu,sizeof(gpu));
PostU pu{time,night?.1f:(rain?moodSat*.4f:moodSat),ao?1.f:0.f,exposure};memcpy(postMap,&pu,sizeof(pu));
if(benchmarkRun)perfBuildMs=perfClock()-perfBuildStart;uint32_t index;VkResult r=vkAcquireNextImageKHR(device,swapchain,UINT64_MAX,acquired,VK_NULL_HANDLE,&index);if(r==VK_ERROR_OUT_OF_DATE_KHR){rebuildSwap=true;return;}if(r!=VK_SUBOPTIMAL_KHR)vkcheck(r,"Acquire image");vkcheck(vkResetFences(device,1,&fence),"Reset fence");vkcheck(vkResetCommandBuffer(command,0),"Reset command buffer");VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};vkcheck(vkBeginCommandBuffer(command,&begin),"Begin frame");performanceMark(0);
VkClearValue clear[2]{};clear[0].color={{gpu.sky.x,gpu.sky.y,gpu.sky.z,1}};clear[1].depthStencil={1,0};
VkRenderPassBeginInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};rp.renderPass=scenePass;rp.framebuffer=sceneFB;rp.renderArea.extent=extent;rp.clearValueCount=2;rp.pClearValues=clear;
vkCmdBeginRenderPass(command,&rp,VK_SUBPASS_CONTENTS_INLINE);VkViewport vp{0,0,(float)extent.width,(float)extent.height,0,1};VkRect2D sc{{0,0},extent};vkCmdSetViewport(command,0,1,&vp);vkCmdSetScissor(command,0,1,&sc);
vkCmdBindDescriptorSets(command,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,1,&descriptorSet,0,nullptr);vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,skyPipeline);vkCmdDraw(command,3,1,0,0);vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline);VkDeviceSize offset=0;vkCmdBindVertexBuffers(command,0,1,&vertexBuffer,&offset);vkCmdDraw(command,staticCount-waterVertexCount,1,0,0);if(waterVertexCount){vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,waterPipeline);vkCmdDraw(command,waterVertexCount,1,staticCount-waterVertexCount,0);vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline);}if(vertexCount>staticCount)vkCmdDraw(command,vertexCount-staticCount,1,staticCount,0);vkCmdEndRenderPass(command);
performanceMark(1);VkRenderPassBeginInfo pp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};pp.renderPass=postPass;pp.framebuffer=framebuffers[index];pp.renderArea.extent=extent;
vkCmdBeginRenderPass(command,&pp,VK_SUBPASS_CONTENTS_INLINE);vkCmdSetViewport(command,0,1,&vp);vkCmdSetScissor(command,0,1,&sc);vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,postPipeline);vkCmdBindDescriptorSets(command,VK_PIPELINE_BIND_POINT_GRAPHICS,postPipeLayout,0,1,&postDescSet,0,nullptr);vkCmdDraw(command,3,1,0,0);vkCmdEndRenderPass(command);
performanceMark(2);recordUiBackground(index);recordReadback(index);performanceMark(3);vkcheck(vkEndCommandBuffer(command),"End frame");VkPipelineStageFlags wait=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};submit.waitSemaphoreCount=1;submit.pWaitSemaphores=&acquired;submit.pWaitDstStageMask=&wait;submit.commandBufferCount=1;submit.pCommandBuffers=&command;submit.signalSemaphoreCount=1;submit.pSignalSemaphores=&presentReady[index];vkcheck(vkQueueSubmit(queue,1,&submit,fence),"Submit frame");VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};present.waitSemaphoreCount=1;present.pWaitSemaphores=&presentReady[index];present.swapchainCount=1;present.pSwapchains=&swapchain;present.pImageIndices=&index;r=vkQueuePresentKHR(queue,&present);if(r==VK_ERROR_OUT_OF_DATE_KHR||r==VK_SUBOPTIMAL_KHR)rebuildSwap=true;else vkcheck(r,"Present frame");vkcheck(vkWaitForFences(device,1,&fence,VK_TRUE,UINT64_MAX),"Wait for render resources");finishReadback();double blurStart=benchmarkRun?perfClock():0;finishUiBackground();if(benchmarkRun)perfBlurMs=perfClock()-blurStart;performanceSample();frames++;}

#include "platform.inc"
