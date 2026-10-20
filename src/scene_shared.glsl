#include "style.glsl"
layout(location=0) in vec3 p;
layout(location=1) in vec3 n;
layout(location=2) flat in int mat;
layout(location=3) in float ao; // HTML instanceColor scalar
layout(location=4) in vec2 foliageUv;
layout(location=0) out vec4 outColor;
layout(std430,binding=0) readonly buffer State {
 mat4 vp;vec4 eye;vec4 sun;vec4 sky;vec4 cfg;vec4 lamps[8];vec4 atmos;vec4 lighting;int cells[20480];uint natureMask[];
} s;
layout(binding=1) uniform sampler2DArray maps; // 0 conc 1 moss 2 ivy 3 brick 4 wood 5 steel … 8+N boardform

bool solid(vec3 q);
float softShadow(vec3 start,vec3 L){
 // Approximate THREE.PCFSoftShadowMap radius=3
 float sh=0.;
 vec3 U=normalize(cross(L,abs(L.y)>.9?vec3(1,0,0):vec3(0,1,0)));
 vec3 B=cross(U,L);
 const vec2 o[9]=vec2[9](vec2(0),vec2(1,0),vec2(-1,0),vec2(0,1),vec2(0,-1),vec2(.7,.7),vec2(-.7,.7),vec2(.7,-.7),vec2(-.7,-.7));
 for(int j=0;j<9;j++){
  vec3 dir=normalize(L+(U*o[j].x+B*o[j].y)*.04);
  vec3 q=start;float hit=1.;
  for(int i=0;i<80;i++){q+=dir*.15;if(q.y>22.||q.x<-2.||q.z<-2.||q.x>s.cfg.w+2.||q.z>s.cfg.w+2.)break;if(solid(q)){hit=0.;break;}}
  sh+=hit;
 }
 return sh/9.;
}
bool structureSolid(vec3 q){
 ivec3 c=ivec3(floor(q));
 if(any(lessThan(c,ivec3(0)))||c.x>=int(s.cfg.w)||c.z>=int(s.cfg.w)||c.y>=20)return false;
 int cellIndex=c.x*640+c.z*20+c.y;
 int v=s.cells[cellIndex];if(v==0)return false;
 int t=(v-1)%64,r=(v-1)/64;
 // Exact fast paths: these shapes are independent of local coordinates and rotation.
 if(t==0||t==5||t==18||t==19||t==20)return true;
 if(t==3||t==13||t==15||t==16||t==25||t==27||t==35||t==37||t==40||t==43||t==44||t==45||t==46)return false;
 vec3 f=fract(q);vec2 d=f.xz-.5;
 for(int i=0;i<r;i++)d=vec2(-d.y,d.x);f.xz=d+.5;
 if(t==1)return f.y<.5;if(t==2||t==7||t==14||t==41||t==42)return length(f.xz-.5)<.22;
 if(t==3||t==13||t==15||t==16||t==25||t==27)return false;
 if(t==4)return f.y<1.-f.z;if(t==6)return f.y<ceil((1.-f.z)*4.)/4.;
 if(t==8)return abs(f.z-.5)<.15&&(f.x<.15||f.x>.85||f.y<.25||f.y>.8);
 if(t==9)return abs(f.z-.5)<.15;
 if(t==10)return !(abs(f.x-.5)<.25&&(f.y<.5||length(vec2(f.x-.5,f.y-.5))<.25));
 if(t==11)return f.z<.15;if(t==12)return f.y>.7&&abs(f.z-.5)<.15;
 if(t==17)return f.y<.5;if(t==21)return abs(f.z-.5)<.25;if(t==22)return abs(f.z-.5)<.15;
 if(t==23)return abs(f.z-.5)<.15&&(f.x<.18||f.x>.82||f.y>.85);
 if(t==24)return f.y<.1||(f.y<.5&&max(abs(f.x-.5),abs(f.z-.5))>.4);
 if(t==26)return length(f.xz-.5)<.12&&f.y<.8;
 if(t==28)return dot(f.xz-.5,f.xz-.5)+f.y*f.y<.25;
 if(t==29)return dot(f.xz-.5,f.xz-.5)<.25;
 if(t==30)return f.y<.5+sqrt(max(0.,.25-(f.x-.5)*(f.x-.5)));
 if(t==31)return abs(f.z-.5)<.05;if(t==32||t==33)return abs(f.z-.5)<.15;
 if(t==34)return f.y>.85;if(t==35||t==37||t==40)return false;
 if(t==36)return f.y<ceil((1.-f.z)*8.)/8.;
 if(t==38)return max(abs(f.x-.5),abs(f.z-.5))<.2;
 if(t==39)return f.y<.35&&abs(f.z-.5)<.2&&abs(f.x-.5)<.45;
 return true;
}
bool solid(vec3 q){
 ivec3 c=ivec3(floor(q));if(any(lessThan(c,ivec3(0)))||c.x>=int(s.cfg.w)||c.z>=int(s.cfg.w)||c.y>=20)return false;int cellIndex=c.x*640+c.z*20+c.y;
 ivec3 sub=clamp(ivec3(fract(q)*4.),ivec3(0),ivec3(3));int bit=sub.x+sub.z*4+sub.y*16;
 if((s.natureMask[cellIndex*2+bit/32]&(1u<<uint(bit%32)))!=0u)return true;
 return structureSolid(q);
}

vec3 sampleMap(int layer,vec2 uv){return texture(maps,vec3(uv,float(layer))).rgb;}
float sampleA(int layer,vec2 uv){return texture(maps,vec3(uv,float(layer))).a;}

// Lush concept-art greens (not washed MC plains yellow-olive)
vec3 biomeFoliage(vec3 wp){
 float h=fract(sin(dot(floor(wp.xz*1.7),vec2(127.1,311.7)))*43758.5453);
 if(h<.3)return vec3(.18,.55,.16);  // deep
 if(h<.55)return vec3(.28,.68,.20); // bright
 if(h<.8)return vec3(.22,.60,.18);  // mid
 return vec3(.35,.72,.24);          // sunlit tip
}
vec3 biomeGrass(vec3 wp){
 float h=fract(sin(dot(floor(wp.xz),vec2(269.5,183.3)))*43758.5453);
 if(h<.4)return vec3(.30,.70,.22);
 if(h<.7)return vec3(.22,.62,.18);
 return vec3(.38,.74,.26);
}
// greyscale mask × biome — punch up so vines read green, not grey
vec3 applyBiome(vec3 tex,vec3 biome){
 float lum=max(tex.r,max(tex.g,tex.b));
 return biome*clamp(.4+lum*1.1,.35,1.5);
}

