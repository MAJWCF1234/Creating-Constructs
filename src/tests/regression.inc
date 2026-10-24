// Optional QA images use the same Vulkan image and native UI painter as the window.
void menuReviewImage(const wchar_t* name){
 if(!menuReviewImages||!uiBackgroundReady)return;
 BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=W;info.bmiHeader.biHeight=-H;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
 void* pixels=nullptr;HDC dc=CreateCompatibleDC(nullptr);HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);auto previous=SelectObject(dc,bitmap);
 BITMAPINFO source=info;source.bmiHeader.biWidth=uiBackgroundWidth;source.bmiHeader.biHeight=-(int)uiBackgroundHeight;
 StretchDIBits(dc,0,0,W,H,0,0,uiBackgroundWidth,uiBackgroundHeight,uiBackgroundMap,&source,DIB_RGB_COLORS,SRCCOPY);composeUi(dc);GdiFlush();
 wchar_t temp[MAX_PATH];GetTempPathW(MAX_PATH,temp);std::wstring directory=std::wstring(temp)+L"CreatingConstructsMenuReview";CreateDirectoryW(directory.c_str(),nullptr);
 BITMAPFILEHEADER file{};file.bfType=0x4d42;file.bfOffBits=sizeof(file)+sizeof(BITMAPINFOHEADER);file.bfSize=file.bfOffBits+W*H*4;FILE* f=nullptr;_wfopen_s(&f,(directory+L"\\"+name).c_str(),L"wb");if(f){fwrite(&file,sizeof(file),1,f);fwrite(&info.bmiHeader,sizeof(BITMAPINFOHEADER),1,f);fwrite(pixels,4,(size_t)W*H,f);fclose(f);}
 SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
}
void clickMenuAction(int id){
 rebuildActions();for(auto a:actions)if(a.action==id){RECT panel;GetWindowRect(modalPanel,&panel);POINT p{panel.left,panel.top};ScreenToClient(mainWindow,&p);int x=(a.rect.left+a.rect.right)/2-p.x,y=(a.rect.top+a.rect.bottom)/2-p.y;SendMessageW(modalPanel,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));SendMessageW(modalPanel,WM_LBUTTONUP,0,MAKELPARAM(x,y));return;}
 check(false,"menu action has a clickable rectangle");
}
void menuSmokeStage(){
if(frames==2){keys['W']=true;showUiModal(6);check(menuActive&&uiModal==6&&!keys['W']&&!IsWindowVisible(dockPanel),"main menu clears input and hides the editor");check(actions.size()==5,"main menu exposes five isolated actions");bool priorWalk=walk;V priorPlayer=player;float oldVelocity=velY;walk=true;keys['W']=true;camera(.05f);check(len(player-priorPlayer)<.00001f&&velY==oldVelocity,"main menu pauses first-person movement and gravity");keys['W']=false;walk=priorWalk;}
if(frames==3){clickMenuAction(62);check(uiModal==7&&menuActive,"menu settings opens without entering the world");float prior=musicVolume;setUiSlider(304,.31f);check(fabsf(musicVolume-.31f)<.001f,"menu audio slider changes the real mixer");musicVolume=prior;action(22);check(uiModal==6,"settings Back returns to main menu");}
if(frames==4){clickMenuAction(61);check(uiModal==3&&menuActive&&IsWindowVisible(saveNameEdit),"main menu opens the real saved world browser");SetWindowTextW(saveNameEdit,L"Menu load test");action(31);action(32);check(!menuActive&&uiModal==0,"loading a saved world from the main menu enters the game");showUiModal(6);action(63);check(uiModal==1&&menuActive,"main menu controls opens");keyDown(VK_ESCAPE,false);check(uiModal==6,"Escape returns from controls to main menu");}
if(frames==5){keyDown(VK_DOWN,false);check(menuSelection==1,"menu keyboard navigation selects saved worlds");keyDown(VK_UP,false);keyDown(VK_RETURN,false);check(!menuActive&&uiModal==0&&IsWindowVisible(dockPanel),"Play restores editor and resumes gameplay");keyDown(VK_ESCAPE,false);check(menuActive,"Escape opens pause menu in game");keyDown(VK_ESCAPE,false);check(!menuActive,"Escape resumes from pause menu");}
if(menuReviewImages&&frames==6){showUiModal(6);menuReviewImage(L"main-menu.bmp");}
if(menuReviewImages&&frames==7){action(62);menuReviewImage(L"settings.bmp");}
if(menuReviewImages&&frames==8){action(22);action(61);menuReviewImage(L"saved-worlds.bmp");}
if(menuReviewImages&&frames==9){action(22);action(60);}
}

void audioSelfTest(){
 FootstepCadence steps;
 bool silent=true;for(int i=0;i<120;i++)silent=silent&&!steps.advance(0,true,false,false);check(silent,"standing still produces no footsteps");
 bool emitted=false;for(int i=0;i<60;i++)emitted=steps.advance(.07f,true,false,false)||emitted;
 check(emitted&&steps.serial>=2,"grounded displacement produces spaced walking steps");
 unsigned count=steps.serial;for(int i=0;i<60;i++)steps.advance(.1f,false,false,false);
 check(steps.serial==count,"airborne travel never produces footsteps");
 MovementVariations variants;bool varied=true;
 for(int surface=0;surface<5;surface++){int last=-1;std::vector<int> heard;for(int i=0;i<128;i++){int clip=variants.choose(surface);varied=varied&&clip!=last;last=clip;if(std::find(heard.begin(),heard.end(),clip)==heard.end())heard.push_back(clip);}varied=varied&&heard.size()==(surface==4?6:4);}
 check(varied,"movement banks use every variation without immediate repeats");
 WaterCueGate gate;check(gate.admit(1000)&&!gate.admit(1001)&&!gate.admit(1419)&&gate.admit(1420),"all water variations share a 420 ms anti-overlap gate");
 World originalWorld=world;world=World{};put(1,0,1,24);put(2,0,1,5);put(3,0,1,40);
 check(movementSurface({1.5f,.1f,1.5f})==4&&movementSurface({1.05f,.5f,1.5f})==0,"splash cues detect the pool interior and exclude its concrete rim");
 check(movementSurface({2.5f,1,1.5f})==3&&movementSurface({3.5f,.2f,1.5f})==4&&movementSurface({3.5f,.9f,1.5f})!=4,"movement distinguishes soft ground and actual water height");world=originalWorld;

 for(int i=0;i<A_Count;i++){
  auto& c=audioClips[i];check(c.pcm.size()>100&&c.rate>=12000,"embedded ADPCM clip decodes");
  bool finite=true;float pk=0;for(float v:c.pcm){finite=finite&&std::isfinite(v);pk=std::max(pk,fabsf(v));}
  check(finite&&pk<.32f,"curated source stays beneath safe digital peak ceiling");
  if(i>=A_stepStone1)check(pk<(isWaterStep(i)?.035f:.09f)&&c.pcm.size()/(float)c.rate<.42f,"movement source has a short tail and a restrained peak ceiling");
 }
 for(int i=0;i<3;i++)check(fabsf(audioClips[i].pcm.front()-audioClips[i].pcm.back())<.08f,"music and ambience loops join without a discontinuity");

 for(auto& c:moodMusic){
  check(c.pcm.size()>20*c.rate,"each new mood has its own complete looping score");
  bool valid=true;double energy=0;for(float v:c.pcm){valid=valid&&std::isfinite(v)&&fabsf(v)<.32f;energy+=v*v;}
  check(valid&&sqrt(energy/c.pcm.size())>.01,"mood scores retain gentle peaks and audible musical content");
  check(fabsf(c.pcm.front()-c.pcm.back())<.08f,"generated mood scores wrap without a discontinuity");
 }
 wchar_t temp[MAX_PATH];GetTempPathW(MAX_PATH,temp);
 std::wstring preview=std::wstring(temp)+L"creating-constructs-audio-preview.wav";
 FILE* f=nullptr;_wfopen_s(&f,preview.c_str(),L"wb");check(f!=nullptr,"audio review mix can be written");
 const uint32_t frames=20*AudioMixer::Rate;uint32_t bytes=frames*4;
 fwrite("RIFF",1,4,f);uint32_t n=bytes+36;fwrite(&n,4,1,f);fwrite("WAVEfmt ",1,8,f);n=16;fwrite(&n,4,1,f);
 uint16_t v=1;fwrite(&v,2,1,f);v=2;fwrite(&v,2,1,f);n=AudioMixer::Rate;fwrite(&n,4,1,f);n*=4;fwrite(&n,4,1,f);v=4;fwrite(&v,2,1,f);v=16;fwrite(&v,2,1,f);fwrite("data",1,4,f);fwrite(&bytes,4,1,f);
 AudioMixer test;test.nightTarget.store(1);test.rainTarget.store(0);test.focus.store(.8f);
 int16_t buffer[AudioMixer::Block*2];
 for(int pos=0;pos<75*AudioMixer::Rate;pos+=AudioMixer::Block){
  int frame=pos/AudioMixer::Block;
  // Build, destroy and UI feedback overlap, then run past every loop boundary.
  if(pos<18*AudioMixer::Rate&&frame%75==0){unsigned w=test.write.load();int id=A_placeStone+(frame/75)%(A_slider-A_placeStone+1);test.queue[w%128]={id,1,1,0};test.write.store(w+1);}
  if(pos>12*AudioMixer::Rate)test.rainTarget.store(1);
  test.mix(buffer,AudioMixer::Block);
  if(pos<(int)frames)fwrite(buffer,4,std::min(AudioMixer::Block,(int)frames-pos),f);
 }
 fclose(f);check(test.mixedEvents>=20,"all interaction cues mix over looping music and ambience");
 // Movement-only listening sequence, rendered through the actual runtime mixer at defaults.
 std::wstring movementReview=std::wstring(temp)+L"creating-constructs-movement-review.wav";FILE* movementFile=nullptr;_wfopen_s(&movementFile,movementReview.c_str(),L"wb");check(movementFile!=nullptr,"first-person listening review opens");
 uint32_t movementBytes=16*AudioMixer::Rate*4,movementRiff=movementBytes+36;unsigned char movementHeader[44];
 FILE* headerFile=nullptr;_wfopen_s(&headerFile,preview.c_str(),L"rb");fread(movementHeader,1,44,headerFile);fclose(headerFile);memcpy(movementHeader+4,&movementRiff,4);memcpy(movementHeader+40,&movementBytes,4);fwrite(movementHeader,1,44,movementFile);
 AudioMixer movement;movement.music.store(0);movement.ambience.store(0);movement.focus.store(.8f);MovementVariations reviewVariants;int lastBeat=-1;
 for(int pos=0;pos<16*AudioMixer::Rate;pos+=AudioMixer::Block){
  int beat=pos/(AudioMixer::Rate/2),surface=std::min(4,pos/(3*AudioMixer::Rate));
  if(beat!=lastBeat){lastBeat=beat;unsigned w=movement.write.load();movement.queue[w%128]={reviewVariants.choose(surface),.54f,1.f,beat%2?.045f:-.045f};movement.write.store(w+1);}
  movement.mix(buffer,AudioMixer::Block);fwrite(buffer,4,std::min(AudioMixer::Block,16*AudioMixer::Rate-pos),movementFile);
 }
 fclose(movementFile);check(movement.peak<.04f&&movement.limited==0,"first-person review retains restrained peaks without limiter pumping");
 check(test.peak>.01f&&test.peak<.85f&&test.limited==0,"75-second default mix retains headroom without clipping or limiter pumping");
 test.focus.store(0);for(int i=0;i<100;i++)test.mix(buffer,AudioMixer::Block);
 int peak=0;for(int16_t value:buffer)peak=std::max(peak,abs((int)value));check(peak<=1,"focus loss and mute fade output to silence");
 test.music.store(0);test.ambience.store(0);test.effects.store(0);test.focus.store(.8f);
 for(int i=0;i<1600;i++)test.mix(buffer,AudioMixer::Block);
 peak=0;for(int16_t value:buffer)peak=std::max(peak,abs((int)value));check(peak<=1,"all three volume controls reach silence");

 // Exercise all four soundtracks, including mid-transition reversals.
 AudioMixer moods;moods.focus.store(.8f);bool unity=true;double transitionJump=0;int16_t last=0;
 std::wstring moodReview=std::wstring(temp)+L"creating-constructs-mood-review.wav";FILE* review=nullptr;_wfopen_s(&review,moodReview.c_str(),L"wb");check(review!=nullptr,"mood listening preview opens");
 // Reuse the valid stereo header, then patch its length for 48 seconds.
 FILE* original=nullptr;_wfopen_s(&original,preview.c_str(),L"rb");unsigned char header[44];fread(header,1,44,original);fclose(original);uint32_t dataBytes=48*AudioMixer::Rate*4,riffBytes=dataBytes+36;memcpy(header+4,&riffBytes,4);memcpy(header+40,&dataBytes,4);fwrite(header,1,44,review);
 for(int pos=0;pos<48*AudioMixer::Rate;pos+=AudioMixer::Block){
  int selected=std::min(3,pos/(12*AudioMixer::Rate));moods.moodTarget.store(selected);
  moods.mix(buffer,AudioMixer::Block);
  transitionJump=std::max(transitionJump,abs((int)buffer[0]-last)/32768.);last=buffer[AudioMixer::Block*2-2];
  fwrite(buffer,4,std::min(AudioMixer::Block,48*AudioMixer::Rate-pos),review);
  float sum=0;for(float w:moods.moodWeights)sum+=w;unity=unity&&fabsf(sum-1)<.001f;
 }
 fclose(review);check(unity,"mood crossfade retains unity total gain");check(moods.peak<.85f&&moods.limited==0,"four-track crossfades preserve mix headroom");
 check(transitionJump<.1,"mood changes introduce no abrupt sample jump");
 moods.moodTarget.store(1);moods.mix(buffer,AudioMixer::Block);moods.moodTarget.store(2);moods.mix(buffer,AudioMixer::Block);moods.moodTarget.store(0);
 for(int i=0;i<800;i++)moods.mix(buffer,AudioMixer::Block);
 check(moods.moodWeights[0]>.999f,"rapid mood switches settle on the latest requested soundtrack");
 // Dump decoded PCM so the build check can compare it against FFmpeg's decoder.
 std::wstring decoded=std::wstring(temp)+L"creating-constructs-audio-decoded.bin";_wfopen_s(&f,decoded.c_str(),L"wb");check(f!=nullptr,"ADPCM decoder parity output opens");
 for(auto& c:audioClips){uint32_t count=(uint32_t)c.pcm.size();fwrite(&count,4,1,f);for(float sample:c.pcm){int16_t value=(int16_t)lroundf(sample*32768);fwrite(&value,2,1,f);}}fclose(f);
 char b[160];sprintf_s(b,"Audio offline mix peak %.2f dBFS; %u events; 75 seconds plus fade tests",20*log10(test.peak),test.mixedEvents);log(b);
}

void savesSelfTest(){
 World original=world;SavedView originalView=savedView();std::wstring directory=savesDirectory;
 wchar_t child[80];swprintf_s(child,L"\\run-%llu",GetTickCount64());savesDirectory+=child;CreateDirectoryW(savesDirectory.c_str(),nullptr);
 world=World{};for(int t=0;t<PieceCount;t++)put(t%10,t/10,3,t,t%4);world.cables={{{1,2,3},{5,6,7}}};world.pipes={{1,2,3,4,1,2,2,3,0},{2,3,4,0,2,1,1,1,1}};mood=3;night=true;rain=true;target={2,3,4};theta=1.3f;
 check(saveCurrentNamed(L"  Quiet courtyard \x2601  "),"named save creates a world snapshot");std::wstring first=saveEntries[saveSelected].path;
 World expected=world,decoded;SavedView view{};std::wstring name;
 check(readNamedSave(first,decoded,view,name),"named save reloads from disk");
 check(name==L"Quiet courtyard \x2601"&&decoded.n==world.n&&memcmp(decoded.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0&&decoded.cables.size()==1,"named save preserves Unicode name, all piece types, rotations and cables");
 check(decoded.pipes.size()==world.pipes.size()&&memcmp(decoded.pipes.data(),world.pipes.data(),world.pipes.size()*sizeof(PipeNode))==0,"named save preserves pipe kinds, faces, slots, rotations and auto adaptation flags");
 {std::wstring truncated=savesDirectory+L"\\truncated-pipes.mns";FILE* in=nullptr;_wfopen_s(&in,first.c_str(),L"rb");fseek(in,0,SEEK_END);long bytes=ftell(in);rewind(in);std::vector<unsigned char> raw(bytes);fread(raw.data(),1,raw.size(),in);fclose(in);FILE* out=nullptr;_wfopen_s(&out,truncated.c_str(),L"wb");fwrite(raw.data(),1,raw.size()-1,out);fclose(out);World bad;SavedView unused{};std::wstring title;check(!readNamedSave(truncated,bad,unused,title),"truncated pipe snapshots are rejected");DeleteFileW(truncated.c_str());}
 check(view.mood==3&&(view.flags&3)==3&&view.target.x==2&&view.theta==1.3f,"named save retains atmosphere and camera");
 world=World{};put(0,0,0,19);mood=1;World previous=world;
 check(loadSelectedNamed(),"selected named save loads successfully");
 check(mood==3&&memcmp(world.c.data(),expected.c.data(),sizeof(Cell)*WorldCells)==0,"loading restores world and mood");
 check(readNamedSave(savesDirectory+L"\\recovery.mns",decoded,view,name)&&memcmp(decoded.c.data(),previous.c.data(),sizeof(Cell)*WorldCells)==0&&view.mood==1,"loading preserves the previous world in recovery");
 check(saveCurrentNamed(L"Quiet courtyard \x2601"),"duplicate names save as separate copies");std::wstring second=saveEntries[saveSelected].path;check(first!=second,"saving a duplicate name does not overwrite an earlier save");
 std::wstring invalid=savesDirectory+L"\\broken.mns";FILE* f=nullptr;_wfopen_s(&f,invalid.c_str(),L"wb");fwrite("broken",1,6,f);fclose(f);
 SaveEntry bad;bad.path=invalid;saveEntries.push_back(bad);saveSelected=(int)saveEntries.size()-1;World before=world;
 check(!loadSelectedNamed()&&memcmp(before.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"corrupt save is rejected without changing the current world");
 refreshNamedSaves();check(saveEntries.size()==3,"save browser lists good worlds and skips corrupt files");
 DeleteFileW(first.c_str());DeleteFileW(second.c_str());DeleteFileW(invalid.c_str());DeleteFileW((savesDirectory+L"\\recovery.mns").c_str());RemoveDirectoryW(savesDirectory.c_str());
 world=original;target=originalView.target;player=originalView.player;theta=originalView.theta;phi=originalView.phi;distanceCam=originalView.distance;yaw=originalView.yaw;pitch=originalView.pitch;fieldOfView=originalView.fov;sunHeight=originalView.sunHeight;sunAngle=originalView.sunAngle;exposure=originalView.exposure;fog=originalView.fog;moodSat=originalView.sat;mood=originalView.mood;night=originalView.flags&1;rain=originalView.flags&2;ao=originalView.flags&4;walk=originalView.flags&8;
 savesDirectory=directory;refreshNamedSaves();history.clear();dirty=true;
}

void buildingSelfTest(){
 World preserved=world;SavedView oldView=savedView();int oldStyle=chosenSurface,oldRotation=surfaceRotation,oldScale=surfaceScale;
 world=World{};world.n=16;put(4,1,4,18);chosenSurface=18;surfaceRotation=3;surfaceScale=2;
 world.surfaces[idx(4,1,4)]=(uint16_t)surfaceRecipe();Viewpoint point;wcscpy_s(point.name,L"Garden light");point.view=savedView();world.viewpoints.push_back(point);
 save();world=World{};check(load()&&surfaceAt(idx(4,1,4))==(18+96+256)&&world.viewpoints.size()==1,"autosave preserves surface variant, texture transform and named viewpoints");
 std::wstring path=savesDirectory+L"\\surface-roundtrip.mns";check(writeNamedSave(path,L"Surface test"),"extended named world writes");World decoded;SavedView view{};std::wstring title;
 check(readNamedSave(path,decoded,view,title)&&decoded.surfaces==world.surfaces&&decoded.viewpoints.size()==1&&wcscmp(decoded.viewpoints[0].name,L"Garden light")==0,"named world preserves materials and viewpoint names");
 std::string shared=exportWorldJson();world=World{};check(loadWorldJson(shared.c_str())&&surfaceAt(idx(4,1,4))==(18+96+256)&&world.viewpoints.size()==1,"Share JSON preserves painted material transforms");
 world=decoded;selected=4;int oldCategory=category;category=5;World unchanged=world;action(605);
 check(selected==4&&memcmp(world.c.data(),unchanged.c.data(),sizeof(Cell)*WorldCells)==0&&world.surfaces==unchanged.surfaces,"Material selection changes future finish without painting existing pieces or changing shape");category=oldCategory;
 world=decoded;remember();world.surfaces[idx(4,1,4)]=5;undoEdit();check(surfaceAt(idx(4,1,4))==(18+96+256),"undo restores painted finish");redoEdit();check(surfaceAt(idx(4,1,4))==5,"redo restores painted finish");
 for(int style=1;style<SurfaceCount;style++)for(int rot=0;rot<4;rot++)for(int scale=0;scale<3;scale++){
  activeSurface=style+rot*32+scale*128;vertices.clear();transparent.clear();origin={4.5f,1.5f,4.5f};turn=0;piece(18);
  bool valid=!vertices.empty();for(auto& v:vertices)valid=valid&&v.m>=128&&std::isfinite(v.p.x);if(!valid)check(false,"all surface transform combinations generate finite tagged geometry");
 }check(true,"276 surface variant and transform combinations generate finite geometry");activeSurface=0;
 world=World{};world.n=16;put(5,0,5,0);hasAdd=true;hasRem=false;hitAdd={5,1,5};selected=6;rotation=0;World before=world;vertices.clear();transparent.clear();placementPreview();
 check(memcmp(before.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0&&world.surfaces==before.surfaces,"adaptation preview leaves authored cells and materials untouched");
 put(5,1,5,0);vertices.clear();transparent.clear();placementPreview();bool blocked=false;for(auto v:transparent)blocked|=(int)v.m==44;check(blocked&&placementStatus.find("Blocked")!=std::string::npos,"blocked footprint generates red preview and explicit feedback");
 world=World{};world.n=16;put(5,0,5,0);hasAdd=hit=true;hasRem=false;hitAdd={5,1,5};selected=4;chosenSurface=5;surfaceScale=1;surfaceRotation=2;edit();check(get(5,1,5).t==4&&surfaceAt(idx(5,1,5))==197,"placing retains chosen shape and applies active material to the new piece");
 world=World{};put(5,1,5,96);check(!canPlace(5,2,5,0)&&freeAt({5.5f,1.02f,5.5f},1.6f),"service entry reserves overhead cell and admits a standing player");
 DeleteFileW(path.c_str());world=preserved;restoreViewpoint(oldView);chosenSurface=oldStyle;surfaceRotation=oldRotation;surfaceScale=oldScale;hasAdd=hasRem=hit=false;selected=0;activeSurface=0;placementStatus.clear();history.clear();redoStack.clear();dirty=true;
}

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

void natureSelfTest(){
 World previous=world;V oldOrigin=origin;int oldTurn=turn;float oldAmbient=ambient;
 world=World{};put(5,0,5,14);origin={5.5f,.5f,5.5f};turn=0;ambient=1;
 while(natureTreeVariant(natureSeed())>=6)origin.x+=1;
 std::fill(std::begin(gpu.natureMask),std::end(gpu.natureMask),0u);bakeNatureMask=true;vertices.clear();transparent.clear();natureTree();
 float top=-100;bool uv=false,bark=false,leaves=false,finite=true;
 for(auto v:vertices){top=std::max(top,v.p.y);uv=uv||v.u!=0||v.v!=0;bark=bark||v.m==23;leaves=leaves||v.m==22;finite=finite&&std::isfinite(v.p.x)&&std::isfinite(v.n.y);}
 check(top>3.8f&&bark&&leaves&&uv&&finite,"imported tree has full-height branching bark and UV-mapped leaf geometry");
 auto first=vertices;vertices.clear();natureTree();check(vertices.size()==first.size()&&memcmp(vertices.data(),first.data(),first.size()*sizeof(Vertex))==0,"tree placement regenerates deterministically");
 int occupied=0;for(auto m:gpu.natureMask)if(m)occupied++;check(occupied>20,"tree alpha geometry fills foliage shadow mask above its root cell");
 auto mask=std::array<uint32_t,WorldCells*2>{};std::copy(std::begin(gpu.natureMask),std::end(gpu.natureMask),mask.begin());
 vertices.clear();ghostMode=true;natureTree();ghostMode=false;bool ghost=true;for(auto v:vertices)ghost=ghost&&v.m==10;
 check(ghost&&std::equal(mask.begin(),mask.end(),std::begin(gpu.natureMask)),"tree placement preview does not alter baked nature shadows");
 vertices.clear();bakeNatureMask=false;origin={2.5f,.5f,2.5f};natureUnderstory(.51f,false);int fern=0,grass=0;for(auto v:vertices){fern+=v.m==24;grass+=v.m==25||v.m==26;}
 check(fern>0&&grass>0&&vertices.size()<5000,"understory combines bounded real fern and grass meshes");
 auto ground=vertices;vertices.clear();natureUnderstory(.51f,false);
 check(vertices.size()==ground.size()&&memcmp(vertices.data(),ground.data(),ground.size()*sizeof(Vertex))==0,"random understory remains stable through geometry refreshes");
 bool variants[8]{};bool materials[NatureTextureCount]{};bool bounded=true;
 for(int x=0;x<12;x++)for(int z=0;z<12;z++){
  origin={x+.5f,.5f,z+.5f};variants[natureTreeVariant(natureSeed())]=true;
  vertices.clear();natureUnderstory(.51f,false);bounded=bounded&&vertices.size()<5000;
  for(auto v:vertices)if(v.m>=22&&v.m<22+NatureTextureCount)materials[(int)v.m-22]=true;
 }
 bool allTrees=true,allPlants=true;for(bool v:variants)allTrees=allTrees&&v;for(int i=2;i<9&&i<NatureTextureCount;i++)allPlants=allPlants&&materials[i];
 check(allTrees,"placement seeds distribute every tree silhouette across the plot");
 check(allPlants&&bounded,"random plant communities include all imported species within the per-tile geometry budget");
 world=previous;origin=oldOrigin;turn=oldTurn;ambient=oldAmbient;vertices.clear();transparent.clear();std::fill(std::begin(gpu.natureMask),std::end(gpu.natureMask),0u);dirty=true;
}

void worldSizeSelfTest(){
 World previous=world;std::wstring oldPath=dataPath;world=World{};world.n=32;
 put(31,19,31,19,3);put(16,2,23,14,2);put(0,0,31,5);world.cables={{{1,4,2},{31,5,30}}};World expected=world;
 check(idx(31,19,31)==31*(MaxPlot*WorldHeight)+31*WorldHeight+19&&get(31,19,31).t==19,"32x32 grid indexes its far corner without overlap");
 save();world=World{};check(load()&&world.n==32&&memcmp(expected.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"32x32 autosave round trip retains far-edge cells");
 std::string json=exportWorldJson();check(json.find("\"stride\":64")!=std::string::npos&&loadWorldJson(json.c_str())&&world.n==32&&get(31,19,31).t==19&&get(16,2,23).t==14,"Share JSON preserves positions beyond the old 16-tile boundary");
 std::wstring named=savesDirectory+L"\\wide-test.mns",name;SavedView v{};World decoded;
 check(writeNamedSave(named,L"32 tile world")&&readNamedSave(named,decoded,v,name)&&decoded.n==32&&memcmp(decoded.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"named saves retain the complete 32x32 world");DeleteFileW(named.c_str());
 world=World{};world.n=64;put(63,19,63,19,1);put(48,2,50,72,0);World wide=world;
 check(idx(63,19,63)==WorldCells-1&&get(63,19,63).t==19,"64x64 grid indexes its far corner without overlap");
 save();world=World{};check(load()&&world.n==64&&memcmp(wide.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"64x64 autosave round trip retains far-edge cells");
 json=exportWorldJson();check(loadWorldJson(json.c_str())&&world.n==64&&get(63,19,63).t==19&&get(48,2,50).t==72,"Share JSON preserves 64-tile dense keys including Roundel");
 named=savesDirectory+L"\\huge-test.mns";check(writeNamedSave(named,L"64 tile world")&&readNamedSave(named,decoded,v,name)&&decoded.n==64&&decoded.c[idx(63,19,63)].t==19,"named saves retain the complete 64x64 world");DeleteFileW(named.c_str());
 // Independent fixtures in the original binary layouts prove backward compatibility.
 std::array<Cell,5120> legacy;legacy[15*320+15*20+2]={19,3};legacy[2*320+3*20+4]={15,1};uint32_t count=0,magic=0x324f4e4d;int size=16;
 std::wstring legacyPath=savesDirectory+L"\\legacy-test.bin";FILE* f=nullptr;_wfopen_s(&f,legacyPath.c_str(),L"wb");fwrite(&magic,4,1,f);fwrite(&size,4,1,f);fwrite(legacy.data(),sizeof(Cell),legacy.size(),f);fwrite(&count,4,1,f);fclose(f);
 dataPath=legacyPath;check(load()&&world.n==16&&get(15,2,15).t==19&&get(2,4,3).t==15,"old 16x16 autosaves migrate with their original coordinates");DeleteFileW(legacyPath.c_str());
 std::wstring legacySlot=savesDirectory+L"\\legacy-test.mns";magic=0x31534e4d;wchar_t title[64]=L"Legacy courtyard";v=savedView();_wfopen_s(&f,legacySlot.c_str(),L"wb");fwrite(&magic,4,1,f);fwrite(title,sizeof(title),1,f);fwrite(&v,sizeof(v),1,f);fwrite(&size,4,1,f);fwrite(legacy.data(),sizeof(Cell),legacy.size(),f);fwrite(&count,4,1,f);fclose(f);
 check(readNamedSave(legacySlot,decoded,v,name)&&decoded.n==16&&decoded.c[idx(15,2,15)].t==19,"old named saves migrate to the wider stride");DeleteFileW(legacySlot.c_str());
 // Legacy Share without stride still decodes the old packing.
 world=World{};check(loadWorldJson("{\"N\":10,\"c\":[[1059,{\"t\":0,\"r\":1}]]}")&&get(1,3,1).t==0&&get(1,3,1).r==1,"legacy Share keys without stride still import");
 dataPath=oldPath;world=previous;history.clear();dirty=true;
}

void waterSelfTest(){
 World previous=world;V oldOrigin=origin;int oldTurn=turn;world=World{};origin={.5f,.5f,.5f};turn=0;
 put(0,0,0,40);vertices.clear();transparent.clear();waterSurface();check(transparent.size()==30,"single water tile has one top and four exposed sides");
 put(1,0,0,40);vertices.clear();transparent.clear();waterSurface();origin={1.5f,.5f,.5f};waterSurface();bool seam=false;
 for(size_t i=0;i+2<transparent.size();i+=3)seam=seam||(fabsf(transparent[i].p.x-1)<.0001f&&fabsf(transparent[i+1].p.x-1)<.0001f&&fabsf(transparent[i+2].p.x-1)<.0001f);
 check(transparent.size()==48&&!seam,"adjacent water tiles omit their hidden interior faces");
 put(0,1,0,40);vertices.clear();transparent.clear();origin={.5f,.5f,.5f};waterSurface();bool top=false;
 for(size_t i=0;i+2<transparent.size();i+=3)top=top||(fabsf(transparent[i].p.y-1)<.0001f&&fabsf(transparent[i+1].p.y-1)<.0001f&&fabsf(transparent[i+2].p.y-1)<.0001f);
 check(!top,"stacked water omits buried surface caps");
 check(cameraWaterDepth({.5f,.5f,.5f})>1.3f&&cameraWaterDepth({.5f,1.9f,.5f})==0,"underwater detection follows the continuous water column");
 world=previous;origin=oldOrigin;turn=oldTurn;vertices.clear();transparent.clear();dirty=true;
}

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

void pipesSelfTest(){
 World original=world;auto oldHistory=history,oldRedo=redoStack;int oldTurn=turn;V oldOrigin=origin;bool oldGhost=ghostMode;ghostMode=false;origin={};turn=0;
 world=World{};world.n=10;for(int x=2;x<7;x++)for(int z=2;z<7;z++)for(int y=1;y<6;y++)put(x,y,z,0);
 const int du[4]={0,0,1,-1},dv[4]={1,-1,0,0};bool steps=true,reciprocal=true;
 for(int face=0;face<6;face++)for(int u=0;u<3;u++)for(int v=0;v<3;v++)for(int d=0;d<4;d++){
  PipeNode n{4,3,4,(int8_t)face,(int8_t)u,(int8_t)v,0,0,0},next,back;
  steps=steps&&pipeStep(n,du[d],dv[d],next)&&fabsf(len(pipeCenter(next)-pipeCenter(n))-PipePitch)<.00001f;
  reciprocal=reciprocal&&pipeStep(next,du[d^1],dv[d^1],back)&&pipeSameCell(n,back);
 }
 check(steps&&reciprocal,"pipes cross host-cell seams continuously and reversibly on every face and slot");
 for(int face=0;face<6;face++)for(int kind=1;kind<=2;kind++){
  PipeNode n{4,3,4,(int8_t)face,1,1,(int8_t)kind,0,1},neighbor=n;neighbor.kind=0;neighbor.u=2;world.pipes={n,neighbor};settlePipes();
  check(world.pipes[0].rot==0,"automatic valve/gauge follows horizontal neighbors");
  world.pipes[1].u=1;world.pipes[1].v=2;settlePipes();check(world.pipes[0].rot==1,"automatic valve/gauge adapts when its neighbor changes axis");
  world.pipes[0].autoAlign=0;world.pipes[0].rot=2;settlePipes();check(world.pipes[0].rot==2,"manual pipe orientation survives automatic settling");
 }
 world.pipes={{4,3,4,4,1,1,0,0,0}};remember();world.pipes.push_back({4,3,4,4,2,1,2,1,1});undoEdit();check(world.pipes.size()==1,"undo restores pipe overlays");redoEdit();check(world.pipes.size()==2&&world.pipes[1].kind==2,"redo restores pipe kind and auto state");
 bool valid=true;for(int kind=0;kind<4;kind++)for(int r=0;r<4;r++)for(int d=0;d<4;d++){
  PipeNode n{4,3,4,4,1,1,(int8_t)kind,(int8_t)r,0};bool expected=kind==0||(kind==3?d==0:(r%2==0?d>=2:d<2));valid=valid&&pipeAllowed(n,d)==expected;
 }check(valid,"all pipe kinds and rotations expose the intended connection ports");
 world=original;history=oldHistory;redoStack=oldRedo;turn=oldTurn;origin=oldOrigin;ghostMode=oldGhost;vertices.clear();transparent.clear();dirty=true;
}

void selfTest(){starter();World initial=world;int count=0;for(auto c:world.c)if(c.t>=0)count++;check(count==129&&world.cables.empty()&&world.pipes.size()==5,"brutalist starter: 129 pieces, five pipes");save();world=World{};check(load(),"binary save reload");check(world.n==initial.n&&memcmp(world.c.data(),initial.c.data(),sizeof(Cell)*WorldCells)==0&&world.cables.size()==initial.cables.size()&&world.pipes.size()==initial.pipes.size(),"save roundtrip equality");remember();put(0,12,0,23,3);undo();check(memcmp(world.c.data(),initial.c.data(),sizeof(Cell)*WorldCells)==0,"undo restores cells");check(solidAt({2.5,.5,3.5}),"board-form floor collision");check(!solidAt({2.5,1.5,3.5}),"corridor airspace is open");put(1,0,2,-1);put(0,1,2,-1);put(1,1,2,-1);put(0,0,2,23);
 check(!solidAt({1.f,.4f,2.45f})&&solidAt({.12f,.4f,2.45f}),"doorway opening and jamb collision");
 check(!solidAt({1.f,.4f,2.45f})&&solidAt({1.88f,.4f,2.45f}),"doorway 2-wide opening and far jamb");
 check(solidAt({.12f,1.4f,2.45f})&&!solidAt({1.f,1.4f,2.45f}),"doorway upper jamb and opening");
 check(!solidAt({1.f,1.7f,2.45f})&&solidAt({1.f,1.9f,2.45f}),"doorway head clearance under lintel");
 check(freeAt({1.f,0,2.45f},1.6f),"doorway passable at standing height");
 check(solidAt({.12f,.4f,2.15f})&&solidAt({.12f,.4f,2.55f}),"doorway jamb has depth toward interior");
 check(!canPlace(0,1,2,0)&&!canPlace(1,0,2,0),"footprint blocks placing into doorway span");put(0,0,2,4);check(solidAt({.5,.2,2.2})&&!solidAt({.5,.9,2.8}),"slope collision");
 world=World{};put(0,0,0,56);put(0,1,0,-1);check(!solidAt({.5f,.5f,.5f})&&solidAt({.5f,1.8f,.5f}),"pilotis walk-under and capital");
 put(2,0,0,57);put(3,0,0,-1);check(solidAt({2.5f,.2f,.5f})&&solidAt({3.5f,.2f,.5f})&&!solidAt({2.5f,.7f,.5f}),"megastep 2-wide tread");
 put(2,0,2,58);for(int dx=0;dx<2;dx++)for(int dz=0;dz<2;dz++)for(int dy=0;dy<2;dy++)if(dx||dz||dy)put(2+dx,dy,2+dz,-1);
 check(solidAt({2.05f,.5f,2.5f})&&!solidAt({2.5f,.5f,2.5f}),"lightwell rim and open center");
 put(5,0,0,59);put(6,0,0,-1);put(5,1,0,-1);put(6,1,0,-1);
 check(solidAt({5.5f,.05f,.5f})&&!solidAt({5.5f,.8f,.5f})&&freeAt({5.5f,.12f,.5f},1.6f),"bridge tunnel fits standing player");
 put(8,0,0,60);put(8,1,0,-1);
 check(solidAt({8.1f,.5f,.5f})&&!solidAt({8.5f,.5f,.3f})&&freeAt({8.5f,0,.3f},1.6f),"tower core fits standing player");
 world=initial;for(int t=0;t<PieceCount;t++){vertices.clear();transparent.clear();origin={};turn=0;ghostMode=t==15;piece(t);ghostMode=false;check(vertices.size()+transparent.size()>0,(std::string("geometry: ")+names[t]).c_str());}dirty=true;}
void ivySelfTest(){
 World preserved=world;V oldOrigin=origin;int oldTurn=turn;
 for(int rot=0;rot<4;rot++){
  world=World{};turn=rot;V backing=transformNormal({0,0,1});for(int y=0;y<5;y++){put(3,y,3,15,rot);put(3-(int)backing.x,y,3-(int)backing.z,0);}
  vertices.clear();transparent.clear();origin={3.5f,4.5f,3.5f};turn=rot;ivyBeginRefresh();ivyGrowth();
  check(!vertices.empty(),"connected ivy emits geometry from upper edge");
  size_t count=vertices.size();float low=100,high=-100;
  uint64_t signature=1469598103934665603ull;
  bool finite=true;for(auto v:vertices){finite=finite&&std::isfinite(v.p.x)&&std::isfinite(v.p.y)&&std::isfinite(v.p.z);low=std::min(low,v.p.y);high=std::max(high,v.p.y);uint32_t bits;memcpy(&bits,&v.p.x,4);signature=(signature^bits)*1099511628211ull;}
  check(finite,"finite ivy vertices");check(high-low>3.4f,"vine stems continue across multiple cells");
  vertices.clear();ivyBeginRefresh();ivyGrowth();uint64_t again=1469598103934665603ull;for(auto v:vertices){uint32_t bits;memcpy(&bits,&v.p.x,4);again=(again^bits)*1099511628211ull;}
  check(vertices.size()==count&&signature==again,"ivy geometry is stable across refreshes");
  vertices.clear();transparent.clear();origin={3.5f,2.5f,3.5f};ivyGrowth();check(vertices.empty()&&transparent.empty(),"middle cells do not restart horizontal ivy bands");
 }
 // Adjacent columns must be emitted as one component, with shared leaf spacing.
 world=World{};for(int x=3;x<=5;x++)for(int y=0;y<5;y++){put(x,y,3,15,0);put(x,y,2,0);}
 origin={3.5f,4.5f,3.5f};turn=0;vertices.clear();transparent.clear();ivyBeginRefresh();ivyGrowth();
 check(ivyGenerated[idx(5,0,3)],"ivy flood joins horizontal and vertical neighbors");
 check(ivyLeavesAccepted>0&&ivyLeavesSkipped>0,"neighbor ivy shares a leaf spacing budget");
 size_t before=vertices.size();origin={4.5f,3.5f,3.5f};ivyGrowth();check(vertices.size()==before,"neighbor cells do not duplicate patch geometry");
 bool sitesFinite=true;for(const auto& bin:ivyLeafBins)for(V site:bin)sitesFinite=sitesFinite&&std::isfinite(site.x)&&std::isfinite(site.y)&&std::isfinite(site.z);check(sitesFinite,"finite adaptive leaf sites");
 world=World{};put(3,2,3,0);
 for(int rot=0;rot<4;rot++){
  turn=rot;V n=transformNormal({0,0,1});origin=V{3.5f,2.5f,3.5f}+n*2.f;
  V attached=transform(ivyAttached({0,0,-.4f}));float outward=dot(attached-V{3.5f,2.5f,3.5f},n);
  check(outward>.5f&&outward<.67f,"nearby ivy attaches to supporting wall in all rotations");
 }
 ivyBeginRefresh();check(ivyReserveLeaf({3,2,3},.1f),"first nearby leaf occupies its spacing site");check(!ivyReserveLeaf({3.025f,2,3},.1f),"nearby overlapping vine leaf is rejected");
 world=preserved;origin=oldOrigin;turn=oldTurn;vertices.clear();transparent.clear();ivyBeginRefresh();dirty=true;
}
void foliageTraceSelfTest(){
 check(foliageRecords.size()*sizeof(F4)<=foliageCapacity,"water foliage acceleration data fits its GPU allocation");
 bool valid=true;size_t coverage=0;
 for(size_t i=0;i<foliageNodes.size();i++){auto node=foliageNodes[i];valid=valid&&node.escape>i&&node.escape<=foliageNodes.size();int count=node.packed&7;size_t first=node.packed>>3;
  if(!count){valid=valid&&i+1<foliageNodes.size();continue;}coverage+=count;valid=valid&&count<=4&&first+count<=foliageTriangles.size();
  for(size_t j=first;j<first+count&&j<foliageTriangles.size();j++)for(auto v:foliageTriangles[j].v){valid=valid&&v.p.x-.014f>=node.lo.x&&v.p.x+.014f<=node.hi.x&&v.p.y>=node.lo.y&&v.p.y<=node.hi.y&&v.p.z-.0056f>=node.lo.z&&v.p.z+.0056f<=node.hi.z;}
 }
 check(valid&&coverage==foliageTriangles.size()&&coverage>0,"water BVH covers every plant triangle with valid escape links and wind bounds");
 bool leaves=false,bark=false;for(auto t:foliageTriangles){leaves=leaves||t.v[0].m==22;bark=bark||t.v[0].m==23;}
 check(leaves&&bark,"water rays receive both leaf cutouts and actual branch meshes");
 check(foliageRecords.size()==1+foliageNodes.size()*2+foliageTriangles.size()*5+350,"water foliage records match the shader storage layout");
}

void catalogSelfTest(){
 World previous=world;V previousOrigin=origin;int previousTurn=turn;
 // Check emitter positions independently of camera and world refresh.
 origin={2.5f,1.5f,4.5f};
 for(turn=0;turn<4;turn++){
  V expected[4]={{0,0,-.4f},{-.4f,0,0},{0,0,.4f},{.4f,0,0}};
  check(len(lampEmitter(27)-(origin+expected[turn]))<.00001f,"sconce light follows fixture rotation");
  check(fabsf(lampEmitter(13).y-1.9f)<.00001f,"strip light uses luminous bar height");
  check(fabsf(lampEmitter(26).y-1.67f)<.00001f,"bollard light uses luminous cap height");
  check(fabsf(lampEmitter(37).y-1.42f)<.00001f,"pendant light uses bulb height");
 }
 bool available[PieceCount]{};
 for(int c=0;c<CategoryCount;c++)for(int j=0;j<CategorySlots&&catTypes[c][j]>=0;j++)available[catTypes[c][j]]=true;
 for(int t=0;t<PieceCount;t++){
  check(available[t],(std::string("palette includes ")+names[t]).c_str());
  for(int rot=0;rot<4;rot++){int v=1+t+PieceEncodingStride*rot;check((v-1)%PieceEncodingStride==t&&(v-1)/PieceEncodingStride==rot,"GPU type and rotation encoding");}
 }
 world=World{};world.n=16;
 for(int t=0;t<PieceCount;t++)put(t%16,t/16,0,t,t%4);
 world.pipes={{2,3,0,4,2,0,2,1,1},{3,1,0,1,0,2,1,3,0}};World all=world;std::string nativeExport=exportWorldJson();check(loadWorldJson(nativeExport.c_str()),"native Share JSON reimports");check(memcmp(all.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"native Share retains all new piece IDs");check(world.pipes.size()==all.pipes.size()&&memcmp(world.pipes.data(),all.pipes.data(),all.pipes.size()*sizeof(PipeNode))==0,"HTML-compatible Share JSON preserves all pipe fields");save();world=World{};check(load(),"41-piece binary world loads");
 check(memcmp(all.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"all 41 types and rotations survive binary saves");
 std::string json="{\"N\":16,\"c\":[";
 for(int t=0;t<PieceCount;t++){if(t)json+=',';int x=t%16,y=t/16,k=x*1024+y;json+="["+std::to_string(k)+",{\"t\":"+std::to_string(t)+",\"r\":"+std::to_string(t%4)+"}]";}
 json+="],\"cab\":[]}";check(loadWorldJson(json.c_str()),"latest HTML world imports");
 check(memcmp(all.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"HTML import retains pieces above ID 27");
 world=World{};
 put(2,0,2,28);check(solidAt({2.5f,.2f,2.5f})&&!solidAt({2.95f,.4f,2.95f}),"dome hemisphere collision");
 put(2,0,2,29);check(solidAt({2.5f,.9f,2.5f})&&!solidAt({2.95f,.5f,2.95f}),"silo cylindrical collision");
 put(2,0,2,30);check(solidAt({2.5f,.9f,2.5f})&&!solidAt({2.98f,.9f,2.5f}),"vault curved collision");
 put(2,0,2,34);check(!solidAt({2.5f,.5f,2.5f})&&solidAt({2.5f,.95f,2.5f}),"floor underside clearance");
 put(2,0,2,36);check(solidAt({2.5f,.7f,2.2f})&&!solidAt({2.5f,.7f,2.8f}),"eight-step stair profile");
 put(2,0,2,37);check(!solidAt({2.5f,.5f,2.5f}),"pendant is nonblocking");
 put(2,0,2,39);check(solidAt({2.5f,.32f,2.5f})&&!solidAt({2.5f,.7f,2.5f}),"bench low profile collision");
 put(2,0,2,40);check(!solidAt({2.5f,.5f,2.5f}),"water remains traversable");
 put(2,0,2,63);check(solidAt({2.5f,.5f,2.5f})&&!solidAt({2.95f,.5f,2.95f}),"stone rock collision");
 put(2,0,2,64);check(solidAt({2.5f,.5f,2.5f})&&!solidAt({2.98f,.9f,2.98f}),"boulder rock collision");
 put(2,0,2,65);check(solidAt({2.5f,.2f,2.5f})&&!solidAt({2.5f,.7f,2.5f}),"log low profile collision");
 put(2,0,2,66);check(solidAt({2.5f,.4f,2.5f})&&!solidAt({2.95f,.4f,2.95f}),"stump trunk collision");
 put(2,0,2,67);check(!solidAt({2.5f,.5f,2.5f}),"mushroom is nonblocking");
 world=World{};put(3,2,3,0);origin={3.5f,2.5f,4.5f};turn=0;
 ivyBeginRefresh();vertices.clear();transparent.clear();IvyRandom leafRng{137u};ivyLeaf({0,0,-.4f},{.7f,-.4f,.3f},.12f,leafRng);
 check(vertices.size()==24,"ivy leaf uses eight folded faces without overlapping flat triangles");
 bool leafValid=true;for(size_t i=0;i+2<vertices.size();i+=3)leafValid=leafValid&&len(cross(vertices[i+1].p-vertices[i].p,vertices[i+2].p-vertices[i].p))>1e-7f;
 check(leafValid,"ivy leaf triangles have nonzero surface area");
 vertices.clear();transparent.clear();ivyBeginRefresh();
 world=previous;origin=previousOrigin;turn=previousTurn;history.clear();dirty=true;
}

void uiSelfTest(){

 for(int type=0;type<PieceCount;type++){int count=0;auto f=pieceFamily(type,count);if(f){bool found=false;for(int i=0;i<count;i++)if(f[i]==type)found=true;check(found,"Every adaptive family includes its selected variant");}}
 {int n=0;auto f=pieceFamily(95,n);check(f&&n==7,"Angle participates in the seven-light cycle");}
 bool oldAtmosphere=atmosphereExpanded,oldWorld=worldExpanded;int oldModal=uiModal;bool oldAudio=audioExpanded,oldSettings=settingsExpanded;int oldScheme=camScheme;
 auto has=[](int id){for(auto a:actions)if(a.action==id)return true;return false;};
 atmosphereExpanded=true;worldExpanded=false;settingsExpanded=false;uiModal=0;rebuildActions();
 check(has(24)&&has(30)&&has(26)&&has(27)&&has(35),"Controls and collapsible section headers are available");
 check(has(300)&&has(309)&&has(42)&&!has(7),"Atmosphere starts expanded with clouds/wind and World starts collapsed");
 for(int c=0;c<CategoryCount;c++)check(has(100+c),"all six HTML category tabs are clickable");
 worldExpanded=true;rebuildActions();check(has(7)&&has(19)&&has(20)&&has(9)&&has(40)&&has(43),"World expands to plot, Share, Hide UI, Export OBJ/FBX and Clear");
 check(has(37),"Redo control is available beside Undo");
 atmosphereExpanded=false;rebuildActions();check(!has(300)&&has(27),"collapsed Atmosphere hides its sliders");
 audioExpanded=true;rebuildActions();check(has(304)&&has(305)&&has(306)&&has(29),"Audio exposes independent music, ambience, effects and mute controls");
 settingsExpanded=true;rebuildActions();check(has(36)&&has(307)&&has(308)&&has(41)&&has(38)&&has(39),"Settings exposes camera, sens, FOV, resolution, fullscreen and photos folder");
 camScheme=1;rebuildActions();check(has(36),"Modifiers camera scheme stays selectable");
 uiModal=3;rebuildActions();check(has(31)&&has(32)&&has(33)&&has(34),"Saves area exposes save, load and paging controls");
 int oldCategory=category;category=5;uiModal=0;dockScrollX=0;rebuildActions();check(has(600)&&has(55)&&has(56),"Material tab exposes surface swatches, texture rotation and scale");category=oldCategory;
 uiModal=5;rebuildActions();check(has(52)&&has(53)&&has(54)&&has(22),"Viewpoints exposes save, paging and close controls");
 uiModal=1;rebuildActions();check(has(22),"Controls dialog provides Close");
 uiModal=2;rebuildActions();check(has(22)&&has(23),"Share dialog provides Load and Close");
 audioExpanded=oldAudio;atmosphereExpanded=oldAtmosphere;worldExpanded=oldWorld;settingsExpanded=oldSettings;camScheme=oldScheme;uiModal=oldModal;rebuildActions();
}

// Independent sampled mesh/physics audit plus adaptation contracts. Output records
// approximations too; it does not silently turn a review finding into a passing test.
bool auditRayTriangle(V p,V direction,V a,V b,V c,float& distance){
 V e=b-a,f=c-a,P=cross(direction,f);float det=dot(e,P);if(fabsf(det)<1e-8f)return false;V T=p-a;float u=dot(T,P)/det;if(u<0||u>1)return false;V Q=cross(T,e);float v=dot(direction,Q)/det;if(v<0||u+v>1)return false;distance=dot(f,Q)/det;return distance>1e-5f;
}
bool auditMeshContains(V p,const std::vector<Vertex>& mesh,bool& boundary){
 V direction=norm({.731f,.413f,.541f});int winding=0;boundary=false;
 for(size_t i=0;i+2<mesh.size();i+=3){V a=mesh[i].p,b=mesh[i+1].p,c=mesh[i+2].p,N=cross(b-a,c-a);float size=len(N);if(size<1e-7f)continue;float t;
  if(auditRayTriangle(p,direction,a,b,c,t)){winding+=dot(N,direction)>0?1:-1;if(t<.035f)boundary=true;}
  if(fabsf(dot(p-a,N))/size<.035f){V q=p-N*(dot(p-a,N)/(size*size));V u=b-a,v=c-a,w=q-a;float uu=dot(u,u),uv=dot(u,v),vv=dot(v,v),wu=dot(w,u),wv=dot(w,v),den=uu*vv-uv*uv;
   if(fabsf(den)>1e-10f){float s=(wu*vv-wv*uv)/den,t0=(wv*uu-wu*uv)/den;if(s>=-.01f&&t0>=-.01f&&s+t0<=1.01f)boundary=true;}
  }
 }
 return winding!=0;
}
void catalogContractTests(const std::wstring& directory){
 World original=world;V previousOrigin=origin;int previousTurn=turn;bool previousBake=bakeNatureMask;bakeNatureMask=false;
 FILE* f=nullptr;_wfopen_s(&f,(directory+L"\\contracts.csv").c_str(),L"wb");if(!f)fail("Could not write catalog contracts");fprintf(f,"type,rotation,samples,mesh_physics_mismatches,footprint_misses,nonfinite\n");
 unsigned checks=0;
 {std::unordered_set<int> keys;for(int x=-2;x<66;x++)for(int z=-2;z<66;z++)for(int y=-2;y<22;y++){
  int key=netPack(x,y,z),a,b,c;netUnpack(key,a,b,c);if(a!=x||b!=y||c!=z||!keys.insert(key).second)fail("64-tile spatial key collision");
 }log("PASS: 110976 spatial keys including negative halo are unique and reversible");}


 for(int r=0;r<4;r++){
  for(int type:{23,51,56,59,60}){
   world=World{};world.n=64;put(8,0,8,type,r);origin={8.5f,.5f,8.5f};turn=r;
   V feet=transform({type==23||type==51?.5f:0.f,-.5f+(type==59?.12f:0.f),type==23||type==51?-.05f:type==60?-.2f:0.f});
   if(!freeAt(feet,1.6f)||!freeAt(feet,1.f))fail("Player clearance regressed in a walk-through part");
  }
  world=World{};world.n=64;origin={8.5f,2.5f,8.5f};turn=r;put(8,2,8,6,r);
  V side=transform({-1,0,0});int x=(int)floorf(side.x),z=(int)floorf(side.z);
  for(int type:{15,40,76,77,82,86,88}){put(x,2,z,type,r);if(stairSideBlocked(-1)){char msg[100];sprintf_s(msg,"Decoration %d removed a stair rail at rotation %d",type,r);fail(msg);}}
  put(x,2,z,0);if(!stairSideBlocked(-1))fail("Solid adjoining wall did not suppress the shared stair rail");
  put(x,2,z,6,r);if(!stairSideBlocked(-1))fail("Aligned stair flights kept an internal shared rail");
 }
 log("PASS: 1.6 m standing and crouching passage clearance, decoration/wall/stair rail contracts in all four rotations");
 for(int type=0;type<PieceCount;type++)for(int rotation=0;rotation<4;rotation++){
  world=World{};world.n=64;put(8,2,8,type,rotation);origin={8.5f,2.5f,8.5f};turn=rotation;ambient=1;ivyBeginRefresh();vertices.clear();transparent.clear();piece(type);vertices.insert(vertices.end(),transparent.begin(),transparent.end());std::vector<Vertex> mesh;
  bool physics=type!=14&&type!=41&&type!=42&&type!=63&&type!=64&&type!=65&&type!=66;
  for(size_t i=0;i+2<vertices.size();i+=3){int m=(int)vertices[i].m;if(m==9||m>=20||m==6||m==16||m==17||m==19)continue;mesh.insert(mesh.end(),vertices.begin()+i,vertices.begin()+i+3);}
  int samples=0,mismatch=0,misses=0,bad=0;
 for(int lx=0;lx<footprintW(type);lx++)for(int lz=0;lz<footprintD(type);lz++)for(int ly=0;ly<footprintH(type);ly++){
  V center=transform({(float)lx,(float)ly,(float)lz});int a,b,c;
  if(!footprintRoot((int)floorf(center.x),(int)floorf(center.y),(int)floorf(center.z),a,b,c)||a!=8||b!=2||c!=8)fail("Rendered footprint and reserved cells differ");
 }for(auto v:vertices)if(!std::isfinite(v.p.x)||!std::isfinite(v.n.y))bad++;
  eachFootprintCell(8,2,8,type,rotation,[&](int x,int y,int z){int a,b,c;if(!footprintRoot(x,y,z,a,b,c)||a!=8||b!=2||c!=8)misses++;});
  if(physics&&!mesh.empty())for(int x=0;x<footprintW(type)*6;x++)for(int y=0;y<footprintH(type)*6;y++)for(int z=0;z<footprintD(type)*6;z++){
   V local{(x+.5f)/6-.5f,(y+.5f)/6-.5f,(z+.5f)/6-.5f},p=transform(local);bool boundary=false,visual=auditMeshContains(p,mesh,boundary);if(boundary)continue;
   bool physical=solidAt(p);samples++;if(visual!=physical)mismatch++;
  }
  fprintf(f,"%d,%d,%d,%d,%d,%d\n",type,rotation,samples,mismatch,misses,bad);checks++;if(bad||misses)log("Catalog contract finding: invalid mesh or footprint");
 }
 fclose(f);
 FILE* interactions=nullptr;_wfopen_s(&interactions,(directory+L"\\interactions.csv").c_str(),L"wb");if(!interactions)fail("Could not write interaction matrix");fprintf(interactions,"adaptive,neighbor,rotation,neighbor_rotation,direction,vertices,nonfinite\n");
 const int adaptive[]={6,36,78,22,83,40,77,82,87,88,89,90,91,92,5,15,17,24,63,64};const V directions[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
 for(int type:adaptive)for(int neighbor=0;neighbor<PieceCount;neighbor++)for(int r=0;r<4;r++)for(int nr=0;nr<4;nr++){
  for(int d=0;d<6;d++){
   world=World{};world.n=12;put(8,3,8,type,r);V q=V{8,3,8}+directions[d];put((int)q.x,(int)q.y,(int)q.z,neighbor,nr);origin={8.5f,3.5f,8.5f};turn=r;ivyBeginRefresh();vertices.clear();transparent.clear();
   if(type==5){origin={};turn=0;emitMossSurfaceNets();}
   else if(type==63||type==64){origin={};turn=0;emitRockSurfaceNets();}
   else if(type==17||type==24){origin={};turn=0;emitMergedBasins();}
   else piece(type);int bad=0;
   for(auto v:vertices)if(!std::isfinite(v.p.x)||!std::isfinite(v.p.y)||!std::isfinite(v.n.z))bad++;for(auto v:transparent)if(!std::isfinite(v.p.x))bad++;
   fprintf(interactions,"%d,%d,%d,%d,%d,%zu,%d\n",type,neighbor,r,nr,d,vertices.size()+transparent.size(),bad);
  }
 }
 fclose(interactions);char summary[160];sprintf_s(summary,"Catalog CPU matrix: %u part/rotation cases and %u directed neighbor cases",checks,(unsigned)(sizeof(adaptive)/sizeof(adaptive[0]))*PieceCount*4*4*6);log(summary);
 world=original;origin=previousOrigin;turn=previousTurn;bakeNatureMask=previousBake;vertices.clear();transparent.clear();ivyBeginRefresh();dirty=true;
}

// Repeatable native Vulkan catalog previews. Never writes the player's world or settings.
struct CatalogAuditCase{int type,rotation,variant=0,face=4,ports=0,kind=0;};
std::vector<CatalogAuditCase> catalogAuditCases;size_t catalogAuditIndex=0;std::wstring catalogAuditDirectory;FILE* catalogAuditLog=nullptr;
void catalogAuditSetup(){
 auto c=catalogAuditCases[catalogAuditIndex];world=World{};world.n=64;world.cables.clear();world.pipes.clear();int x=6,z=6,mountY=c.variant==3&&c.face==3?6:1;
 if(c.variant==1){bool found=false;for(int a=3;a<40&&!found;a++)for(int b=3;b<40&&!found;b++){origin={a+.5f,1.5f,b+.5f};turn=0;if(natureTreeVariant(natureSeed())==c.ports){x=a;z=b;found=true;}}}
 for(int a=x-2;a<=x+2;a++)for(int b=z-2;b<=z+2;b++)put(a,0,b,47);
 auditPlantModel=c.variant==2?c.ports:-1;origin={x+.5f,1.5f,z+.5f};turn=c.rotation;ambient=1;ghostMode=false;ivyBeginRefresh();
 if(c.variant==3||isPipeTool(c.type)){
  put(x,mountY,z,0);PipeNode center{};center.cx=x;center.cy=mountY;center.cz=z;center.face=(int8_t)c.face;center.u=center.v=1;center.kind=(int8_t)(c.variant==3?c.kind:c.type-43);center.rot=(int8_t)c.rotation;world.pipes.push_back(center);
  for(int d=0;d<4;d++)if(c.ports&(1<<d)){PipeNode next=center;next.kind=0;next.u+=(d==2)-(d==3);next.v+=(d==0)-(d==1);world.pipes.push_back(next);}
 }else if(auditPlantModel<0){
  put(x,1,z,c.type,c.rotation);
  if(c.type==15||c.type==77||c.type==82){V d=transformNormal({0,0,-1});put(x+(int)d.x,1,z+(int)d.z,47);}
  if(c.type==87||c.type==90||c.type==92)put(x,2,z,34);
  if(c.type==88||c.type==89){put(x,0,z,4,c.rotation);}
  if(c.type==91)put(x,0,z,1);
  if(c.type==16){world.cables.push_back({{x-.4f,2,z+.5f},{x+1.4f,2.4f,z+.5f}});}
 }
 vertices.clear();transparent.clear();grass.clear();origin={x+.5f,1.5f,z+.5f};turn=c.rotation;
 if(auditPlantModel>=0)natureVariantMesh(auditPlantModel);else piece(c.type);
 vertices.insert(vertices.end(),transparent.begin(),transparent.end());V lo{1e6f,1e6f,1e6f},hi{-1e6f,-1e6f,-1e6f};size_t bad=0,degenerate=0;
 for(auto v:vertices){lo=foliageMin(lo,v.p);hi=foliageMax(hi,v.p);if(!std::isfinite(v.p.x)||!std::isfinite(v.p.y)||!std::isfinite(v.p.z)||!std::isfinite(v.n.x)||!std::isfinite(v.u)||!std::isfinite(v.v))bad++;}
 for(size_t i=0;i+2<vertices.size();i+=3)if(len(cross(vertices[i+1].p-vertices[i].p,vertices[i+2].p-vertices[i].p))<.000001f)degenerate++;
 if(vertices.empty()){lo={x-.5f,1,z-.5f};hi={x+1.5f,2,z+1.5f};}
 target=(lo+hi)*.5f;distanceCam=std::clamp(len(hi-lo)*1.6f,4.f,18.f);theta=.65f;phi=1.05f;buildFov=45;walk=false;night=false;rain=false;wind=false;clouds=0;mood=0;sunHeight=45;sunAngle=135;exposure=.95f;fog=.02f;moodSat=.1f;
 if(c.variant==3){target={x+.5f,mountY+.5f,z+.5f};V n=PipeN[c.face];theta=atan2f(n.x,n.z);phi=acosf(n.y*.9f);if(c.face==2||c.face==3)phi=c.face==2?.3f:2.8f;distanceCam=4;}
 wchar_t filename[80];swprintf_s(filename,L"\\%04zu-t%02d-r%d-v%d.bmp",catalogAuditIndex,c.type,c.rotation,c.variant);captureFile=catalogAuditDirectory+filename;capturePending=true;captureAtFrame=frames+1;hideUI=true;dirty=true;layout();
 if(catalogAuditLog){fprintf(catalogAuditLog,"{\"case\":%zu,\"type\":%d,\"rotation\":%d,\"variant\":%d,\"selection\":%d,\"face\":%d,\"ports\":%d,\"vertices\":%zu,\"nonfinite\":%zu,\"degenerate\":%zu}\n",catalogAuditIndex,c.type,c.rotation,c.variant,c.ports,c.face,c.ports,vertices.size(),bad,degenerate);fflush(catalogAuditLog);}
}
void catalogAuditStart(const std::wstring& args){
 wchar_t module[MAX_PATH];GetModuleFileNameW(nullptr,module,MAX_PATH);catalogAuditDirectory=module;catalogAuditDirectory.resize(catalogAuditDirectory.find_last_of(L"\\"));catalogAuditDirectory.resize(catalogAuditDirectory.find_last_of(L"\\"));catalogAuditDirectory+=L"\\output\\catalog-audit";
 std::wstring requested=argumentValue(args,L"--audit-dir=");if(!requested.empty())catalogAuditDirectory=requested;
 CreateDirectoryW(catalogAuditDirectory.c_str(),nullptr);if(args.find(L"--catalog-contracts")!=std::wstring::npos)catalogContractTests(catalogAuditDirectory);_wfopen_s(&catalogAuditLog,(catalogAuditDirectory+L"\\cases.jsonl").c_str(),L"wb");if(!catalogAuditLog)fail("Could not open catalog audit output");
 for(int t=0;t<PieceCount;t++)for(int r=0;r<4;r++)catalogAuditCases.push_back({t,r});
 for(int i=0;i<8;i++)catalogAuditCases.push_back({14,0,1,4,i});
 for(int i=0;i<21;i++)catalogAuditCases.push_back({86,0,2,4,i});
 for(int face=0;face<6;face++)for(int ports=0;ports<16;ports++)catalogAuditCases.push_back({43,0,3,face,ports,0});
 for(int face=0;face<6;face++)for(int kind=1;kind<4;kind++)for(int r=0;r<4;r++)catalogAuditCases.push_back({43+kind,r,3,face,15,kind});
 fixedFrame=true;catalogAuditSetup();
}
bool catalogAuditAdvance(){if(capturePending)return true;catalogAuditIndex++;if(catalogAuditIndex>=catalogAuditCases.size()){if(catalogAuditLog){fclose(catalogAuditLog);catalogAuditLog=nullptr;}log("Catalog Vulkan previews completed");return false;}catalogAuditSetup();return true;}

