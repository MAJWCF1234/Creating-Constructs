#ifndef FOLIAGE_GEOMETRY_DECLARED
#define FOLIAGE_GEOMETRY_DECLARED
layout(std430,binding=2) readonly buffer FoliageGeometry {vec4 records[];} foliage;
#endif
bool leafShadow(vec3 start,vec3 direction){
 int count=floatBitsToInt(foliage.records[0].x),offset=floatBitsToInt(foliage.records[0].y);if(count==0)return false;
 vec3 inverse=1./vec3(abs(direction.x)<1e-6?1e-6:direction.x,abs(direction.y)<1e-6?1e-6:direction.y,abs(direction.z)<1e-6?1e-6:direction.z);
 int node=0;
 for(int visit=0;visit<1024&&node<count;visit++){
  vec4 lo=foliage.records[1+node*2],hi=foliage.records[2+node*2];int packed=floatBitsToInt(lo.w),escape=floatBitsToInt(hi.w);
  vec3 a=(lo.xyz-start)*inverse,b=(hi.xyz-start)*inverse,near=min(a,b),far=max(a,b);
  if(max(max(near.x,near.y),max(near.z,0.))>min(min(far.x,far.y),min(far.z,40.))){node=escape;continue;}
  if(packed==0){node++;continue;}
  int first=packed>>3,n=packed&7;
  for(int i=0;i<n;i++){
   int index=offset+(first+i)*5;vec4 A=foliage.records[index],B=foliage.records[index+1],C=foliage.records[index+2],UV=foliage.records[index+3],NA=foliage.records[index+4];int material=int(UV.w);
   vec3 a=A.xyz,b=B.xyz,c=C.xyz;bool bark=material==23||material==32||material==33||material==35;
   if(material>=22&&material<38&&!bark){float amp=.014*(s.lighting.y>.4?2.6:1.)*(1.+s.lighting.w*.4);vec3 vertices[3]=vec3[3](a,b,c);for(int j=0;j<3;j++){float w=sin(s.eye.w*1.3+vertices[j].x*.7+vertices[j].z)*amp;vertices[j].xz+=vec2(w,w*.4);}a=vertices[0];b=vertices[1];c=vertices[2];}
   vec3 e1=b-a,e2=c-a,P=cross(direction,e2);float det=dot(e1,P);if(abs(det)<1e-7)continue;vec3 T=start-a;float u=dot(T,P)/det;if(u<0.||u>1.)continue;vec3 Q=cross(T,e1);float v=dot(direction,Q)/det;if(v<0.||u+v>1.)continue;float t=dot(e2,Q)/det;if(t<.018||t>40.)continue;
   if(bark||material==20||material==21)return true;
   vec2 uv=vec2(A.w,B.w)*(1.-u-v)+vec2(C.w,UV.x)*u+UV.yz*v;
   int layer=material>=22?material-14:material==6?2:material==17?6:7;
   if(material==16){vec3 point=start+direction*t,n=abs(NA.xyz);uv=(n.y>.65?point.xz:n.x>.65?point.zy:point.xy)*2.;}
   if(textureLod(maps,vec3(uv,float(layer)),0.).a>=.5)return true;
  }node=escape;
 }return false;
}
