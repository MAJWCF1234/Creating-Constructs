void savesSelfTest(){
 World original=world;SavedView originalView=savedView();std::wstring directory=savesDirectory;
 wchar_t child[80];swprintf_s(child,L"\\run-%llu",GetTickCount64());savesDirectory+=child;CreateDirectoryW(savesDirectory.c_str(),nullptr);
 world=World{};for(int t=0;t<PieceCount;t++)put(t%10,t/10,3,t,t%4);world.cables={{{1,2,3},{5,6,7}}};mood=3;night=true;rain=true;target={2,3,4};theta=1.3f;
 check(saveCurrentNamed(L"  Quiet courtyard \x2601  "),"named save creates a world snapshot");std::wstring first=saveEntries[saveSelected].path;
 World expected=world,decoded;SavedView view{};std::wstring name;
 check(readNamedSave(first,decoded,view,name),"named save reloads from disk");
 check(name==L"Quiet courtyard \x2601"&&decoded.n==world.n&&memcmp(decoded.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0&&decoded.cables.size()==1,"named save preserves Unicode name, all piece types, rotations and cables");
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
