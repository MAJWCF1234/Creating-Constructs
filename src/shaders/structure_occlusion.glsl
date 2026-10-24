// Shared scene and volumetric light occlusion.
#define PART_ABS abs
#define PART_CEIL ceil
#define PART_SQRT sqrt
#define PART_MAX max
bool partSolidLocal(int type,vec3 f){
#include "../world/solidity.inc"
}
#undef PART_ABS
#undef PART_CEIL
#undef PART_SQRT
#undef PART_MAX
bool structureSolid(vec3 q){
 ivec3 c=ivec3(floor(q));
 if(any(lessThan(c,ivec3(0)))||c.x>=int(s.cfg.w)||c.z>=int(s.cfg.w)||c.y>=20)return false;
 int v=s.cells[c.x*1280+c.z*20+c.y];if(v==0)return false;
 int t=(v-1)%128,r=((v-1)/128)%4,code=((v-1)/512)%64;
 if(t==3)return false; // Glass transmits the sun; its player collider remains solid.
 vec3 f=fract(q);
 if(code>0){int k=code-1;f+=vec3(k%3-1,k/9,(k/3)%3-1);}
 vec2 d=f.xz-.5;for(int i=0;i<r;i++)d=vec2(-d.y,d.x);f.xz=d+.5;
 if(t==96){float amount=float((v>>24)&127)/127.;float a=amount*amount*(3.-2.*amount)*1.570796327,cosa=cos(a),sina=sin(a);vec2 delta=f.xz-vec2(.08,.14);vec2 door=vec2(delta.x*cosa+delta.y*sina,-delta.x*sina+delta.y*cosa);
  return partSolidLocal(t,f)||(f.y>.04&&f.y<1.88&&door.x>.01&&door.x<.83&&abs(door.y)<.036);
 }
 // Visual shadow silhouettes keep thin rails and glazing separate from player proxies.
 if(t==11){float x=f.x;return abs(f.z-.1)<.031&&(abs(f.y-.92)<.031||abs(f.y-.7)<.021||(f.y<.9&&(abs(x-.03)<.031||abs(x-.5)<.031||abs(x-.97)<.031)));}
 if(t==21)return f.z>.05&&f.z<.55&&abs(mod(f.x,.2)-.1)<.04;
 if(t==33)return abs(f.z-.5)<.15&&(f.y<.35||f.y>.65);
 return partSolidLocal(t,f);
}

bool solid(vec3 q){
 ivec3 c=ivec3(floor(q));if(any(lessThan(c,ivec3(0)))||c.x>=int(s.cfg.w)||c.z>=int(s.cfg.w)||c.y>=20)return false;int cellIndex=c.x*1280+c.z*20+c.y;
 ivec3 sub=clamp(ivec3(fract(q)*4.),ivec3(0),ivec3(3));int bit=sub.x+sub.z*4+sub.y*16;
 if((s.natureMask[cellIndex*2+bit/32]&(1u<<uint(bit%32)))!=0u)return true;
 return structureSolid(q);
}
