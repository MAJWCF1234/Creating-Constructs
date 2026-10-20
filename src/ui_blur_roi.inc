// The same integer blur kernel, limited to visible panel pixels plus its full halo.
bool verifyGlassBlur=false;int verifiedBlurRegions=0;std::vector<RECT> uiBlurCoverage;
std::vector<uint32_t> uiBlurScratchA,uiBlurScratchB;
// StretchDIBits crop coordinates remain bottom-origin even for a top-down DIB.
RECT uiSampleRect(RECT rect){LONG top=(LONG)uiBackgroundHeight-rect.bottom,bottom=(LONG)uiBackgroundHeight-rect.top;return {rect.left,top,rect.right,bottom};}
void blurUiRect(RECT rect){
 rect.left=std::max(0L,rect.left);rect.top=std::max(0L,rect.top);rect.right=std::min((LONG)uiBackgroundWidth,rect.right);rect.bottom=std::min((LONG)uiBackgroundHeight,rect.bottom);if(rect.left>=rect.right||rect.top>=rect.bottom)return;
 int fullW=(uiBackgroundWidth+1)/2,fullH=(uiBackgroundHeight+1)/2;
 int left=std::max(0,(int)rect.left/2-10),top=std::max(0,(int)rect.top/2-10),right=std::min(fullW,((int)rect.right+1)/2+10),bottom=std::min(fullH,((int)rect.bottom+1)/2+10),width=right-left,height=bottom-top;
 auto& a=uiBlurScratchA;auto& b=uiBlurScratchB;a.resize((size_t)width*height);b.resize(a.size());auto pixels=(uint32_t*)uiBackgroundMap;
 for(int y=0;y<height;y++)for(int x=0;x<width;x++)a[y*width+x]=pixels[std::min((y+top)*2,(int)uiBackgroundHeight-1)*uiBackgroundWidth+std::min((x+left)*2,(int)uiBackgroundWidth-1)];
 auto filter=[&](bool horizontal){int major=horizontal?height:width,minor=horizontal?width:height;auto pos=[&](int m,int n){return horizontal?m*width+n:n*width+m;};
  for(int m=0;m<major;m++){int blue=0,green=0,red=0;auto add=[&](uint32_t c,int sign){blue+=sign*(c&255);green+=sign*((c>>8)&255);red+=sign*((c>>16)&255);};for(int k=-4;k<=4;k++)add(a[pos(m,std::clamp(k,0,minor-1))],1);
   for(int n=0;n<minor;n++){b[pos(m,n)]=0xff000000u|(uint32_t)(red/9)<<16|(uint32_t)(green/9)<<8|(uint32_t)(blue/9);add(a[pos(m,std::clamp(n-4,0,minor-1))],-1);add(a[pos(m,std::clamp(n+5,0,minor-1))],1);}
  }a.swap(b);
 };
 filter(true);filter(false);filter(true);filter(false);uiBlurred.resize((size_t)uiBackgroundWidth*uiBackgroundHeight);
 for(int y=rect.top;y<rect.bottom;y++)for(int x=rect.left;x<rect.right;x++){
  int xx=x/2-left,yy=y/2-top;uint32_t c[4]={a[yy*width+xx],a[yy*width+std::min(xx+1,width-1)],a[std::min(yy+1,height-1)*width+xx],a[std::min(yy+1,height-1)*width+std::min(xx+1,width-1)]};uint32_t value=0xff000000;
  for(int shift=0;shift<24;shift+=8){int t=(c[0]>>shift)&255,bottomValue=(c[2]>>shift)&255;if(x&1){t=(t+((c[1]>>shift)&255))/2;bottomValue=(bottomValue+((c[3]>>shift)&255))/2;}int v=(y&1)?(t+bottomValue)/2:t;value|=(uint32_t)v<<shift;}uiBlurred[y*uiBackgroundWidth+x]=value;
 }
 uiBlurCoverage.push_back(rect);
}
void ensureUiGlassRect(RECT rect){
 if(!uiBackgroundReady)return;
 rect=uiSampleRect(rect);
 for(auto r:uiBlurCoverage)if(r.left<=rect.left&&r.top<=rect.top&&r.right>=rect.right&&r.bottom>=rect.bottom)return;
 blurUiRect(rect);
}
void blurUiBackground(){
 uiBlurCoverage.clear();
 for(HWND h:{topPanel,sidePanel,dockPanel,modalPanel})if(h&&IsWindowVisible(h)){RECT r;GetClientRect(h,&r);MapWindowPoints(h,mainWindow,(POINT*)&r,2);blurUiRect(uiSampleRect(r));}
 if(verifyGlassBlur){
  std::vector<uint32_t> optimized=uiBlurred;blurUiBackgroundReference();
  for(auto r:uiBlurCoverage){for(int y=r.top;y<r.bottom;y++)for(int x=r.left;x<r.right;x++)if(optimized[y*uiBackgroundWidth+x]!=uiBlurred[y*uiBackgroundWidth+x])fail("Glass blur appearance comparison failed");verifiedBlurRegions++;}
  uiBlurred.swap(optimized);
 }
}
