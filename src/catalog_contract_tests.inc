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
