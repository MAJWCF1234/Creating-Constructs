void doorsSelfTest(){
 World preserved=world;SavedView oldView=savedView();int oldSelected=selected;bool oldWalk=walk;
 auto build=[&](int rotation){world=World{};world.n=12;resetDoorMotion();for(int x=2;x<10;x++)for(int z=2;z<10;z++)put(x,0,z,34);put(5,1,5,96,rotation);doorRoots={idx(5,1,5)};doorMotions[idx(5,1,5)]={0,rotation};origin={5.5f,1.5f,5.5f};turn=rotation;};
 for(int rotation=0;rotation<4;rotation++){
  build(rotation);int index=idx(5,1,5);V closed=transform({0,.4f,-.36f}),opened=transform({-.42f,.4f,.06f});
  check(solidAt(closed),"closed service leaf blocks the doorway in every rotation");
  V runDirection=transformNormal({0,0,1});player=transform({0,-.5f,-.90f});V runStart=player;walk=true;yaw=atan2f(-runDirection.x,-runDirection.z);pitch=0;grounded=true;velY=hx=hz=0;keys['W']=keys[VK_SHIFT]=true;
  for(int frame=0;frame<16;frame++)for(int sub=0;sub<3;sub++)playerStep(.05f/3);keys['W']=keys[VK_SHIFT]=false;
  check(dot(player-runStart,runDirection)<.4f,"closed leaf stops sprinting even at the maximum frame step");
  origin={5.5f,1.5f,5.5f};turn=rotation;
  world.doors[index]=1;doorMotions[index].amount=1;
  check(!solidAt(closed)&&solidAt(opened),"open leaf collider follows its hinge and clears the passage");
  vertices.clear();transparent.clear();activeSurface=0;piece(96);bool finite=!vertices.empty(),fits=true;
  for(auto v:vertices){finite=finite&&std::isfinite(v.p.x)&&std::isfinite(v.n.z);fits=fits&&v.p.x>=4.999f&&v.p.x<=6.001f&&v.p.z>=4.999f&&v.p.z<=6.001f&&v.p.y>=.999f&&v.p.y<=3.001f;}
  if(!fits){for(auto v:vertices)if(v.p.x<4.999f||v.p.x>6.001f||v.p.z<4.999f||v.p.z>6.001f||v.p.y<.999f||v.p.y>3.001f){char message[140];sprintf_s(message,"Door bounds: r%d point %.4f %.4f %.4f",rotation,v.p.x,v.p.y,v.p.z);log(message);break;}}
  check(finite&&fits,"open door mesh and hardware fit the reserved two-metre footprint");
  V direction=transformNormal({0,0,1});player=transform({0,-.5f,-.90f});V start=player;walk=true;yaw=atan2f(-direction.x,-direction.z);pitch=0;grounded=true;velY=hx=hz=0;keys['W']=true;
  for(int i=0;i<80;i++)playerStep(1.f/120);keys['W']=false;
  check(dot(player-start,direction)>1.8f,"standing player walks through open service door in every rotation");
 }
 build(0);int index=idx(5,1,5);walk=false;world.doors[index]=1;advanceDoors(.10f);
 check(doorAmount(index)>.05f&&doorAmount(index)<.3f,"opening advances through intermediate poses instead of snapping");
 float intermediate=doorAmount(index);world.doors.erase(index);advanceDoors(.04f);check(doorAmount(index)<intermediate,"door can reverse smoothly before opening completes");
 doorMotions[index].amount=1;world.doors.erase(index);walk=true;player={5.5f,1.f,5.14f};for(int i=0;i<90;i++)advanceDoors(1.f/60);
 check(world.doors.count(index)&&doorAmount(index)>.95f&&freeAt(player,1.6f),"closing door reopens around the player without crushing or trapping them");
 walk=false;world.doors[index]=1;doorMotions[index].amount=1;save();world=World{};check(load()&&world.doors.count(index)&&doorAmount(index)==1,"autosave preserves open door state");
 std::wstring path=savesDirectory+L"\\door-roundtrip.mns";check(writeNamedSave(path,L"Door roundtrip"),"door snapshot writes");World decoded;SavedView view{};std::wstring name;
 check(readNamedSave(path,decoded,view,name)&&decoded.doors.count(index),"named snapshots preserve open door state");
 std::string json=exportWorldJson();world=World{};check(loadWorldJson(json.c_str())&&world.doors.count(index),"Share JSON preserves open door state");
 remember();world.doors.erase(index);undoEdit();check(world.doors.count(index),"undo restores door target state");redoEdit();check(!world.doors.count(index),"redo restores door target state");
 DeleteFileW(path.c_str());world=preserved;resetDoorMotion();restoreViewpoint(oldView);walk=oldWalk;selected=oldSelected;std::fill(std::begin(keys),std::end(keys),false);history.clear();redoStack.clear();origin={};turn=0;vertices.clear();transparent.clear();dirty=true;
}
void queueDoorReviewImage(const wchar_t* name){
 if(!doorReviewImages)return;wchar_t temp[MAX_PATH];GetTempPathW(MAX_PATH,temp);std::wstring directory=std::wstring(temp)+L"CreatingConstructsDoorReview";CreateDirectoryW(directory.c_str(),nullptr);
 captureFile=directory+L"\\"+name;capturePending=true;captureAtFrame=frames+1;
}
void doorSmokeStage(){
 int index=idx(5,1,5);
 if(frames==116){world=World{};world.n=12;resetDoorMotion();for(int x=2;x<10;x++)for(int z=2;z<10;z++)put(x,0,z,34);put(5,1,5,96);walk=true;night=rain=false;player={5.5f,1,3.1f};fieldOfView=75;yaw=PI;pitch=0;velY=hx=hz=0;grounded=true;dirty=true;layout();queueDoorReviewImage(L"closed.bmp");}
 if(frames==118){check(aimedDoor()==index&&interactDoor(),"first-person aim and interaction open the rendered door");}
 if(frames==130){int code=gpu.cells[index];check(doorAmount(index)>.05f&&doorAmount(index)<.95f&&((code>>24)&127)>0&&((code-1)%128)==96,"live Vulkan door pose changes without corrupting piece identity");queueDoorReviewImage(L"opening.bmp");}
 if(frames==152)queueDoorReviewImage(L"open.bmp");
 if(frames==154){check(doorAmount(index)>.99f&&!solidAt({5.5f,2,5.14f}),"rendered door reaches fully open pose with clear collision");player={5.5f,1,4.3f};keys['W']=true;}
 if(frames==170){keys['W']=false;check(player.z>5.2f,"first-person movement crosses the animated doorway");player={5.5f,1,5.14f};world.doors.erase(index);}
 if(frames==210){check(world.doors.count(index)&&doorAmount(index)>.95f,"live door safely reopens when closing is obstructed");player={5.5f,1,3.1f};world.doors.erase(index);}
 if(frames==248){check(doorAmount(index)<.01f&&solidAt({5.5f,2,5.14f})&&((gpu.cells[index]>>24)&127)==0,"live door closes and updates both collider and GPU shadow pose");queueDoorReviewImage(L"closed-again.bmp");}
}
