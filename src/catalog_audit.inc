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
