layout(std430,binding=2) readonly buffer FoliageGeometry {vec4 records[];} foliage;
vec3 waterPlantWind(vec3 p,int material){if(material>=22&&material!=23){float w=sin(s.eye.w*1.3+p.x*.7+p.z)*.014;p.x+=w;p.z+=w*.4;}return p;}
bool waterPlantBox(vec3 start,vec3 invDirection,vec3 lo,vec3 hi,float maximum){vec3 a=(lo-start)*invDirection,b=(hi-start)*invDirection;vec3 near=min(a,b),far=max(a,b);return max(max(near.x,near.y),max(near.z,0.))<=min(min(far.x,far.y),min(far.z,maximum));}
bool waterMayHitPlant(vec3 start,vec3 direction,vec3 inverse,float maximum){
 vec3 a=(vec3(-4,-2,-4)-start)*inverse,b=(vec3(36,26,36)-start)*inverse,near=min(a,b),far=max(a,b);float entry=max(0.,max(near.x,max(near.y,near.z))),exit=min(maximum,min(far.x,min(far.y,far.z)));if(entry>exit)return false;
 vec3 q=start+direction*(entry+.001);ivec3 cell=ivec3(floor(q))+ivec3(4,2,4),step=ivec3(sign(direction));vec3 boundary=floor(q)+vec3(direction.x>=0.?1.:0.,direction.y>=0.?1.:0.,direction.z>=0.?1.:0.);vec3 next=(boundary-start)*inverse,delta=abs(inverse);
 int offset=floatBitsToInt(foliage.records[0].w);
 for(int visit=0;visit<128;visit++){
  if(any(lessThan(cell,ivec3(0)))||any(greaterThanEqual(cell,ivec3(40,28,40))))break;
  int bit=cell.x*1120+cell.z*28+cell.y,word=bit/32;uint value=floatBitsToUint(foliage.records[offset+word/4][word%4]);if((value&(1u<<uint(bit%32)))!=0u)return true;
  int axis=next.x<next.y?(next.x<next.z?0:2):(next.y<next.z?1:2);if(next[axis]>exit)break;cell[axis]+=step[axis];next[axis]+=delta[axis];
 }
 return false;
}
bool waterPlantTrace(vec3 start,vec3 direction,inout float maximum,out vec3 hit,out vec3 normal,out vec3 albedo,out float ambient){
 int count=floatBitsToInt(foliage.records[0].x),offset=floatBitsToInt(foliage.records[0].y);int node=0;bool found=false;
 vec3 safeDirection=vec3(abs(direction.x)<1e-7?1e-7:direction.x,abs(direction.y)<1e-7?1e-7:direction.y,abs(direction.z)<1e-7?1e-7:direction.z);vec3 inverse=1./safeDirection;
 if(count==0||!waterMayHitPlant(start,direction,inverse,maximum))return false;
 for(int visit=0;visit<1024&&node<count;visit++){
  vec4 lo=foliage.records[1+node*2],hi=foliage.records[2+node*2];int packed=floatBitsToInt(lo.w),escape=floatBitsToInt(hi.w);
  if(!waterPlantBox(start,inverse,lo.xyz,hi.xyz,maximum)){node=escape;continue;}
  if(packed==0){node++;continue;}
  int first=packed>>3,n=packed&7;
  for(int i=0;i<n;i++){
   int index=offset+(first+i)*5;vec4 A=foliage.records[index],B=foliage.records[index+1],C=foliage.records[index+2],UV=foliage.records[index+3],NA=foliage.records[index+4];int material=int(UV.w);
   vec3 a=waterPlantWind(A.xyz,material),b=waterPlantWind(B.xyz,material),c=waterPlantWind(C.xyz,material),e1=b-a,e2=c-a;
   vec3 P=cross(direction,e2);float det=dot(e1,P);if(abs(det)<1e-7)continue;float reciprocal=1./det;vec3 T=start-a;float u=dot(T,P)*reciprocal;if(u<0.||u>1.)continue;
   vec3 Q=cross(T,e1);float v=dot(direction,Q)*reciprocal;if(v<0.||u+v>1.)continue;float distance=dot(e2,Q)*reciprocal;if(distance<.008||distance>=maximum)continue;
   vec3 point=start+direction*distance;vec2 uv=vec2(A.w,B.w)*(1.-u-v)+vec2(C.w,UV.x)*u+UV.yz*v;vec3 N=normalize(cross(e1,e2));vec4 sampleColor=vec4(1);vec3 color;
   if(material>=22){sampleColor=textureLod(maps,vec3(uv,float(material-14)),0.);if(material!=23&&sampleColor.a<.48)continue;float lum=dot(sampleColor.rgb,vec3(.299,.587,.114));color=material==23?sampleColor.rgb*vec3(.78,.79,.73):mix(vec3(lum),sampleColor.rgb,.25)*vec3(.69,.72,.61);}
   else if(material==20){float variation=fract(sin(dot(floor(point*31.),vec3(127.1,311.7,74.7)))*43758.5453);color=mix(vec3(.095,.145,.048),vec3(.23,.31,.105),variation);}
   else if(material==21)color=vec3(.105,.105,.052);
   else {int layer=material==6?2:material==17?6:7;if(material==16){vec3 n=abs(NA.xyz);uv=(n.y>.65?point.xz:n.x>.65?point.zy:point.xy)*2.;}sampleColor=textureLod(maps,vec3(uv,float(layer)),0.);if(sampleColor.a<(material==6||material==16?.5:.45))continue;color=material==17?sampleColor.rgb*vec3(.706,.784,.604):applyBiome(sampleColor.rgb,biomeFoliage(point));}
   maximum=distance;hit=point;normal=dot(N,direction)>0.?-N:N;albedo=color;ambient=NA.w;found=true;
  }
  node=escape;
 }
 return found;
}
