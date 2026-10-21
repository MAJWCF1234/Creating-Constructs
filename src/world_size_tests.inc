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
