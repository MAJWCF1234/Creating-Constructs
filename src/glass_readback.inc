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
#include "ui_blur_roi.inc"
void destroyUiBackground(){if(uiBackgroundMap)vkUnmapMemory(device,uiBackgroundMemory);if(uiBackgroundBuffer)vkDestroyBuffer(device,uiBackgroundBuffer,nullptr);if(uiBackgroundMemory)vkFreeMemory(device,uiBackgroundMemory,nullptr);uiBackgroundMap=nullptr;uiBackgroundBuffer=VK_NULL_HANDLE;uiBackgroundMemory=VK_NULL_HANDLE;uiBackgroundReady=false;}
void recordUiBackground(uint32_t index){
 uiBackgroundRecorded=false;if(hideUI||!captureFile.empty()||frames%8)return;
 if(uiBackgroundWidth!=extent.width||uiBackgroundHeight!=extent.height||!uiBackgroundMap){destroyUiBackground();uiBackgroundWidth=extent.width;uiBackgroundHeight=extent.height;buffer((VkDeviceSize)extent.width*extent.height*4,VK_BUFFER_USAGE_TRANSFER_DST_BIT,uiBackgroundBuffer,uiBackgroundMemory,&uiBackgroundMap);}
 VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};b.srcAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;b.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT;b.oldLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;b.newLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;b.srcQueueFamilyIndex=b.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;b.image=images[index];b.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
 vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,nullptr,0,nullptr,1,&b);
 VkBufferImageCopy c{};c.imageSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};c.imageExtent={extent.width,extent.height,1};vkCmdCopyImageToBuffer(command,images[index],VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,uiBackgroundBuffer,1,&c);
 b.srcAccessMask=VK_ACCESS_TRANSFER_READ_BIT;b.dstAccessMask=0;b.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;b.newLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,0,0,nullptr,0,nullptr,1,&b);uiBackgroundRecorded=true;
}
void finishUiBackground(){if(!uiBackgroundRecorded)return;
 if(swapFormat==VK_FORMAT_R8G8B8A8_UNORM||swapFormat==VK_FORMAT_R8G8B8A8_SRGB){auto p=(unsigned char*)uiBackgroundMap;for(size_t i=0;i<(size_t)uiBackgroundWidth*uiBackgroundHeight*4;i+=4)std::swap(p[i],p[i+2]);}
 blurUiBackground();uiBackgroundReady=true;for(HWND h:{topPanel,sidePanel,dockPanel,modalPanel})if(h)InvalidateRect(h,nullptr,FALSE);
}
