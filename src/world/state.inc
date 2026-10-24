// Authored target states persist; transient poses never become level geometry.
struct DoorMotion{float amount=0;int rotation=0;};
std::unordered_map<int,DoorMotion> doorMotions;std::vector<int> doorRoots;
bool doorFramePass=false,doorHingeActive=false;float doorHingeAngle=0;
std::string doorHint;bool doorReviewImages=false;
float doorEase(float f){return f*f*(3.f-2.f*f);}
float doorAmount(int index){auto motion=doorMotions.find(index);if(motion!=doorMotions.end())return motion->second.amount;return world.doors.count(index)?1.f:0.f;}
void resetDoorMotion(){doorMotions.clear();doorRoots.clear();}
void forgetDoor(int index){world.doors.erase(index);doorMotions.erase(index);}
V doorHingePoint(V p){if(!doorHingeActive)return p;float x=p.x+.42f,z=p.z+.36f,c=cosf(doorHingeAngle),s=sinf(doorHingeAngle);return {x*c-z*s-.42f,p.y,x*s+z*c-.36f};}
V doorHingeNormal(V p){if(!doorHingeActive)return p;float c=cosf(doorHingeAngle),s=sinf(doorHingeAngle);return {p.x*c-p.z*s,p.y,p.x*s+p.z*c};}
bool doorLeafContains(V f,float amount){
 if(f.y<.04f||f.y>1.88f)return false;
 float a=doorEase(amount)*PI*.5f,c=cosf(a),s=sinf(a),x=f.x-.08f,z=f.z-.14f;
 float u=x*c+z*s,v=-x*s+z*c;
 return u>.01f&&u<.83f&&fabsf(v)<.036f;
}
V doorLocalPoint(V p,int index,int rotation){
 int x=index/(MaxPlot*WorldHeight),z=(index/WorldHeight)%MaxPlot,y=index%WorldHeight;
 V q=p-V{x+.5f,(float)y,z+.5f};for(int i=0;i<rotation;i++)q={-q.z,q.y,q.x};return {q.x+.5f,q.y,q.z+.5f};
}
int doorGpuBits(int index){return (int)lroundf(std::clamp(doorAmount(index),0.f,1.f)*127.f)<<24;}

void pruneDoorStates(){for(auto it=world.doors.begin();it!=world.doors.end();){if(world.c[it->first].t!=96)it=world.doors.erase(it);else ++it;}resetDoorMotion();}

bool doorBodyTouches(V foot,float height,int index,int rotation,float amount,float radius=.28f){
 V f=doorLocalPoint(foot,index,rotation);if(f.y+height<=.04f||f.y>=1.88f)return false;
 float a=doorEase(amount)*PI*.5f,c=cosf(a),sn=sinf(a),x=f.x-.08f,z=f.z-.14f;
 float u=x*c+z*sn,v=-x*sn+z*c;
 float du=u-std::clamp(u,.01f,.83f),dv=v-std::clamp(v,-.036f,.036f);return du*du+dv*dv<radius*radius;
}

// Per-piece surface choices. Sparse optional save chunks preserve MON4/MNS4 compatibility.
const char* surfaceNames[]={"Original material","Cast concrete","Board-form concrete","Aggregate","Red clay brick","Mossy red brick","Stair brick","Arch brick 1","Arch brick 2","Arch brick 3","Brick keystone","Brick mortar","Tudor narrow 1","Tudor narrow 2","Tudor rising","Tudor diagonal 1","Tudor diagonal 2","Tudor falling","Tudor cross","Tudor box","Timber","Steel","Natural stone","Soil"};
constexpr int SurfaceCount=24;
int chosenSurface=0,surfaceRotation=0,surfaceScale=1,activeSurface=0;
int surfaceRecipe(){return chosenSurface?chosenSurface+surfaceRotation*32+surfaceScale*128:0;}
bool validSurface(int recipe){return recipe>=0&&recipe<512&&(recipe&31)<SurfaceCount&&((recipe>>7)&3)<3;}
bool surfaceEligible(int m){return m==52||m==0||m==7||m==5||m==8||m==18||m==19||m==38||m==39;}
int surfaceAt(int index){auto it=world.surfaces.find(index);return it==world.surfaces.end()?0:it->second;}
bool writeSurfaceChunk(FILE* f){
 uint32_t tag=0x3154414d,count=(uint32_t)world.surfaces.size();
 if(fwrite(&tag,4,1,f)!=1||fwrite(&count,4,1,f)!=1)return false;
 for(auto item:world.surfaces){uint32_t index=item.first;uint16_t value=item.second;if(fwrite(&index,4,1,f)!=1||fwrite(&value,2,1,f)!=1)return false;}
 uint32_t viewTag=0x31574556,views=(uint32_t)world.viewpoints.size();if(fwrite(&viewTag,4,1,f)!=1||fwrite(&views,4,1,f)!=1)return false;
 for(auto& v:world.viewpoints)if(fwrite(v.name,sizeof(v.name),1,f)!=1||fwrite(&v.view,sizeof(v.view),1,f)!=1)return false;
 uint32_t doorTag=0x31524f44,doors=(uint32_t)world.doors.size();if(fwrite(&doorTag,4,1,f)!=1||fwrite(&doors,4,1,f)!=1)return false;
 for(auto item:world.doors){uint32_t index=item.first;uint8_t target=item.second;if(fwrite(&index,4,1,f)!=1||fwrite(&target,1,1,f)!=1)return false;}
 return true;
}
bool readSurfaceChunk(FILE* f,World& w){
 w.surfaces.clear();w.viewpoints.clear();w.doors.clear();uint32_t tag=0,count=0;size_t bytes=fread(&tag,1,4,f);if(!bytes&&feof(f))return true;
 if(bytes!=4||tag!=0x3154414d||fread(&count,4,1,f)!=1||count>WorldCells)return false;
 for(uint32_t i=0;i<count;i++){uint32_t index;uint16_t value;if(fread(&index,4,1,f)!=1||fread(&value,2,1,f)!=1||index>=WorldCells||w.c[index].t<0||!validSurface(value)||w.surfaces.count(index))return false;w.surfaces[index]=value;}
 bytes=fread(&tag,1,4,f);if(!bytes&&feof(f))return true;
 if(bytes!=4||tag!=0x31574556||fread(&count,4,1,f)!=1||count>16)return false;
 for(uint32_t i=0;i<count;i++){Viewpoint v;if(fread(v.name,sizeof(v.name),1,f)!=1||v.name[39]!=0||fread(&v.view,sizeof(v.view),1,f)!=1||!validSavedView(v.view))return false;w.viewpoints.push_back(v);}
 bytes=fread(&tag,1,4,f);if(!bytes&&feof(f))return true;
 if(bytes!=4||tag!=0x31524f44||fread(&count,4,1,f)!=1||count>WorldCells)return false;
 for(uint32_t i=0;i<count;i++){uint32_t index;uint8_t target;if(fread(&index,4,1,f)!=1||fread(&target,1,1,f)!=1||index>=WorldCells||w.c[index].t!=96||target!=1||w.doors.count(index))return false;w.doors[index]=target;}return true;
}

int decodedSurfaceMaterial(int packed){int recipe=packed>=128?(packed-128)/64:0,base=packed>=128?(packed-128)%64:packed;int style=recipe&31;
 return style==1?0:style==2?18:style==3?19:style>=4&&style<=19?7:style==20?5:style==21?8:style==22?38:style==23?39:base;
}
