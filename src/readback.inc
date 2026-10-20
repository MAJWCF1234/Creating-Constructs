// Capture the app's own presented Vulkan image, independent of desktop occlusion.
bool capturePending=false,inspectVines=false;std::wstring captureFile;
VkBuffer readbackBuffer=VK_NULL_HANDLE;VkDeviceMemory readbackMemory=VK_NULL_HANDLE;void* readbackMap=nullptr;
void recordReadback(uint32_t index){
 if(!capturePending||frames!=4)return;
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
 FILE* f=nullptr;_wfopen_s(&f,captureFile.c_str(),L"wb");if(f){fwrite(&file,sizeof(file),1,f);fwrite(&info,sizeof(info),1,f);fwrite(bytes,4,(size_t)extent.width*extent.height,f);fclose(f);log("Vulkan viewport captured");}else log("ERROR: viewport capture file could not be written");
 vkUnmapMemory(device,readbackMemory);vkDestroyBuffer(device,readbackBuffer,nullptr);vkFreeMemory(device,readbackMemory,nullptr);readbackMap=nullptr;capturePending=false;
}
