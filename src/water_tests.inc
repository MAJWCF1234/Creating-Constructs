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
