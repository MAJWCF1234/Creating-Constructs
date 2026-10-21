// Site vegetation follows actual surfaces instead of adding floating concrete shelves.
bool natureSupport(V start,V direction,float reach,V& result){
 bool before=solidAt(transform(start));if(before)return false;
 for(float d=.035f;d<=reach;d+=.035f){V p=start+direction*d;if(!solidAt(transform(p)))continue;V inside=p,outside=p-direction*.035f;
  for(int i=0;i<7;i++){V mid=(inside+outside)*.5f;if(solidAt(transform(mid)))inside=mid;else outside=mid;}result=outside-direction*.012f;return true;
 }return false;
}
float natureFloor(float x,float z,bool slope=false){V surface;if(natureSupport({x,-.43f,z},{0,-1,0},2.35f,surface)){
 V p=transform(surface);int cx=(int)floorf(p.x),cy=(int)floorf(p.y-.03f),cz=(int)floorf(p.z);float fx=p.x-cx,fz=p.z-cz;
 if(get(cx,cy,cz).t==17){float left=get(cx-1,cy,cz).t==17?0.f:.18f,right=get(cx+1,cy,cz).t==17?1.f:.82f,back=get(cx,cy,cz-1).t==17?0.f:.18f,front=get(cx,cy,cz+1).t==17?1.f:.82f;if(fx>left&&fx<right&&fz>back&&fz<front)return cy+.392f-origin.y;}
 return surface.y;
 }return slope?-z:-.485f;}
void natureFreeLeaf(V root,V axis,V N,float size,IvyRandom& rng){
 N=norm(N);axis=norm(axis-N*dot(axis,N));V across=norm(cross(axis,N));V center=root+axis*(size*.43f)+N*(size*.05f);
 if(!ivyReserveLeaf(transform(center),size))return;
 const float shape[8][2]={{0,0},{-.36f,.18f},{-.5f,.44f},{-.19f,.57f},{0,1},{.23f,.57f},{.5f,.42f},{.35f,.17f}};V edge[8];
 for(int i=0;i<8;i++)edge[i]=root+axis*(shape[i][1]*size)+across*(shape[i][0]*size*.75f);
 float previous=ambient;ambient*=rng.range(.82f,1.06f);for(int i=0;i<8;i++)tri(center,edge[i],edge[(i+1)%8],20);ambient=previous;
}
void natureHangingFern(int variant,V root,float size,float angle){
 switch(variant%3){case 0:natureModel(nature_bracken,root,size,angle,false,1,true);break;case 1:natureModel(nature_bracken_2,root,size,angle,false,1,true);break;default:natureModel(nature_bracken_3,root,size,angle,false,1,true);break;}
}
void natureHanging(int type){
 IvyRandom rng{natureSeed()^(uint32_t)(type*73)};bool continuation=get((int)floorf(origin.x),(int)floorf(origin.y)+1,(int)floorf(origin.z)).t==type;
 bool supported=false;V sample;for(float x:{-.3f,0.f,.3f})for(float z:{-.3f,0.f,.3f})supported=supported||natureSupport({x,.43f,z},{0,1,0},1.6f,sample);
 for(int i=0;i<4;i++){
  V root{rng.range(-.38f,.38f),.43f,rng.range(-.36f,.36f)},support;
  if(i==0){root.x=0;root.z=0;}
  if(!continuation&&natureSupport(root,{0,1,0},1.6f,support))root=support;
  else {if(supported&&!continuation)continue;root.y=.48f;}
  if(i<3&&!continuation)natureHangingFern((int)(rng.next()*3),root,rng.range(.55f,.82f),rng.range(0,2*PI));
  float length=rng.range(.65f,1.08f),phase=rng.range(0,2*PI);V previous=root;
  for(int j=1;j<=8;j++){float t=j/8.f;V p=root+V{sinf(t*3+phase)*.06f*t,-length*t,cosf(t*4+phase)*.055f*t};if(solidAt(transform(p)))break;
   tube(previous,p,.006f,21);float side=j%2?1.f:-1.f;
   natureFreeLeaf(p,norm(V{side*.65f,-.65f,.12f}),V{rng.range(-.4f,.4f),.15f,1},rng.range(.11f,.18f),rng);previous=p;
  }
 }
}
void natureGreenWall(){
 IvyRandom rng{natureSeed()^0x51u};
 for(int i=0;i<6;i++){
  float x=-.43f+i*.172f;V previous{};bool connected=false;
  for(int j=0;j<6;j++){
   V p{x+rng.range(-.045f,.045f),-.45f+j*.17f,-.43f},surface;
   if(natureSupport(p,{0,0,-1},1.35f,surface))p=surface;else p.z=-.46f;
   p.z+=.025f;if(connected)tube(previous,p,.006f,21);
   natureFreeLeaf(p,norm(V{j%2?.65f:-.65f,.7f,.08f}),V{rng.range(-.2f,.2f),rng.range(-.15f,.15f),1},rng.range(.14f,.21f),rng);previous=p;connected=true;
  }
 }
}
void natureSurfaceCover(int type){
 bool slope=type==88;const int divisions=4;
 float heights[5][5];for(int x=0;x<=divisions;x++)for(int z=0;z<=divisions;z++)heights[x][z]=natureFloor(-.49f+x*.245f,-.49f+z*.245f,slope);
 for(int ix=0;ix<divisions;ix++)for(int iz=0;iz<divisions;iz++){
  float x=-.49f+ix*.245f,z=-.49f+iz*.245f,x1=x+.245f,z1=z+.245f;
  quad({x,heights[ix][iz],z},{x,heights[ix][iz+1],z1},{x1,heights[ix+1][iz+1],z1},{x1,heights[ix+1][iz],z},1);
 }
 IvyRandom rng{natureSeed()^(uint32_t)(type*97)};
 for(int i=0;i<7;i++){
  float x=rng.range(-.38f,.38f),z=rng.range(-.38f,.38f);V root{x,natureFloor(x,z,slope)+.012f,z};float scale=rng.range(.72f,1.02f),angle=rng.range(0,2*PI);
  if(i<2)natureFern((int)(rng.next()*3),root,scale*.58f,angle);
  else if(type==91&&i==2)natureModel(nature_clover,root,scale*.72f,angle,false);
  else natureGrass((int)(rng.next()*6),root,scale,angle,i==4||i==6);
 }
}
void natureCascade(){
 bool above=faceNeighbor(77,0,1),below=faceNeighbor(77,0,-1);V face;
 float z=-.46f;if(natureSupport({0,0,-.42f},{0,0,-1},1.2f,face))z=face.z+.012f;
 // One continuous sheet per cell; stacked cells meet exactly at their boundaries.
 float left=faceNeighbor(77,-1,0)?-.5f:-.49f,right=faceNeighbor(77,1,0)?.5f:.49f,bottom=-.5f;
 auto belowCell=get((int)floorf(origin.x),(int)floorf(origin.y)-1,(int)floorf(origin.z));if(belowCell.t==40)bottom=-.65f;else if(belowCell.t==24)bottom=-1.18f;
 size_t first=transparent.size();quad({left,.5f,z},{right,.5f,z},{right,bottom,z},{left,bottom,z},9);
 for(size_t i=first;i<transparent.size();i++)transparent[i].u=2; // flow, not a pond's volume
 if(!above)quad({-.49f,.495f,z},{-.49f,.495f,z+.12f},{.49f,.495f,z+.12f},{.49f,.495f,z},9);
 if(!below){IvyRandom rng{natureSeed()^177u};for(float x:{-.39f,.39f}){V root;if(natureSupport({x,-.47f,.15f},{0,-1,0},.35f,root))natureGrass((int)(rng.next()*6),root,.7f,rng.range(0,2*PI),true);}}
}
