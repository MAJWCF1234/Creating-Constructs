std::string placementStatus;int ghostMaterial=10;
bool adaptingPiece(int t){return t==6||t==15||t==17||t==22||t==24||t==36||t==40||t==63||t==64||t==77||t==78||(t>=87&&t<=92);}
void previewFrame(V center,float w,float h,float d,int mat){
 float a=.018f;for(float x:{-w*.5f,w*.5f})for(float z:{-d*.5f,d*.5f})box(a,h,a,center.x+x,center.y,center.z+z,mat);
 for(float y:{-h*.5f,h*.5f}){for(float z:{-d*.5f,d*.5f})box(w,a,a,center.x,center.y+y,center.z+z,mat);for(float x:{-w*.5f,w*.5f})box(a,a,d,center.x+x,center.y+y,center.z,mat);}
}
void placementPreview(){
 std::string next;
 if(hasAdd&&selected!=16&&!isPipeTool(selected)){
  int type=selected,r=rotation;autoFacing(type,r);int x=(int)hitAdd.x,y=(int)hitAdd.y,z=(int)hitAdd.z;bool valid=canPlace(x,y,z,type,r);int neighbors=0;
  for(V d:{V{1,0,0},V{-1,0,0},V{0,1,0},V{0,-1,0},V{0,0,1},V{0,0,-1}})if(get(x+(int)d.x,y+(int)d.y,z+(int)d.z).t>=0)neighbors++;
  next=valid?"Ready to place":"Blocked: clear the outlined footprint";
  if(valid&&adaptingPiece(type)&&neighbors)next="Adapts to "+std::to_string(neighbors)+" neighboring pieces";
  int index=idx(x,y,z);Cell old=world.c[index];world.c[index]={(int8_t)type,(int8_t)r};
  if(valid&&(type==23||type==51||type==59)){
   bool clear=false;for(float dx:{.35f,.65f,1.f})for(float dz:{.35f,.65f})if(freeAt({x+dx,y+.02f,z+dz},1.6f))clear=true;
   if(!clear)next="Opening: check 1.6 m player clearance";
  }
  origin=hitAdd+V{.5f,.5f,.5f};turn=r;ghostMaterial=valid?45:44;ghostMode=true;piece(type);ghostMode=false;ghostMaterial=10;turn=0;origin={};
  world.c[index]=old;
  if(!valid)eachFootprintCell(x,y,z,type,r,[&](int cx,int cy,int cz){previewFrame({cx+.5f,cy+.5f,cz+.5f},1.015f,1.015f,1.015f,44);});
  if(valid&&adaptingPiece(type))for(V d:{V{1,0,0},V{-1,0,0},V{0,1,0},V{0,-1,0},V{0,0,1},V{0,0,-1}}){V q=hitAdd+d;if(get((int)q.x,(int)q.y,(int)q.z).t>=0)previewFrame(q+V{.5f,.5f,.5f},1.02f,1.02f,1.02f,46);}
  if(valid&&(type==87||type==90||type==92)&&get(x,y+1,z).t<0)next="Hanging growth: place under a ledge for support";
 }
 if(next!=placementStatus){placementStatus=next;if(topPanel)InvalidateRect(topPanel,nullptr,FALSE);}
}
