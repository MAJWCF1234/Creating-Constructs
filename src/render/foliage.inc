// Exact plant triangles for water rays. The coarse nature mask is only for shadows.
VkBuffer foliageBuffer;VkDeviceMemory foliageMemory;void* foliageMap=nullptr;size_t foliageCapacity=16;
bool reflectedPlant(int material){return material==6||material==16||material==17||material==20||material==21||(material>=22&&material<38);}
struct FoliageTriangle {Vertex v[3];V lo,hi,center;};
struct FoliageNode {V lo,hi;uint32_t packed=0,escape=0;};
std::vector<FoliageTriangle> foliageTriangles;std::vector<FoliageNode> foliageNodes;std::vector<F4> foliageRecords;
V foliageMin(V a,V b){return {std::min(a.x,b.x),std::min(a.y,b.y),std::min(a.z,b.z)};}
V foliageMax(V a,V b){return {std::max(a.x,b.x),std::max(a.y,b.y),std::max(a.z,b.z)};}
float foliageBits(uint32_t bits){float value;memcpy(&value,&bits,4);return value;}
void foliageBuildNode(size_t begin,size_t end){
 uint32_t index=(uint32_t)foliageNodes.size();foliageNodes.push_back({});V lo{1e6f,1e6f,1e6f},hi{-1e6f,-1e6f,-1e6f},clo=lo,chi=hi;
 for(size_t i=begin;i<end;i++){auto& t=foliageTriangles[i];lo=foliageMin(lo,t.lo);hi=foliageMax(hi,t.hi);clo=foliageMin(clo,t.center);chi=foliageMax(chi,t.center);}
 foliageNodes[index].lo=lo-V{.06f,.002f,.026f};foliageNodes[index].hi=hi+V{.06f,.002f,.026f};
 if(end-begin<=4)foliageNodes[index].packed=((uint32_t)begin<<3)|(uint32_t)(end-begin);
 else {V extent=chi-clo;int axis=extent.y>extent.x?1:0;if(extent.z>(axis==0?extent.x:extent.y))axis=2;size_t mid=(begin+end)/2;
  std::nth_element(foliageTriangles.begin()+begin,foliageTriangles.begin()+mid,foliageTriangles.begin()+end,[axis](const FoliageTriangle& a,const FoliageTriangle& b){return axis==0?a.center.x<b.center.x:axis==1?a.center.y<b.center.y:a.center.z<b.center.z;});
  foliageBuildNode(begin,mid);foliageBuildNode(mid,end);
 }
 foliageNodes[index].escape=(uint32_t)foliageNodes.size();
}
void buildFoliageTrace(){
 foliageTriangles.clear();foliageNodes.clear();foliageRecords.clear();
 for(size_t i=0;i+2<vertices.size();i+=3){if(!reflectedPlant((int)vertices[i].m))continue;
  FoliageTriangle t{};for(int j=0;j<3;j++)t.v[j]=vertices[i+j];t.lo=foliageMin(t.v[0].p,foliageMin(t.v[1].p,t.v[2].p));t.hi=foliageMax(t.v[0].p,foliageMax(t.v[1].p,t.v[2].p));t.center=(t.lo+t.hi)*.5f;
  if(len(cross(t.v[1].p-t.v[0].p,t.v[2].p-t.v[0].p))>.000001f)foliageTriangles.push_back(t);
 }
 if(!foliageTriangles.empty())foliageBuildNode(0,foliageTriangles.size());
 uint32_t triangleOffset=1+(uint32_t)foliageNodes.size()*2;foliageRecords.push_back({foliageBits((uint32_t)foliageNodes.size()),foliageBits(triangleOffset),foliageBits((uint32_t)foliageTriangles.size()),0});
 for(auto n:foliageNodes){foliageRecords.push_back({n.lo.x,n.lo.y,n.lo.z,foliageBits(n.packed)});foliageRecords.push_back({n.hi.x,n.hi.y,n.hi.z,foliageBits(n.escape)});}
 for(const auto& t:foliageTriangles){const auto& a=t.v[0];const auto& b=t.v[1];const auto& c=t.v[2];int m=(int)a.m;
  auto uv=[m](const Vertex& v){return (m==6||m==17||m==19)?V{v.n.x,v.n.y,0}:V{v.u,v.v,0};};V A=uv(a),B=uv(b),C=uv(c);V N=norm(cross(b.p-a.p,c.p-a.p));
  foliageRecords.push_back({a.p.x,a.p.y,a.p.z,A.x});foliageRecords.push_back({b.p.x,b.p.y,b.p.z,A.y});foliageRecords.push_back({c.p.x,c.p.y,c.p.z,B.x});foliageRecords.push_back({B.y,C.x,C.y,a.m});foliageRecords.push_back({N.x,N.y,N.z,a.a});
 }
 // A conservative 1 m occupancy grid rejects empty water rays before BVH traversal.
 std::array<uint32_t,1400> occupancy{};
 for(auto t:foliageTriangles){int x0=std::clamp((int)floorf(t.lo.x-.06f)+4,0,39),x1=std::clamp((int)floorf(t.hi.x+.06f)+4,0,39),y0=std::clamp((int)floorf(t.lo.y)+2,0,27),y1=std::clamp((int)floorf(t.hi.y)+2,0,27),z0=std::clamp((int)floorf(t.lo.z-.026f)+4,0,39),z1=std::clamp((int)floorf(t.hi.z+.026f)+4,0,39);
  for(int x=x0;x<=x1;x++)for(int z=z0;z<=z1;z++)for(int y=y0;y<=y1;y++){int bit=x*1120+z*28+y;occupancy[bit/32]|=1u<<(bit%32);}
 }
 foliageRecords[0].w=foliageBits((uint32_t)foliageRecords.size());for(int i=0;i<1400;i+=4)foliageRecords.push_back({foliageBits(occupancy[i]),foliageBits(occupancy[i+1]),foliageBits(occupancy[i+2]),foliageBits(occupancy[i+3])});
 size_t bytes=foliageRecords.size()*sizeof(F4);if(bytes>128*1024*1024)fail("Foliage reflection geometry capacity exceeded");
 if(bytes>foliageCapacity){vkUnmapMemory(device,foliageMemory);vkDestroyBuffer(device,foliageBuffer,nullptr);vkFreeMemory(device,foliageMemory,nullptr);foliageCapacity=bytes;buffer(bytes,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,foliageBuffer,foliageMemory,&foliageMap);
  VkDescriptorBufferInfo bi{foliageBuffer,0,bytes};VkWriteDescriptorSet wr{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};wr.dstSet=descriptorSet;wr.dstBinding=2;wr.descriptorCount=1;wr.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;wr.pBufferInfo=&bi;vkUpdateDescriptorSets(device,1,&wr,0,nullptr);
 }
 memcpy(foliageMap,foliageRecords.data(),bytes);
}
