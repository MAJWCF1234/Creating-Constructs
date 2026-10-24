#include "style.glsl"
layout(location=0) in vec3 p;
layout(location=1) in vec3 n;
layout(location=2) flat in int mat;
layout(location=3) in float ao; // HTML instanceColor scalar
layout(location=4) in vec2 foliageUv;
layout(location=0) out vec4 outColor;
layout(std430,binding=0) readonly buffer State {
 mat4 vp;vec4 eye;vec4 sun;vec4 sky;vec4 cfg;vec4 lamps[16];vec4 atmos;vec4 lighting;int cells[81920];uint natureMask[];
} s; // MaxPlot 64 → 64*64*20
layout(binding=1) uniform sampler2DArray maps; // 0 conc 1 moss 2 ivy 3 brick 4 wood 5 steel … 8+N boardform

// Recipes: style bits 0..4, texture quarter turns 5..6, scale 7..8.
int surfaceBase(int style,int fallback){
 if(style==1)return 0;if(style==2)return 18;if(style==3)return 19;
 if(style>=4&&style<=19)return 7;
 return style==20?5:style==21?8:style==22?38:style==23?39:fallback;
}
vec3 surfacePoint(vec3 point,int recipe){
 int rotation=(recipe>>5)&3;for(int i=0;i<rotation;i++)point=vec3(point.z,point.y,-point.x);
 int scale=(recipe>>7)&3;return point*(scale==0?.5:scale==2?2.:1.);
}
ivec2 brickLayers(int recipe){int style=recipe&31;
 if(style<5)return ivec2(3,39);
 int index=style==5?2:style==6?4:style-1;
 int top=style==5?3:style==6?5:1;
 return ivec2(40+index,40+top);
}
vec4 sampleBrickLod(vec3 point,vec3 normal,int recipe,float lod){
 ivec2 layers=brickLayers(recipe);vec3 weights=pow(abs(normal),vec3(4.));weights/=max(dot(weights,vec3(1.)),1e-5);
 return textureLod(maps,vec3(point.zy*2.,float(layers.x)),lod)*weights.x
       +textureLod(maps,vec3(point.xz*2.,float(layers.y)),lod)*weights.y
       +textureLod(maps,vec3(point.xy*2.,float(layers.x)),lod)*weights.z;
}
vec4 sampleBrick(vec3 point,vec3 normal,int recipe){
 ivec2 layers=brickLayers(recipe);vec3 weights=pow(abs(normal),vec3(4.));weights/=max(dot(weights,vec3(1.)),1e-5);
 return texture(maps,vec3(point.zy*2.,float(layers.x)))*weights.x
       +texture(maps,vec3(point.xz*2.,float(layers.y)))*weights.y
       +texture(maps,vec3(point.xy*2.,float(layers.x)))*weights.z;
}
bool solid(vec3 q);
bool structureSolid(vec3 q);
float structureSunRay(vec3 q,vec3 direction){
 vec3 safe=vec3(abs(direction.x)<1e-5?1e-5:direction.x,abs(direction.y)<1e-5?1e-5:direction.y,abs(direction.z)<1e-5?1e-5:direction.z);
 for(int i=0;i<240;i++){
  if(q.y>20.||q.x<0.||q.z<0.||q.x>=s.cfg.w||q.z>=s.cfg.w)return 1.;
  if(q.y<0.||structureSolid(q))return 0.;
  ivec3 cell=ivec3(floor(q));int value=s.cells[cell.x*1280+cell.z*20+cell.y];
  vec3 boundary=floor(q)+vec3(greaterThan(safe,vec3(0.)));vec3 distances=(boundary-q)/safe;
  float advance=min(distances.x,min(distances.y,distances.z));
  q+=direction*(value==0?max(.006,advance+.003):min(.06,max(.006,advance+.003)));
 }return 0.;
}
#include "foliage_shadow.glsl"
float softShadow(vec3 start,vec3 L){
 // Approximate THREE.PCFSoftShadowMap radius=3
 float sh=0.;
 vec3 U=normalize(cross(L,abs(L.y)>.9?vec3(1,0,0):vec3(0,1,0)));
 vec3 B=cross(U,L);
 const vec2 o[9]=vec2[9](vec2(0),vec2(1,0),vec2(-1,0),vec2(0,1),vec2(0,-1),vec2(.7,.7),vec2(-.7,.7),vec2(.7,-.7),vec2(-.7,-.7));
 for(int j=0;j<9;j++){
  vec3 dir=normalize(L+(U*o[j].x+B*o[j].y)*.04);
  float hit=structureSunRay(start,dir);
  sh+=hit;
 }
 return (sh>0.&&leafShadow(start,L))?0.:sh/9.;
}
#include "structure_occlusion.glsl"

vec3 sampleMap(int layer,vec2 uv){return textureLod(maps,vec3(uv,float(layer)),0.).rgb;}
float sampleA(int layer,vec2 uv){return textureLod(maps,vec3(uv,float(layer)),0.).a;}

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


vec3 fixtureColor(float power){int kind=int(power/100.);return kind==2?vec3(.15,.85,1.):kind==3?vec3(1.,.14,.55):kind==4?vec3(.30,1.,.42):kind==5?vec3(.35,.8,.85):kind==6?vec3(.7,.3,1.):vec3(1.,.851,.627);}
float fixturePower(float power){return mod(power,100.);}
