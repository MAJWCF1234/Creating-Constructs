void serviceDoorFrame(){
 box(.08f,2.f,.22f,-.46f,.5f,-.36f,8);box(.08f,2.f,.22f,.46f,.5f,-.36f,8);box(.84f,.10f,.22f,0,1.45f,-.36f,8);
 box(.018f,1.86f,.025f,.413f,.45f,-.32f,3); // latch stop and strike plate
 box(.025f,.20f,.035f,.407f,.40f,-.30f,8);
 V previous=origin;origin=origin+transformNormal({-.42f,0,-.36f});for(float y:{-.22f,.50f,1.17f})cylinder(.033f,.13f,y,3,8);origin=previous;
}
void serviceDoorLeaf(float amount){
 doorHingeActive=true;doorHingeAngle=doorEase(amount)*PI*.5f;
 box(.82f,1.84f,.060f,0,.46f,-.36f,52);
 box(.70f,.68f,.008f,0,.82f,-.394f,52);box(.70f,.68f,.008f,0,.82f,-.326f,52);
 box(.70f,.53f,.008f,0,-.13f,-.394f,52);box(.70f,.53f,.008f,0,-.13f,-.326f,52);
 for(float z:{-.414f,-.306f}){
  box(.045f,.16f,.012f,.30f,.40f,z,3);box(.035f,.035f,.045f,.30f,.43f,z,8);box(.15f,.025f,.025f,.245f,.43f,z+(z<-.36f?-.010f:.010f),8);
 }
 doorHingeActive=false;doorHingeAngle=0;
}
bool doorPlayerObstructs(int index,int rotation,float amount){
 if(!walk)return false;float height=keys['C']||keys[VK_CONTROL]?1.f:1.6f;
 return doorBodyTouches(player,height,index,rotation,amount,.29f);
}
void advanceDoors(float dt){
 for(int index:doorRoots){
  auto cell=world.c[index];if(cell.t!=96)continue;auto& motion=doorMotions[index];
  for(int sub=0;sub<3;sub++){
   float target=world.doors.count(index)?1.f:0.f;if(fabsf(target-motion.amount)<.0001f)break;
   float next=motion.amount+std::clamp(target-motion.amount,-dt*.58f,dt*.58f);
   if(doorPlayerObstructs(index,cell.r,next)){if(target<.5f){world.doors[index]=1;save();}break;}
   bool finished=fabsf(target-next)<.0001f;motion.amount=next;if(finished)audioEvent(A_click,.18f,.65f);
  }
  int bits=doorGpuBits(index),x=index/(MaxPlot*WorldHeight),z=(index/WorldHeight)%MaxPlot,y=index%WorldHeight;
  for(int h=0;h<2;h++)if(inb(x,y+h,z)){int k=idx(x,y+h,z);gpu.cells[k]=(gpu.cells[k]&0x00ffffff)|bits;}
 }
}
void renderDoorLeaves(){
 for(int index:doorRoots){auto cell=world.c[index];if(cell.t!=96)continue;
  int x=index/(MaxPlot*WorldHeight),z=(index/WorldHeight)%MaxPlot,y=index%WorldHeight;
  origin={x+.5f,y+.5f,z+.5f};turn=cell.r;ambient=1;activeSurface=surfaceAt(index);serviceDoorLeaf(doorAmount(index));
 }origin={};turn=0;ambient=1;activeSurface=0;
}
int aimedDoor(){
 if(!walk)return -1;
 for(float distance=.10f;distance<=2.6f;distance+=.035f){V q=eye+forward*distance;int x=(int)floorf(q.x),y=(int)floorf(q.y),z=(int)floorf(q.z),bx,by,bz;
  if(footprintRoot(x,y,z,bx,by,bz)&&get(bx,by,bz).t==96)return idx(bx,by,bz);
  if(solidAt(q))return -1;
 }return -1;
}
bool interactDoor(){
 int index=aimedDoor();if(index<0)return false;remember();if(world.doors.count(index))world.doors.erase(index);else world.doors[index]=1;
 audioEvent(A_click,.28f,.75f);save();return true;
}
void updateDoorHint(){
 std::string next;int index=aimedDoor();if(index>=0)next=world.doors.count(index)?"F / click: close door":"F / click: open door";
 if(next!=doorHint){doorHint=next;if(topPanel)InvalidateRect(topPanel,nullptr,FALSE);}
}
