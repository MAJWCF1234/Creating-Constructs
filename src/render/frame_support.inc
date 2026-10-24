// Opt-in repeatable replay measurements. Normal play keeps its existing timing.
bool benchmarkRun=false,fixedFrame=false,benchmarkStorm=false,benchmarkWalk=false;
VkQueryPool perfQueries=VK_NULL_HANDLE;float perfTimestampPeriod=0;uint32_t perfTimestampBits=0;
double perfCpuStart=0,perfBuildStart=0,perfBuildMs=0,perfBlurMs=0,perfPaintMs=0;
double perfClock(){LARGE_INTEGER t,f;QueryPerformanceCounter(&t);QueryPerformanceFrequency(&f);return t.QuadPart*1000.0/f.QuadPart;}
struct PerfSample{double frame,build,blur,paint,scene,post,copy;};std::vector<PerfSample> perfSamples;
void initPerformance(){if(!benchmarkRun)return;VkPhysicalDeviceProperties p;vkGetPhysicalDeviceProperties(physical,&p);perfTimestampPeriod=p.limits.timestampPeriod;uint32_t n;vkGetPhysicalDeviceQueueFamilyProperties(physical,&n,nullptr);std::vector<VkQueueFamilyProperties> qs(n);vkGetPhysicalDeviceQueueFamilyProperties(physical,&n,qs.data());perfTimestampBits=qs[family].timestampValidBits;if(perfTimestampBits){VkQueryPoolCreateInfo ci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};ci.queryType=VK_QUERY_TYPE_TIMESTAMP;ci.queryCount=4;vkcheck(vkCreateQueryPool(device,&ci,nullptr,&perfQueries),"Profile timestamp pool");}}
void performanceMark(int stage){if(!benchmarkRun||!perfQueries)return;if(!stage)vkCmdResetQueryPool(command,perfQueries,0,4);vkCmdWriteTimestamp(command,stage?VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT:VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,perfQueries,stage);}
void performanceSample(){if(!benchmarkRun)return;uint64_t times[4]{};if(perfQueries)vkcheck(vkGetQueryPoolResults(device,perfQueries,0,4,sizeof(times),times,8,VK_QUERY_RESULT_64_BIT|VK_QUERY_RESULT_WAIT_BIT),"Read GPU timings");auto ms=[&](int a,int b){uint64_t mask=perfTimestampBits==64?UINT64_MAX:((1ull<<perfTimestampBits)-1);return ((times[b]-times[a])&mask)*perfTimestampPeriod/1000000.0;};if(frames>=60)perfSamples.push_back({perfClock()-perfCpuStart+perfPaintMs,perfBuildMs,perfBlurMs,perfPaintMs,ms(0,1),ms(1,2),ms(2,3)});perfPaintMs=0;}
void writePerformance(){if(!benchmarkRun)return;wchar_t dir[MAX_PATH];GetTempPathW(MAX_PATH,dir);std::wstring path=std::wstring(dir)+L"creating-constructs-performance.json";FILE* f=nullptr;_wfopen_s(&f,path.c_str(),L"wb");if(!f)return;auto mean=[&](double PerfSample::*field){double sum=0;for(auto s:perfSamples)sum+=s.*field;return sum/std::max((size_t)1,perfSamples.size());};std::vector<double> values;for(auto s:perfSamples)values.push_back(s.frame);std::sort(values.begin(),values.end());double p95=values.empty()?0:values[(values.size()-1)*95/100];
 fprintf(f,"{\n\"frames\":%zu,\"width\":%u,\"height\":%u,\"gpu\":\"%s\",\n\"frame_mean_ms\":%.6f,\"frame_p95_ms\":%.6f,\"build_mean_ms\":%.6f,\"blur_mean_ms\":%.6f,\"paint_mean_ms\":%.6f,\"gpu_scene_mean_ms\":%.6f,\"gpu_post_mean_ms\":%.6f,\"gpu_copy_mean_ms\":%.6f,\"validation_errors\":%d\n}",perfSamples.size(),extent.width,extent.height,gpuName.c_str(),mean(&PerfSample::frame),p95,mean(&PerfSample::build),mean(&PerfSample::blur),mean(&PerfSample::paint),mean(&PerfSample::scene),mean(&PerfSample::post),mean(&PerfSample::copy),validationErrors);fclose(f);
}

// Capture the app's own presented Vulkan image, independent of desktop occlusion.
bool capturePending=false,inspectVines=false;std::wstring captureFile;int captureAtFrame=4;
bool auditRun=false;int auditPlantModel=-1;
VkBuffer readbackBuffer=VK_NULL_HANDLE;VkDeviceMemory readbackMemory=VK_NULL_HANDLE;void* readbackMap=nullptr;
void requestPhotoCapture(){if(photosDirectory.empty())return;captureFile=uniquePhotoPath();capturePending=true;captureAtFrame=frames+2;hideUI=true;log("Photo capture queued");}
void recordReadback(uint32_t index){
 if(!capturePending||frames!=captureAtFrame)return;
 buffer((VkDeviceSize)extent.width*extent.height*4,VK_BUFFER_USAGE_TRANSFER_DST_BIT,readbackBuffer,readbackMemory,&readbackMap);
 VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};b.srcAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;b.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;b.oldLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;b.newLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;b.image=images[index];b.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
 vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&b);
 VkBufferImageCopy c{};c.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};c.imageExtent={extent.width,extent.height,1};vkCmdCopyImageToBuffer(command,images[index],VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,readbackBuffer,1,&c);
 b.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;b.dstAccessMask=0;b.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;b.newLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
 vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,0,0,nullptr,0,nullptr,1,&b);
}
void finishReadback(){
 if(!readbackMap)return;
 auto bytes=(unsigned char*)readbackMap;
 if(swapFormat==VK_FORMAT_R8G8B8A8_UNORM||swapFormat==VK_FORMAT_R8G8B8A8_SRGB)for(size_t i=0;i<(size_t)extent.width*extent.height*4;i+=4)std::swap(bytes[i],bytes[i+2]);
 BITMAPFILEHEADER file{};file.bfType=0x4d42;file.bfOffBits=sizeof(file)+sizeof(BITMAPINFOHEADER);file.bfSize=file.bfOffBits+extent.width*extent.height*4;
 BITMAPINFOHEADER info{};info.biSize=sizeof(info);info.biWidth=extent.width;info.biHeight=-(LONG)extent.height;info.biPlanes=1;info.biBitCount=32;
 CreateDirectoryW(photosDirectory.c_str(),nullptr);
 FILE* f=nullptr;_wfopen_s(&f,captureFile.c_str(),L"wb");if(f){fwrite(&file,sizeof(file),1,f);fwrite(&info,sizeof(info),1,f);fwrite(bytes,4,(size_t)extent.width*extent.height,f);fclose(f);log("Vulkan viewport captured");}else log("ERROR: viewport capture file could not be written");
 vkUnmapMemory(device,readbackMemory);vkDestroyBuffer(device,readbackBuffer,nullptr);vkFreeMemory(device,readbackMemory,nullptr);readbackMap=nullptr;capturePending=false;
 if(!smoke&&!benchmarkRun&&!auditRun)hideUI=false;
}

// Read back only the game's image for native frosted-glass panel backgrounds.
VkBuffer uiBackgroundBuffer=VK_NULL_HANDLE;VkDeviceMemory uiBackgroundMemory=VK_NULL_HANDLE;
void* uiBackgroundMap=nullptr;uint32_t uiBackgroundWidth=0,uiBackgroundHeight=0;bool uiBackgroundRecorded=false,uiBackgroundReady=false;
std::vector<uint32_t> uiBlurred;
void blurUiBackgroundReference(){
 int width=(uiBackgroundWidth+1)/2,height=(uiBackgroundHeight+1)/2;std::vector<uint32_t> a((size_t)width*height),b(a.size());auto pixels=(uint32_t*)uiBackgroundMap;
 for(int y=0;y<height;y++)for(int x=0;x<width;x++)a[y*width+x]=pixels[std::min(y*2,(int)uiBackgroundHeight-1)*uiBackgroundWidth+std::min(x*2,(int)uiBackgroundWidth-1)];
 auto filter=[&](bool horizontal){int major=horizontal?height:width,minor=horizontal?width:height;auto pos=[&](int m,int n){return horizontal?m*width+n:n*width+m;};
  for(int m=0;m<major;m++){int blue=0,green=0,red=0;auto add=[&](uint32_t c,int sign){blue+=sign*(c&255);green+=sign*((c>>8)&255);red+=sign*((c>>16)&255);};for(int k=-4;k<=4;k++)add(a[pos(m,std::clamp(k,0,minor-1))],1);
   for(int n=0;n<minor;n++){b[pos(m,n)]=0xff000000u|(uint32_t)(red/9)<<16|(uint32_t)(green/9)<<8|(uint32_t)(blue/9);add(a[pos(m,std::clamp(n-4,0,minor-1))],-1);add(a[pos(m,std::clamp(n+5,0,minor-1))],1);}
  }a.swap(b);
 };
 filter(true);filter(false);filter(true);filter(false);uiBlurred.resize((size_t)uiBackgroundWidth*uiBackgroundHeight);
 for(uint32_t y=0;y<uiBackgroundHeight;y++)for(uint32_t x=0;x<uiBackgroundWidth;x++){
  int xx=x/2,yy=y/2;uint32_t c[4]={a[yy*width+xx],a[yy*width+std::min(xx+1,width-1)],a[std::min(yy+1,height-1)*width+xx],a[std::min(yy+1,height-1)*width+std::min(xx+1,width-1)]};uint32_t value=0xff000000;
  for(int shift=0;shift<24;shift+=8){int top=((c[0]>>shift)&255),bottom=((c[2]>>shift)&255);if(x&1){top=(top+((c[1]>>shift)&255))/2;bottom=(bottom+((c[3]>>shift)&255))/2;}int v=(y&1)?(top+bottom)/2:top;value|=(uint32_t)v<<shift;}uiBlurred[y*uiBackgroundWidth+x]=value;
 }
}
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

void destroyUiBackground(){if(uiBackgroundMap)vkUnmapMemory(device,uiBackgroundMemory);if(uiBackgroundBuffer)vkDestroyBuffer(device,uiBackgroundBuffer,nullptr);if(uiBackgroundMemory)vkFreeMemory(device,uiBackgroundMemory,nullptr);uiBackgroundMap=nullptr;uiBackgroundBuffer=VK_NULL_HANDLE;uiBackgroundMemory=VK_NULL_HANDLE;uiBackgroundReady=false;}
void recordUiBackground(uint32_t index){
 uiBackgroundRecorded=false;if(hideUI||!captureFile.empty()||frames%4)return;
 if(uiBackgroundWidth!=extent.width||uiBackgroundHeight!=extent.height||!uiBackgroundMap){destroyUiBackground();uiBackgroundWidth=extent.width;uiBackgroundHeight=extent.height;buffer((VkDeviceSize)extent.width*extent.height*4,VK_BUFFER_USAGE_TRANSFER_DST_BIT,uiBackgroundBuffer,uiBackgroundMemory,&uiBackgroundMap);}
 VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};b.srcAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;b.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;b.oldLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;b.newLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;b.image=images[index];b.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
 vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&b);
 VkBufferImageCopy c{};c.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};c.imageExtent={extent.width,extent.height,1};vkCmdCopyImageToBuffer(command,images[index],VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,uiBackgroundBuffer,1,&c);
 b.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;b.dstAccessMask=0;b.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;b.newLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,0,0,nullptr,0,nullptr,1,&b);uiBackgroundRecorded=true;
}
void finishUiBackground(){if(!uiBackgroundRecorded)return;
 if(swapFormat==VK_FORMAT_R8G8B8A8_UNORM||swapFormat==VK_FORMAT_R8G8B8A8_SRGB){auto p=(unsigned char*)uiBackgroundMap;for(size_t i=0;i<(size_t)uiBackgroundWidth*uiBackgroundHeight*4;i+=4)std::swap(p[i],p[i+2]);}
 blurUiBackground();uiBackgroundReady=true;for(HWND h:{topPanel,sidePanel,dockPanel,modalPanel})if(h&&IsWindowVisible(h))InvalidateRect(h,nullptr,FALSE);
}
