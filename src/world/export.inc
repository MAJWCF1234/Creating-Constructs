// Wavefront OBJ + ASCII FBX export of the current baked static mesh.
// Always refresh first — dynamicGeometry clears `vertices` every frame, so a dirty-only
// export wrote empty files while still reporting the stale piece count.
static void exportWriteMaterials(FILE* m){
 auto write=[&](int id,float r,float g,float b,float Ns=20){fprintf(m,"newmtl mat%d\nKd %.3f %.3f %.3f\nNs %.1f\nillum 2\n\n",id,r,g,b,Ns);};
 write(0,.55f,.54f,.52f);write(1,.45f,.55f,.35f);write(2,.61f,.77f,.81f,80);write(3,.17f,.17f,.18f,40);write(4,1.f,.96f,.85f);
 write(5,.45f,.32f,.2f);write(7,.5f,.35f,.28f);write(8,.55f,.55f,.58f,60);write(9,.3f,.5f,.65f,10);write(11,.47f,.47f,.47f);
 write(13,.08f,.08f,.08f);write(15,.29f,.23f,.16f);write(16,.4f,.55f,.3f);write(17,.7f,.78f,.6f);write(18,.62f,.6f,.56f);
 write(19,.55f,.52f,.48f);write(38,.5f,.48f,.45f);write(39,.35f,.32f,.28f);write(41,.92f,.78f,.12f);write(42,1.f,.72f,.42f);write(43,.72f,.86f,1.f);write(48,.08f,.8f,1.f);write(49,1.f,.08f,.36f);write(50,.025f,.18f,.20f);write(51,.03f,.38f,.07f);write(52,.25f,.31f,.29f);
}
static size_t exportMeshVertexCount(){
 size_t count=0;for(size_t i=0;i+2<vertices.size();i+=3)if((int)vertices[i].m!=10)count+=3;return count;
}
bool exportWorldObj(const std::wstring& path){
 refresh();renderDoorLeaves();
 FILE* f=nullptr;_wfopen_s(&f,path.c_str(),L"wb");if(!f)return false;
 fprintf(f,"# Creating Constructs mesh export\n# pieces=%d N=%d verts=%zu\nmtllib world.mtl\no Construct\n",pieces,world.n,exportMeshVertexCount());
 // Remap skipping ghost tris so face indices stay contiguous.
 std::vector<int> remap(vertices.size(),0);int next=1;
 for(size_t i=0;i+2<vertices.size();i+=3){
  if((int)vertices[i].m==10)continue;
  for(int k=0;k<3;k++){const Vertex& v=vertices[i+k];fprintf(f,"v %.5f %.5f %.5f\n",v.p.x,v.p.y,v.p.z);remap[i+k]=next++;}
 }
 for(size_t i=0;i+2<vertices.size();i+=3){
  if((int)vertices[i].m==10)continue;
  for(int k=0;k<3;k++){const Vertex& v=vertices[i+k];fprintf(f,"vn %.5f %.5f %.5f\n",v.n.x,v.n.y,v.n.z);}
 }
 int lastMat=-999;for(size_t i=0;i+2<vertices.size();i+=3){
  int mat=decodedSurfaceMaterial((int)vertices[i].m);if(mat==10)continue;
  if(mat!=lastMat){fprintf(f,"usemtl mat%d\n",mat);lastMat=mat;}
  int a=remap[i],b=remap[i+1],c=remap[i+2];
  fprintf(f,"f %d//%d %d//%d %d//%d\n",a,a,b,b,c,c);
 }
 fclose(f);
 std::wstring mtl=path;size_t slash=mtl.find_last_of(L"/\\");std::wstring dir=slash==std::wstring::npos?L".":mtl.substr(0,slash);
 std::wstring mtlPath=dir+L"\\world.mtl";FILE* m=nullptr;_wfopen_s(&m,mtlPath.c_str(),L"wb");if(m){exportWriteMaterials(m);fclose(m);}
 return exportMeshVertexCount()>0;
}
bool exportWorldFbx(const std::wstring& path){
 refresh();renderDoorLeaves();
 size_t triCount=0;for(size_t i=0;i+2<vertices.size();i+=3)if((int)vertices[i].m!=10)triCount++;
 if(!triCount)return false;
 FILE* f=nullptr;_wfopen_s(&f,path.c_str(),L"wb");if(!f)return false;
 // Minimal ASCII FBX 7.4 that Blender / Maya / Unity accept for a single mesh.
 fprintf(f,"; FBX 7.4.0 project file\n; Creating Constructs export  pieces=%d  tris=%zu\n",pieces,triCount);
 fprintf(f,"FBXHeaderExtension:  {\n\tFBXHeaderVersion: 1003\n\tFBXVersion: 7400\n}\n");
 fprintf(f,"GlobalSettings:  {\n\tVersion: 1000\n\tProperties70:  {\n\t\tP: \"UpAxis\", \"int\", \"Integer\", \"\",1\n\t\tP: \"FrontAxis\", \"int\", \"Integer\", \"\",2\n\t\tP: \"CoordAxis\", \"int\", \"Integer\", \"\",0\n\t\tP: \"UnitScaleFactor\", \"double\", \"Number\", \"\",100\n\t}\n}\n");
 fprintf(f,"Documents:  {\n\tCount: 1\n\tDocument: 1000000000, \"\", \"Scene\" {\n\t\tRootNode: 0\n\t}\n}\n");
 fprintf(f,"References:  {\n}\nDefinitions:  {\n\tVersion: 100\n\tCount: 3\n\tObjectType: \"GlobalSettings\" {\n\t\tCount: 1\n\t}\n\tObjectType: \"Model\" {\n\t\tCount: 1\n\t}\n\tObjectType: \"Geometry\" {\n\t\tCount: 1\n\t}\n}\n");
 fprintf(f,"Objects:  {\n");
 fprintf(f,"\tGeometry: 2000000000, \"Geometry::Construct\", \"Mesh\" {\n");
 fprintf(f,"\t\tVertices: *%zu {\n\t\t\ta: ",triCount*9);
 bool first=true;
 for(size_t i=0;i+2<vertices.size();i+=3){
  if((int)vertices[i].m==10)continue;
  for(int k=0;k<3;k++){const V& p=vertices[i+k].p;fprintf(f,"%s%.5f,%.5f,%.5f",first?"":",",p.x,p.y,p.z);first=false;}
 }
 fprintf(f,"\n\t\t}\n");
 fprintf(f,"\t\tPolygonVertexIndex: *%zu {\n\t\t\ta: ",triCount*3);
 first=true;int vi=0;
 for(size_t i=0;i+2<vertices.size();i+=3){
  if((int)vertices[i].m==10)continue;
  fprintf(f,"%s%d,%d,%d",first?"":",",vi,vi+1,~(vi+2));first=false;vi+=3;
 }
 fprintf(f,"\n\t\t}\n");
 fprintf(f,"\t\tGeometryVersion: 124\n");
 fprintf(f,"\t\tLayerElementNormal: 0 {\n\t\t\tVersion: 101\n\t\t\tName: \"\"\n\t\t\tMappingInformationType: \"ByPolygonVertex\"\n\t\t\tReferenceInformationType: \"Direct\"\n");
 fprintf(f,"\t\t\tNormals: *%zu {\n\t\t\t\ta: ",triCount*9);
 first=true;
 for(size_t i=0;i+2<vertices.size();i+=3){
  if((int)vertices[i].m==10)continue;
  for(int k=0;k<3;k++){const V& n=vertices[i+k].n;fprintf(f,"%s%.5f,%.5f,%.5f",first?"":",",n.x,n.y,n.z);first=false;}
 }
 fprintf(f,"\n\t\t\t}\n\t\t}\n");
 fprintf(f,"\t\tLayer: 0 {\n\t\t\tVersion: 100\n\t\t\tLayerElement:  {\n\t\t\t\tType: \"LayerElementNormal\"\n\t\t\t\tTypedIndex: 0\n\t\t\t}\n\t\t}\n");
 fprintf(f,"\t}\n");
 fprintf(f,"\tModel: 3000000000, \"Model::Construct\", \"Mesh\" {\n\t\tVersion: 232\n\t\tProperties70:  {\n");
 fprintf(f,"\t\t\tP: \"InheritType\", \"enum\", \"\", \"\",1\n\t\t\tP: \"DefaultAttributeIndex\", \"int\", \"Integer\", \"\",0\n");
 fprintf(f,"\t\t}\n\t\tShading: T\n\t\tCulling: \"CullingOff\"\n\t}\n");
 fprintf(f,"}\n");
 fprintf(f,"Connections:  {\n\tC: \"OO\",3000000000,0\n\tC: \"OO\",2000000000,3000000000\n}\n");
 fclose(f);return true;
}
std::wstring uniqueExportPath(const wchar_t* ext){
 SYSTEMTIME st;GetLocalTime(&st);wchar_t name[96];
 swprintf_s(name,L"\\construct-%04d%02d%02d-%02d%02d%02d%ls",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,ext);
 return exportsDirectory+name;
}
std::wstring uniquePhotoPath(){
 SYSTEMTIME st;GetLocalTime(&st);wchar_t name[96];
 swprintf_s(name,L"\\photo-%04d%02d%02d-%02d%02d%02d.bmp",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
 return photosDirectory+name;
}
