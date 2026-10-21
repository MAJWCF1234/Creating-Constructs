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
