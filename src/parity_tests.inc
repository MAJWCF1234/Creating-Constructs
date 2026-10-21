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
