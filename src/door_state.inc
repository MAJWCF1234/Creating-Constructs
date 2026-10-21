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
