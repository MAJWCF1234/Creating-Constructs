void natureSpecialSelfTest(){
 World previous=world;auto oldHistory=history,oldRedo=redoStack;V oldOrigin=origin;int oldTurn=turn;bool oldGhost=ghostMode,oldBake=bakeNatureMask;float oldAmbient=ambient;
 bakeNatureMask=false;ambient=1;ghostMode=false;
 auto reset=[&](){world=World{};origin={4.5f,2.5f,4.5f};turn=0;vertices.clear();transparent.clear();ivyBeginRefresh();};
 auto all=[&](){auto result=vertices;result.insert(result.end(),transparent.begin(),transparent.end());return result;};
 for(int rot=0;rot<4;rot++){
  bool savedAdd=hasAdd,savedRem=hasRem;V savedHitAdd=hitAdd,savedHitRem=hitRem;hasAdd=hasRem=true;hitAdd={4,2,4};V direction{0,0,-1};for(int i=0;i<rot;i++)direction={direction.z,direction.y,-direction.x};hitRem=hitAdd+direction;
  for(int type:{77,82}){int piece=type,facing=(rot+1)%4;autoFacing(piece,facing);check(piece==type&&facing==rot,"Cascade and Green wall auto-face every wall direction");}
  hasAdd=savedAdd;hasRem=savedRem;hitAdd=savedHitAdd;hitRem=savedHitRem;
  reset();put(4,1,4,4,rot);put(4,2,4,89,rot);turn=rot;
  float high=natureFloor(0,-.3f),low=natureFloor(0,.3f);check(high>low+.45f&&high<-.6f&&low> -1.5f,"Captop follows the real slope height in every rotation");
  put(4,1,4,93,rot);check(natureFloor(0,.3f)>natureFloor(0,-.3f)+.45f,"Captop follows the opposite Downslope rise");
  put(4,1,4,49,rot);check(natureFloor(0,.3f)>natureFloor(0,-.35f)+.6f,"Captop follows the visible stepped Batter surface");put(4,1,4,4,rot);
  for(int type:{88,89,91}){vertices.clear();transparent.clear();ivyBeginRefresh();natureSurfaceCover(type);auto data=all();bool valid=!data.empty();for(auto v:data)valid=valid&&std::isfinite(v.p.x)&&std::isfinite(v.n.y)&&v.m!=18;check(valid,"surface vegetation has finite meshes and adds no floating concrete plate");}
  reset();turn=rot;put(4,2,4,87,rot);put(4,3,4,0);
  for(int type:{87,90,92}){vertices.clear();transparent.clear();ivyBeginRefresh();natureHanging(type);auto first=all();float top=-100,bottom=100;for(auto v:first){top=std::max(top,v.p.y);bottom=std::min(bottom,v.p.y);}check(top<3.06f&&bottom<2.5f&&bottom>1.5f,"hanging foliage anchors below the ceiling and grows downward");
   vertices.clear();transparent.clear();ivyBeginRefresh();natureHanging(type);auto second=all();check(first.size()==second.size()&&memcmp(first.data(),second.data(),first.size()*sizeof(Vertex))==0,"hanging variation remains stable after refresh");}
  reset();turn=rot;put(4,2,4,77,rot);natureCascade();size_t solo=transparent.size();check(solo==12,"single Cascade has one flow sheet and one upper lip");
  put(4,3,4,77,rot);vertices.clear();transparent.clear();natureCascade();check(transparent.size()==6,"stacked Cascades remove buried lips");
  for(auto v:transparent)check(v.u==2,"Cascade sheet uses the falling-water shader");
  put(4,1,4,40);vertices.clear();transparent.clear();natureCascade();float bottom=100;for(auto v:transparent)bottom=std::min(bottom,v.p.y);check(fabsf(bottom-1.85f)<.001f,"Cascade reaches the adjoining water surface without a vertical gap");
  put(4,1,4,24);vertices.clear();transparent.clear();natureCascade();bottom=100;for(auto v:transparent)bottom=std::min(bottom,v.p.y);check(fabsf(bottom-1.32f)<.001f,"Cascade also reaches a shallow Pool surface");
  reset();turn=rot;put(4,2,4,82,rot);V back=transform({0,0,-1});put((int)floorf(back.x),2,(int)floorf(back.z),0);natureGreenWall();auto leaves=all();bool nearWall=leaves.size()>100;for(auto v:leaves){V q=v.p-origin;for(int j=0;j<rot;j++)q={-q.z,q.y,q.x};nearWall=nearWall&&q.z<-.25f&&q.z>-.6f;}check(nearWall,"Green wall hugs its supporting face in every rotation");
 }
 reset();put(4,3,4,12);put(4,2,4,87);natureHanging(87);bool beamAnchored=false;for(auto v:vertices)if(v.m==21&&v.p.y>3.65f){beamAnchored=true;check(fabsf(v.p.z-4.5f)<.17f,"hanging roots stay attached to a narrow beam");}check(beamAnchored,"narrow beam supplies a real hanging anchor");
 reset();put(4,2,4,77);put(5,2,4,77);natureCascade();float right=-100;for(auto v:transparent)if(v.u==2)right=std::max(right,v.p.x);check(fabsf(right-5.f)<.0001f,"neighboring Cascades meet along a shared edge");
 reset();put(4,1,4,59);put(4,3,4,91);origin={4.5f,3.5f,4.5f};check(canPlace(5,0,4,92)&&natureFloor(0,0)<-.45f,"bridge top vegetation finds the deck instead of floating above it");
 reset();put(4,1,4,17);put(4,2,4,89);check(natureFloor(0,0)<-1.f,"surface cover finds the recessed Planter soil height");
 reset();for(int type:{77,82,87,88,89,90,91,92}){put(4,2,4,type);check(!solidAt({4.5f,2.5f,4.5f}),"decorative nature does not create an invisible blocking cube");}
 reset();put(4,2,4,90);put(4,3,4,0);put(4,1,4,90);natureHanging(90);origin.y=1.5f;natureHanging(90);check(ivyLeavesAccepted>0&&ivyLeavesSkipped>0,"stacked hanging vegetation shares leaf spacing with nearby strands");
 // Exercise the actual editing history and named-save serialization for a mixed arrangement.
 reset();history.clear();redoStack.clear();put(4,2,4,77);put(4,3,4,77);put(5,2,4,87);put(6,2,4,89,2);World arrangement=world;
 remember();put(5,2,4,-1);undoEdit();check(get(5,2,4).t==87,"undo restores hanging vegetation in an arrangement");redoEdit();check(get(5,2,4).t<0,"redo removes the same vegetation without affecting neighboring water");undoEdit();
 std::wstring path=savesDirectory+L"\\nature-interactions-test.mns";check(writeNamedSave(path,L"Nature interaction test"),"nature arrangement writes a named save");World restored;SavedView view{};std::wstring title;check(readNamedSave(path,restored,view,title)&&memcmp(restored.c.data(),arrangement.c.data(),sizeof(Cell)*WorldCells)==0,"save/load retains stacked and rotated nature interactions");DeleteFileW(path.c_str());
 world=previous;history=oldHistory;redoStack=oldRedo;origin=oldOrigin;turn=oldTurn;ghostMode=oldGhost;bakeNatureMask=oldBake;ambient=oldAmbient;vertices.clear();transparent.clear();ivyBeginRefresh();dirty=true;
}
