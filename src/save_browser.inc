constexpr int SaveThumbW=144,SaveThumbH=80;
void writeSaveThumbnail(const std::wstring& path){
 if(!uiBackgroundReady||!uiBackgroundMap||!uiBackgroundWidth||!uiBackgroundHeight)return;
 std::vector<uint32_t> pixels(SaveThumbW*SaveThumbH);auto source=(uint32_t*)uiBackgroundMap;
 for(int y=0;y<SaveThumbH;y++)for(int x=0;x<SaveThumbW;x++)pixels[y*SaveThumbW+x]=source[(y*uiBackgroundHeight/SaveThumbH)*uiBackgroundWidth+x*uiBackgroundWidth/SaveThumbW];
 BITMAPFILEHEADER file{};file.bfType=0x4d42;file.bfOffBits=sizeof(file)+sizeof(BITMAPINFOHEADER);file.bfSize=file.bfOffBits+(DWORD)pixels.size()*4;
 BITMAPINFOHEADER info{};info.biSize=sizeof(info);info.biWidth=SaveThumbW;info.biHeight=-SaveThumbH;info.biPlanes=1;info.biBitCount=32;
 FILE* f=nullptr;_wfopen_s(&f,(path+L".bmp").c_str(),L"wb");if(f){fwrite(&file,sizeof(file),1,f);fwrite(&info,sizeof(info),1,f);fwrite(pixels.data(),4,pixels.size(),f);fclose(f);}
}
void buildSaveThumbnail(SaveEntry& entry,const World& w){
 entry.thumbnail.resize(SaveThumbW*SaveThumbH);FILE* f=nullptr;_wfopen_s(&f,(entry.path+L".bmp").c_str(),L"rb");
 if(f){BITMAPFILEHEADER file{};BITMAPINFOHEADER info{};bool ok=fread(&file,sizeof(file),1,f)==1&&fread(&info,sizeof(info),1,f)==1&&file.bfType==0x4d42&&info.biWidth==SaveThumbW&&info.biHeight==-SaveThumbH&&info.biBitCount==32&&info.biCompression==0;
  if(ok){fseek(f,file.bfOffBits,SEEK_SET);ok=fread(entry.thumbnail.data(),4,entry.thumbnail.size(),f)==entry.thumbnail.size();}fclose(f);if(ok)return;}
 // Existing saves get an actual plan of their geometry until saved with a camera thumbnail.
 for(int y=0;y<SaveThumbH;y++)for(int x=0;x<SaveThumbW;x++){
  int cx=x*w.n/SaveThumbW,cz=y*w.n/SaveThumbH,highest=-1,type=-1;
  for(int h=0;h<WorldHeight;h++){auto c=w.c[idx(cx,h,cz)];if(c.t>=0){highest=h;type=c.t;}}
  uint32_t shade=highest<0?32u:(uint32_t)(85+highest*6);
  entry.thumbnail[y*SaveThumbW+x]=type==14||type==15||type==5?0xff425c35u:type==40||type==24?0xff345e70u:0xff000000u|shade*0x010101u;
 }
}
void drawSaveThumbnail(HDC dc,const SaveEntry& entry,RECT rect){
 if(entry.thumbnail.empty())return;BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=SaveThumbW;info.bmiHeader.biHeight=-SaveThumbH;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
 SetStretchBltMode(dc,HALFTONE);StretchDIBits(dc,rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top,0,0,SaveThumbW,SaveThumbH,entry.thumbnail.data(),&info,DIB_RGB_COLORS,SRCCOPY);
}

void drawSurfaceThumbnail(HDC dc,int x,int y,int style){
 if(style>=4&&style<=19){int index=style==4?0:style==5?2:style==6?4:style-1;static std::array<std::vector<uint32_t>,SurfaceCount> swatches;auto& pixels=swatches[style];const unsigned char* src=brickPixels[index];
  if(pixels.empty()){pixels.resize(128*128);for(int i=0;i<128*128;i++)pixels[i]=0xff000000u|(uint32_t)src[i*4]<<16|(uint32_t)src[i*4+1]<<8|src[i*4+2];}BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=128;info.bmiHeader.biHeight=-128;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;SetStretchBltMode(dc,HALFTONE);StretchDIBits(dc,x,y,44,44,0,0,128,128,pixels.data(),&info,DIB_RGB_COLORS,SRCCOPY);
 }else{COLORREF colors[]={RGB(145,145,143),RGB(165,162,153),RGB(130,124,114),RGB(145,140,126)};RECT rr{x,y,x+44,y+44};fill(dc,rr,style<4?colors[style]:style==20?RGB(112,80,50):style==21?RGB(105,117,123):style==22?RGB(117,109,89):RGB(65,63,43));}
}
