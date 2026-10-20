void worldSizeSelfTest(){
 World previous=world;std::wstring oldPath=dataPath;world=World{};world.n=32;
 put(31,19,31,19,3);put(16,2,23,14,2);put(0,0,31,5);world.cables={{{1,4,2},{31,5,30}}};World expected=world;
 check(idx(31,19,31)==WorldCells-1&&get(31,19,31).t==19,"32x32 grid indexes its far corner without overlap");
 save();world=World{};check(load()&&world.n==32&&memcmp(expected.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"32x32 autosave round trip retains far-edge cells");
 std::string json=exportWorldJson();check(loadWorldJson(json.c_str())&&world.n==32&&get(31,19,31).t==19&&get(16,2,23).t==14,"Share JSON preserves positions beyond the old 16-tile boundary");
 std::wstring named=savesDirectory+L"\\wide-test.mns",name;SavedView v{};World decoded;
 check(writeNamedSave(named,L"32 tile world")&&readNamedSave(named,decoded,v,name)&&decoded.n==32&&memcmp(decoded.c.data(),world.c.data(),sizeof(Cell)*WorldCells)==0,"named saves retain the complete 32x32 world");DeleteFileW(named.c_str());
 // Independent fixtures in the original binary layouts prove backward compatibility.
 std::array<Cell,5120> legacy;legacy[15*320+15*20+2]={19,3};legacy[2*320+3*20+4]={15,1};uint32_t count=0,magic=0x324f4e4d;int size=16;
 std::wstring legacyPath=savesDirectory+L"\\legacy-test.bin";FILE* f=nullptr;_wfopen_s(&f,legacyPath.c_str(),L"wb");fwrite(&magic,4,1,f);fwrite(&size,4,1,f);fwrite(legacy.data(),sizeof(Cell),legacy.size(),f);fwrite(&count,4,1,f);fclose(f);
 dataPath=legacyPath;check(load()&&world.n==16&&get(15,2,15).t==19&&get(2,4,3).t==15,"old 16x16 autosaves migrate with their original coordinates");DeleteFileW(legacyPath.c_str());
 std::wstring legacySlot=savesDirectory+L"\\legacy-test.mns";magic=0x31534e4d;wchar_t title[64]=L"Legacy courtyard";v=savedView();_wfopen_s(&f,legacySlot.c_str(),L"wb");fwrite(&magic,4,1,f);fwrite(title,sizeof(title),1,f);fwrite(&v,sizeof(v),1,f);fwrite(&size,4,1,f);fwrite(legacy.data(),sizeof(Cell),legacy.size(),f);fwrite(&count,4,1,f);fclose(f);
 check(readNamedSave(legacySlot,decoded,v,name)&&decoded.n==16&&decoded.c[idx(15,2,15)].t==19,"old named saves migrate to the wider stride");DeleteFileW(legacySlot.c_str());
 dataPath=oldPath;world=previous;history.clear();dirty=true;
}
