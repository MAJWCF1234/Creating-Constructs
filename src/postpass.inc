// Offscreen scene + exact HTML post pass (SSAO/bloom/tonemap)
struct PostU{float t,sat,ao,ex,sunX,sunY,rain,night;};
VkImage sceneColorImg=0,sceneDepthImg=0;VkImageView sceneColorView=0,sceneDepthView=0;VkDeviceMemory sceneColorMem=0,sceneDepthMem=0;
VkFramebuffer sceneFB=0;VkRenderPass scenePass=0,postPass=0;VkPipeline postPipeline=0;VkPipelineLayout postPipeLayout=0;
VkDescriptorSetLayout postDescLayout=0;VkDescriptorSet postDescSet=0;VkDescriptorPool postPool=0;
VkBuffer postBuf=0;VkDeviceMemory postMem=0;void* postMap=0;VkSampler sceneSampler=0;

void destroySceneTargets(){
 if(sceneFB)vkDestroyFramebuffer(device,sceneFB,nullptr);sceneFB=0;
 if(sceneColorView)vkDestroyImageView(device,sceneColorView,nullptr);sceneColorView=0;
 if(sceneDepthView)vkDestroyImageView(device,sceneDepthView,nullptr);sceneDepthView=0;
 if(sceneColorImg)vkDestroyImage(device,sceneColorImg,nullptr);sceneColorImg=0;
 if(sceneDepthImg)vkDestroyImage(device,sceneDepthImg,nullptr);sceneDepthImg=0;
 if(sceneColorMem)vkFreeMemory(device,sceneColorMem,nullptr);sceneColorMem=0;
 if(sceneDepthMem)vkFreeMemory(device,sceneDepthMem,nullptr);sceneDepthMem=0;
}
void createSceneTargets(){
 destroySceneTargets();
 // Half-float like HTML WebGLRenderTarget HalfFloatType (needed for bloom >1)
 VkFormat colorFmt=VK_FORMAT_R16G16B16A16_SFLOAT;
 VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};ii.imageType=VK_IMAGE_TYPE_2D;ii.format=colorFmt;ii.extent={extent.width,extent.height,1};ii.mipLevels=ii.arrayLayers=1;ii.samples=VK_SAMPLE_COUNT_1_BIT;ii.tiling=VK_IMAGE_TILING_OPTIMAL;ii.usage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
 vkcheck(vkCreateImage(device,&ii,nullptr,&sceneColorImg),"scene color");VkMemoryRequirements req;vkGetImageMemoryRequirements(device,sceneColorImg,&req);
 VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};ai.allocationSize=req.size;ai.memoryTypeIndex=memoryType(req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
 vkcheck(vkAllocateMemory(device,&ai,nullptr,&sceneColorMem),"scene color mem");vkcheck(vkBindImageMemory(device,sceneColorImg,sceneColorMem,0),"scene color bind");
 sceneColorView=imageView(sceneColorImg,colorFmt,VK_IMAGE_ASPECT_COLOR_BIT);
 ii.format=VK_FORMAT_D32_SFLOAT;ii.usage=VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
 vkcheck(vkCreateImage(device,&ii,nullptr,&sceneDepthImg),"scene depth");vkGetImageMemoryRequirements(device,sceneDepthImg,&req);ai.allocationSize=req.size;ai.memoryTypeIndex=memoryType(req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
 vkcheck(vkAllocateMemory(device,&ai,nullptr,&sceneDepthMem),"scene depth mem");vkcheck(vkBindImageMemory(device,sceneDepthImg,sceneDepthMem,0),"scene depth bind");
 sceneDepthView=imageView(sceneDepthImg,VK_FORMAT_D32_SFLOAT,VK_IMAGE_ASPECT_DEPTH_BIT);
 VkImageView atts[]={sceneColorView,sceneDepthView};VkFramebufferCreateInfo fi{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};fi.renderPass=scenePass;fi.attachmentCount=2;fi.pAttachments=atts;fi.width=extent.width;fi.height=extent.height;fi.layers=1;
 vkcheck(vkCreateFramebuffer(device,&fi,nullptr,&sceneFB),"scene fb");
 // update post descriptors to new views
 if(postDescSet&&sceneSampler){VkDescriptorImageInfo ci{sceneSampler,sceneColorView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};VkDescriptorImageInfo di{sceneSampler,sceneDepthView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
  VkWriteDescriptorSet wr[2]{};wr[0].sType=wr[1].sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;wr[0].dstSet=wr[1].dstSet=postDescSet;wr[0].dstBinding=0;wr[0].descriptorCount=1;wr[0].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;wr[0].pImageInfo=&ci;wr[1].dstBinding=1;wr[1].descriptorCount=1;wr[1].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;wr[1].pImageInfo=&di;vkUpdateDescriptorSets(device,2,wr,0,nullptr);}
}
void initPostPass(){
 samples=VK_SAMPLE_COUNT_1_BIT;
 // scene render pass: color+depth → SHADER_READ
 VkAttachmentDescription at[2]{};
 at[0].format=VK_FORMAT_R16G16B16A16_SFLOAT;at[0].samples=VK_SAMPLE_COUNT_1_BIT;at[0].loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;at[0].storeOp=VK_ATTACHMENT_STORE_OP_STORE;at[0].initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;at[0].finalLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
 at[1].format=VK_FORMAT_D32_SFLOAT;at[1].samples=VK_SAMPLE_COUNT_1_BIT;at[1].loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;at[1].storeOp=VK_ATTACHMENT_STORE_OP_STORE;at[1].stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;at[1].stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;at[1].initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;at[1].finalLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
 VkAttachmentReference color{0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},depth{1,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
 VkSubpassDescription sub{};sub.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;sub.colorAttachmentCount=1;sub.pColorAttachments=&color;sub.pDepthStencilAttachment=&depth;
 VkSubpassDependency dep{};dep.srcSubpass=VK_SUBPASS_EXTERNAL;dep.dstSubpass=0;dep.srcStageMask=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;dep.dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT|VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;dep.srcAccessMask=VK_ACCESS_SHADER_READ_BIT;dep.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
 VkSubpassDependency dep2{};dep2.srcSubpass=0;dep2.dstSubpass=VK_SUBPASS_EXTERNAL;dep2.srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT|VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;dep2.dstStageMask=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;dep2.srcAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;dep2.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
 VkSubpassDependency deps[]={dep,dep2};VkRenderPassCreateInfo ri{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};ri.attachmentCount=2;ri.pAttachments=at;ri.subpassCount=1;ri.pSubpasses=&sub;ri.dependencyCount=2;ri.pDependencies=deps;
 vkcheck(vkCreateRenderPass(device,&ri,nullptr,&scenePass),"scene pass");
 // post pass: swapchain color only
 VkAttachmentDescription pat{};pat.format=swapFormat;pat.samples=VK_SAMPLE_COUNT_1_BIT;pat.loadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;pat.storeOp=VK_ATTACHMENT_STORE_OP_STORE;pat.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;pat.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
 VkAttachmentReference pcolor{0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};VkSubpassDescription psub{};psub.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;psub.colorAttachmentCount=1;psub.pColorAttachments=&pcolor;
 VkSubpassDependency pdep{};pdep.srcSubpass=VK_SUBPASS_EXTERNAL;pdep.dstSubpass=0;pdep.srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;pdep.dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;pdep.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
 VkRenderPassCreateInfo pri{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};pri.attachmentCount=1;pri.pAttachments=&pat;pri.subpassCount=1;pri.pSubpasses=&psub;pri.dependencyCount=1;pri.pDependencies=&pdep;
 vkcheck(vkCreateRenderPass(device,&pri,nullptr,&postPass),"post pass");
 // post descriptors: tD, tZ, PostU
 VkDescriptorSetLayoutBinding pb[3]={{0,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_FRAGMENT_BIT,nullptr},{1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,VK_SHADER_STAGE_FRAGMENT_BIT,nullptr},{2,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1,VK_SHADER_STAGE_FRAGMENT_BIT,nullptr}};
 VkDescriptorSetLayoutCreateInfo pli{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};pli.bindingCount=3;pli.pBindings=pb;vkcheck(vkCreateDescriptorSetLayout(device,&pli,nullptr,&postDescLayout),"post layout");
 VkPipelineLayoutCreateInfo ppli{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};ppli.setLayoutCount=1;ppli.pSetLayouts=&postDescLayout;vkcheck(vkCreatePipelineLayout(device,&ppli,nullptr,&postPipeLayout),"post pipe layout");
 buffer(sizeof(PostU),VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,postBuf,postMem,&postMap);
 VkDescriptorPoolSize pps[2]={{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2},{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1}};
 VkDescriptorPoolCreateInfo ppi{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};ppi.maxSets=1;ppi.poolSizeCount=2;ppi.pPoolSizes=pps;vkcheck(vkCreateDescriptorPool(device,&ppi,nullptr,&postPool),"post pool");
 VkDescriptorSetAllocateInfo dai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};dai.descriptorPool=postPool;dai.descriptorSetCount=1;dai.pSetLayouts=&postDescLayout;vkcheck(vkAllocateDescriptorSets(device,&dai,&postDescSet),"post set");
 VkSamplerCreateInfo sp{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};sp.magFilter=sp.minFilter=VK_FILTER_NEAREST;sp.addressModeU=sp.addressModeV=sp.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
 vkcheck(vkCreateSampler(device,&sp,nullptr,&sceneSampler),"scene sampler");
 VkDescriptorBufferInfo pbi{postBuf,0,sizeof(PostU)};
 VkWriteDescriptorSet wr{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};wr.dstSet=postDescSet;wr.dstBinding=2;wr.descriptorCount=1;wr.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;wr.pBufferInfo=&pbi;vkUpdateDescriptorSets(device,1,&wr,0,nullptr);
 // post pipeline
 VkShaderModule vs=shader(post_vert,sizeof(post_vert)),fs=shader(post_frag,sizeof(post_frag));
 VkPipelineShaderStageCreateInfo stages[2]{};for(int i=0;i<2;i++){stages[i].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;stages[i].stage=i?VK_SHADER_STAGE_FRAGMENT_BIT:VK_SHADER_STAGE_VERTEX_BIT;stages[i].module=i?fs:vs;stages[i].pName="main";}
 VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
 VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};ia.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
 VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};vp.viewportCount=vp.scissorCount=1;
 VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};rs.polygonMode=VK_POLYGON_MODE_FILL;rs.cullMode=VK_CULL_MODE_NONE;rs.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE;rs.lineWidth=1;
 VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};ms.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
 VkPipelineDepthStencilStateCreateInfo ds{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
 VkPipelineColorBlendAttachmentState att{};att.colorWriteMask=15;
 VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};cb.attachmentCount=1;cb.pAttachments=&att;
 VkDynamicState dyn[]={VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};VkPipelineDynamicStateCreateInfo dy{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};dy.dynamicStateCount=2;dy.pDynamicStates=dyn;
 VkGraphicsPipelineCreateInfo gpi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};gpi.stageCount=2;gpi.pStages=stages;gpi.pVertexInputState=&vi;gpi.pInputAssemblyState=&ia;gpi.pViewportState=&vp;gpi.pRasterizationState=&rs;gpi.pMultisampleState=&ms;gpi.pDepthStencilState=&ds;gpi.pColorBlendState=&cb;gpi.pDynamicState=&dy;gpi.layout=postPipeLayout;gpi.renderPass=postPass;
 vkcheck(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&gpi,nullptr,&postPipeline),"post pipeline");
 vkDestroyShaderModule(device,vs,nullptr);vkDestroyShaderModule(device,fs,nullptr);
}
