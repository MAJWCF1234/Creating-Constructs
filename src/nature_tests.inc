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
