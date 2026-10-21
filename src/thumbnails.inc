HBITMAP pieceThumbs[PieceCount]{};
COLORREF thumbMaterial(int m){m=decodedSurfaceMaterial(m);if(m==52)return RGB(86,105,98);if(m==48)return RGB(25,205,255);if(m==49)return RGB(255,35,100);if(m==50)return RGB(15,65,70);if(m==51)return RGB(20,135,45);if(m>=22&&m!=23)return RGB(103,116,71);if(m==23)return RGB(81,69,48);if(m==1||m==6||m==16||m==17||m==19||m==20)return RGB(97,137,66);if(m==2)return RGB(137,186,201);if(m==4)return RGB(255,224,155);if(m==5||m==15)return RGB(137,98,65);if(m==7)return RGB(141,77,58);if(m==3||m==8||m==13||m==21)return RGB(82,91,96);if(m==9)return RGB(50,114,132);return RGB(177,176,165);}
void buildThumbnails(){
 World savedWorld=world;V savedOrigin=origin;int savedTurn=turn;float savedAmbient=ambient;
 V view=norm(V{1,.8f,1.2f}),R=norm(cross({0,1,0},view)),U=cross(view,R);
 struct Face{POINT p[3];float depth;COLORREF color;};
 for(int type=0;type<PieceCount;type++){
  world=World{};origin={.5f,.5f,.5f};turn=0;ambient=1;ivyBeginRefresh();
  if(type==15){origin={.5f,.5f,1.5f};put(0,0,0,0);put(0,0,1,15);}
  vertices.clear();transparent.clear();piece(type);vertices.insert(vertices.end(),transparent.begin(),transparent.end());
  float lx=1e6f,ly=1e6f,hx=-1e6f,hy=-1e6f;for(auto v:vertices){float x=dot(v.p,R),y=dot(v.p,U);lx=std::min(lx,x);hx=std::max(hx,x);ly=std::min(ly,y);hy=std::max(hy,y);}
  float scale=39/std::max(.1f,std::max(hx-lx,hy-ly));std::vector<Face> faces;
  for(size_t i=0;i+2<vertices.size();i+=3){Face f{};V N=vertices[i].n;if(dot(N,view)<-.01f)continue;float shade=.35f+.65f*std::max(0.f,dot(N,norm(V{-.5f,1,.8f})));COLORREF c=thumbMaterial((int)vertices[i].m);
   f.color=RGB((int)(GetRValue(c)*shade),(int)(GetGValue(c)*shade),(int)(GetBValue(c)*shade));
   for(int j=0;j<3;j++){V p=vertices[i+j].p;f.p[j]={(LONG)(24+(dot(p,R)-(lx+hx)*.5f)*scale),(LONG)(24-(dot(p,U)-(ly+hy)*.5f)*scale)};f.depth+=dot(p,view);}faces.push_back(f);
  }
  std::sort(faces.begin(),faces.end(),[](const Face&a,const Face&b){return a.depth<b.depth;});
  HDC dc=CreateCompatibleDC(nullptr);BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=48;info.bmiHeader.biHeight=-48;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;void* pixels;
  HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);HGDIOBJ old=SelectObject(dc,bitmap);RECT all{0,0,48,48};HBRUSH background=CreateSolidBrush(RGB(1,2,3));FillRect(dc,&all,background);DeleteObject(background);HGDIOBJ oldPen=SelectObject(dc,GetStockObject(NULL_PEN));
  for(auto f:faces){HBRUSH b=CreateSolidBrush(f.color);HGDIOBJ oldBrush=SelectObject(dc,b);Polygon(dc,f.p,3);SelectObject(dc,oldBrush);DeleteObject(b);}
  SelectObject(dc,oldPen);SelectObject(dc,old);DeleteDC(dc);pieceThumbs[type]=bitmap;
 }
 world=savedWorld;origin=savedOrigin;turn=savedTurn;ambient=savedAmbient;vertices.clear();transparent.clear();ivyBeginRefresh();dirty=true;
}
void drawThumbnail(HDC dc,int x,int y,int type){if(!pieceThumbs[type])return;RECT visible{x,y,x+48,y+48};if(!RectVisible(dc,&visible))return;HDC mem=CreateCompatibleDC(dc);HGDIOBJ old=SelectObject(mem,pieceThumbs[type]);TransparentBlt(dc,x,y,48,48,mem,0,0,48,48,RGB(1,2,3));SelectObject(mem,old);DeleteDC(mem);}
