#include <io.h>
// Named world snapshots live beside the existing autosave, never inside the executable.
struct SaveEntry {std::wstring path,name;int pieces=0;FILETIME modified{};std::vector<uint32_t> thumbnail;};
std::wstring savesDirectory;std::vector<SaveEntry> saveEntries;int saveSelected=-1,savePage=0;std::string savesStatus;
void writeSaveThumbnail(const std::wstring& path);
void buildSaveThumbnail(SaveEntry& entry,const World& w);
HWND saveNameEdit=nullptr;WNDPROC saveNameOldProc=nullptr;
SavedView savedView(){uint32_t cloudBits=(uint32_t)std::clamp((int)(clouds*100.f+.5f),0,100);return {target,player,theta,phi,distanceCam,yaw,pitch,walk?fieldOfView:buildFov,sunHeight,sunAngle,exposure,fog,moodSat,mood,(night?1u:0u)|(rain?2u:0u)|(ao?4u:0u)|(walk?8u:0u)|(wind?16u:0u)|(cloudBits<<8)};}
bool validSavedView(const SavedView& v){
 float values[]={v.target.x,v.target.y,v.target.z,v.player.x,v.player.y,v.player.z,v.theta,v.phi,v.distance,v.yaw,v.pitch,v.fov,v.sunHeight,v.sunAngle,v.exposure,v.fog,v.sat};
 for(float x:values)if(!std::isfinite(x)||fabsf(x)>10000)return false;
 return v.mood>=0&&v.mood<5&&v.distance>=4&&v.distance<=70&&v.fov>=40&&v.fov<=110&&v.phi>=.1f&&v.phi<=1.6f&&v.exposure>=.5f&&v.exposure<=1.8f&&v.fog>=0&&v.fog<=1&&(v.flags&~0xFF1Fu)==0;
}
bool readNamedSave(const std::wstring& path,World& w,SavedView& view,std::wstring& name){
 FILE* f=nullptr;_wfopen_s(&f,path.c_str(),L"rb");if(!f)return false;uint32_t magic=0,count=0;wchar_t title[64]{};
 int grid=MaxPlot;bool ok=fread(&magic,4,1,f)==1;if(magic==0x31534e4d)grid=16;else if(magic==0x32534e4d)grid=32;else if(magic!=0x33534e4d&&magic!=0x34534e4d)ok=false; // MNS1=16, MNS2=32, MNS3=64
 ok=ok&&fread(title,sizeof(title),1,f)==1&&title[63]==0&&fread(&view,sizeof(view),1,f)==1&&validSavedView(view)&&fread(&w.n,4,1,f)==1&&w.n>=4&&w.n<=grid&&w.n<=MaxPlot&&readWorldCells(f,w,grid)&&fread(&count,4,1,f)==1&&count<=2048;
 if(ok){w.cables.resize(count);ok=!count||fread(w.cables.data(),sizeof(Cable),count,f)==count;}
 if(ok&&magic==0x34534e4d){uint32_t pc=0;ok=fread(&pc,4,1,f)==1&&pc<=8000;if(ok){w.pipes.resize(pc);ok=!pc||fread(w.pipes.data(),sizeof(PipeNode),pc,f)==pc;}}
 else w.pipes.clear();
 if(ok)for(auto p:w.pipes)if(!validPipeNode(p,w.n))ok=false;
 if(ok)ok=readSurfaceChunk(f,w);fclose(f);if(!ok)return false;
 for(auto c:w.c)if(c.t< -1||c.t>=PieceCount||c.r<0||c.r>3)return false;
 for(auto c:w.cables)if(!std::isfinite(len(c.a))||!std::isfinite(len(c.b))||len(c.a)>100||len(c.b)>100)return false;
 name=title;return !name.empty();
}
bool writeNamedSave(const std::wstring& path,const std::wstring& name){
 std::wstring temporary=path+L".tmp";FILE* f=nullptr;_wfopen_s(&f,temporary.c_str(),L"wb");if(!f)return false;
 uint32_t magic=0x34534e4d,count=(uint32_t)world.cables.size();wchar_t title[64]{};wcsncpy_s(title,name.c_str(),63);SavedView view=savedView(); // MNS4 = 64³ dense + pipes
 bool ok=fwrite(&magic,4,1,f)==1&&fwrite(title,sizeof(title),1,f)==1&&fwrite(&view,sizeof(view),1,f)==1&&fwrite(&world.n,4,1,f)==1&&fwrite(world.c.data(),sizeof(Cell),WorldCells,f)==WorldCells&&fwrite(&count,4,1,f)==1&&(!count||fwrite(world.cables.data(),sizeof(Cable),count,f)==count);
 uint32_t pc=(uint32_t)world.pipes.size();ok=ok&&fwrite(&pc,4,1,f)==1&&(!pc||fwrite(world.pipes.data(),sizeof(PipeNode),pc,f)==pc);
 ok=ok&&writeSurfaceChunk(f);ok=fflush(f)==0&&ok;if(ok)ok=_commit(_fileno(f))==0;ok=fclose(f)==0&&ok;
 if(ok)ok=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
 if(!ok)DeleteFileW(temporary.c_str());return ok;
}
void refreshNamedSaves(){
 saveEntries.clear();WIN32_FIND_DATAW d{};HANDLE find=FindFirstFileW((savesDirectory+L"\\*.mns").c_str(),&d);
 if(find!=INVALID_HANDLE_VALUE){do{if(d.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;SaveEntry e;e.path=savesDirectory+L"\\"+d.cFileName;e.modified=d.ftLastWriteTime;World w;SavedView v{};if(!readNamedSave(e.path,w,v,e.name))continue;for(auto c:w.c)if(c.t>=0)e.pieces++;buildSaveThumbnail(e,w);saveEntries.push_back(std::move(e));}while(FindNextFileW(find,&d));FindClose(find);}
 std::sort(saveEntries.begin(),saveEntries.end(),[](const SaveEntry& a,const SaveEntry& b){return CompareFileTime(&a.modified,&b.modified)>0;});
 savePage=std::clamp(savePage,0,std::max(0,((int)saveEntries.size()-1)/5));saveSelected=saveEntries.empty()?-1:std::clamp(saveSelected,0,(int)saveEntries.size()-1);
}
std::wstring uniqueSavePath(){static unsigned serial=0;FILETIME ft;GetSystemTimeAsFileTime(&ft);wchar_t name[80];swprintf_s(name,L"\\world-%08lx%08lx-%u.mns",ft.dwHighDateTime,ft.dwLowDateTime,serial++);return savesDirectory+name;}
bool saveCurrentNamed(const std::wstring& supplied){
 auto begin=supplied.find_first_not_of(L" \t\r\n"),end=supplied.find_last_not_of(L" \t\r\n");std::wstring name=begin==std::wstring::npos?L"Untitled world":supplied.substr(begin,end-begin+1);name.resize(std::min((size_t)63,name.size()));
 std::wstring path=uniqueSavePath();if(!writeNamedSave(path,name)){savesStatus="Could not save. Your current world is still open.";return false;}
 writeSaveThumbnail(path);refreshNamedSaves();for(size_t i=0;i<saveEntries.size();i++)if(saveEntries[i].path==path){saveSelected=(int)i;savePage=saveSelected/5;}
 savesStatus="Saved a new copy.";return true;
}
bool loadSelectedNamed(){
 if(saveSelected<0||saveSelected>=(int)saveEntries.size()){savesStatus="Choose a saved world first.";return false;}
 World next;SavedView v{};std::wstring name;
 if(!readNamedSave(saveEntries[saveSelected].path,next,v,name)){savesStatus="Could not read this save. Your world has not changed.";return false;}
 if(!writeNamedSave(savesDirectory+L"\\recovery.mns",L"Before last load")){savesStatus="Could not back up the current world; load cancelled.";return false;}
 remember();world=std::move(next);resetDoorMotion();target=v.target;player=v.player;namedWalkSpawn=v.player;hasNamedWalkSpawn=true;theta=v.theta;phi=v.phi;distanceCam=v.distance;yaw=v.yaw;pitch=v.pitch;if(v.flags&8)fieldOfView=v.fov;else buildFov=std::clamp(v.fov,40.f,100.f);sunHeight=v.sunHeight;sunAngle=v.sunAngle;exposure=v.exposure;fog=v.fog;moodSat=v.sat;mood=v.mood;night=v.flags&1;rain=v.flags&2;ao=v.flags&4;walk=v.flags&8;wind=v.flags&16;clouds=std::clamp(float((v.flags>>8)&255)/100.f,0.f,1.f);velY=hx=hz=wallRun=camRoll=0;grounded=false;spaceHeld=false;pendingCable=drag=false;button=-1;std::fill(std::begin(keys),std::end(keys),false);dirty=true;save();savesStatus="Loaded. Your previous world is in Before last load.";return true;
}

int viewpointPage=0;
void restoreViewpoint(const SavedView& v){
 target=v.target;player=v.player;theta=v.theta;phi=v.phi;distanceCam=v.distance;yaw=v.yaw;pitch=v.pitch;
 walk=(v.flags&8)!=0;if(walk){fieldOfView=v.fov;namedWalkSpawn=v.player;hasNamedWalkSpawn=true;}else buildFov=v.fov;
 sunHeight=v.sunHeight;sunAngle=v.sunAngle;exposure=v.exposure;fog=v.fog;moodSat=v.sat;mood=v.mood;
 night=v.flags&1;rain=v.flags&2;ao=v.flags&4;wind=v.flags&16;clouds=std::clamp(float((v.flags>>8)&255)/100.f,0.f,1.f);
 velY=hx=hz=wallRun=camRoll=0;grounded=false;spaceHeld=false;drag=pendingCable=false;button=-1;std::fill(std::begin(keys),std::end(keys),false);
}
