#version 450
#extension GL_GOOGLE_include_directive : require
#include "style.glsl"
layout(location=0) in vec2 uv;
layout(location=0) out vec4 color;
layout(std430,binding=0) readonly buffer State {
 mat4 vp;vec4 eye;vec4 sun;vec4 sky;vec4 cfg;vec4 lamps[8];vec4 atmos;vec4 lighting;int cells[20480];uint natureMask[];
} s;

float hash21(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float noise2(vec2 p){
 vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);
 float a=hash21(i),b=hash21(i+vec2(1,0)),c=hash21(i+vec2(0,1)),d=hash21(i+vec2(1,1));
 return mix(mix(a,b,f.x),mix(c,d,f.x),f.y);
}
float fbm2(vec2 p){
 float v=0.,a=.5;for(int i=0;i<5;i++){v+=a*noise2(p);p=p*2.13+vec2(17.1,9.3);a*=.5;}
 return v;
}
float hash31(vec3 p){return fract(sin(dot(p,vec3(127.1,311.7,74.7)))*43758.5453);}
float noise3(vec3 p){
 vec3 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);
 float n000=hash31(i),n100=hash31(i+vec3(1,0,0)),n010=hash31(i+vec3(0,1,0)),n110=hash31(i+vec3(1,1,0));
 float n001=hash31(i+vec3(0,0,1)),n101=hash31(i+vec3(1,0,1)),n011=hash31(i+vec3(0,1,1)),n111=hash31(i+vec3(1,1,1));
 return mix(mix(mix(n000,n100,f.x),mix(n010,n110,f.x),f.y),mix(mix(n001,n101,f.x),mix(n011,n111,f.x),f.y),f.z);
}
float fbm3(vec3 p){
 float v=0.,a=.55;for(int i=0;i<4;i++){v+=a*noise3(p);p=p*2.07+vec3(1.7,9.2,3.1);a*=.5;}
 return v;
}

void main(){
 vec4 a=inverse(s.vp)*vec4(uv*2.-1.,1.,1.);
 vec3 vp=normalize(a.xyz/a.w-s.eye.xyz);
 float h=clamp(vp.y,0.,1.);
 float time=s.eye.w;
 float night=s.sky.w;
 float raining=s.lighting.y;
 vec3 sunD=normalize(s.sun.xyz);
 vec3 top=s.atmos.rgb,hor=s.sky.rgb;

 // Base sky gradient + soft 2D cloud film
 float cl=0.,am=.5;vec2 q=vp.xz/(vp.y+.35)*1.5;
 for(int i=0;i<4;i++){
  vec2 ip=floor(q);vec2 f=fract(q);f=f*f*(3.-2.*f);
  float a0=fract(sin(dot(ip,vec2(12.9,78.2)))*43758.5);
  float b=fract(sin(dot(ip+vec2(1.,0.),vec2(12.9,78.2)))*43758.5);
  float c2=fract(sin(dot(ip+vec2(0.,1.),vec2(12.9,78.2)))*43758.5);
  float d=fract(sin(dot(ip+vec2(1.,1.),vec2(12.9,78.2)))*43758.5);
  cl+=mix(mix(a0,b,f.x),mix(c2,d,f.x),f.y)*am;am*=.5;q*=2.1;
 }
 vec3 c=mix(hor,top,pow(h,.5));
 c*=1.+(cl-.5)*.22*smoothstep(0.,.25,vp.y);

 // Volumetric cloud slab (cheap raymarch through altitude band)
 if(vp.y>.02){
  float cloudH=140.+raining*40.;
  float thick=55.+raining*35.;
  float tEnter=cloudH/max(vp.y,.02);
  float tExit=(cloudH+thick)/max(vp.y,.02);
  tEnter=clamp(tEnter,0.,900.);tExit=clamp(tExit,tEnter,950.);
  float steps=mix(10.,14.,1.-night);
  float densAccum=0.,lightAccum=0.,dt=(tExit-tEnter)/steps;
  vec3 wind=vec3(time*.018,0.,time*.011);
  float cover=mix(.42,.72,raining)*(1.-night*.75);
  for(int i=0;i<14;i++){
   if(float(i)>=steps)break;
   float t=tEnter+(float(i)+.5)*dt;
   vec3 p=vp*t+wind*40.;
   float n=fbm3(p*.0075);
   float shape=smoothstep(1.-cover,1.05-cover*.35,n+fbm2(p.xz*.004)*.15);
   float h01=clamp((t*vp.y-cloudH)/thick,0.,1.);
   shape*=smoothstep(0.,.18,h01)*smoothstep(1.,.55,h01);
   densAccum+=shape*(1.-densAccum)*.22;
   float sunOcclusion=smoothstep(.2,.9,n);
   lightAccum+=shape*exp(-sunOcclusion*2.2)*(1.-densAccum);
  }
  vec3 sunCol=mix(vec3(1.,.86,.68),vec3(.55,.62,.72),raining);
  vec3 cloudLit=mix(hor*.55,sunCol,clamp(lightAccum*1.6,0.,1.));
  cloudLit=mix(cloudLit,top*.7,.25);
  float alpha=clamp(densAccum*mix(1.15,1.45,raining),0.,.92)*(1.-night*.55);
  alpha*=smoothstep(.02,.18,vp.y);
  c=mix(c,cloudLit,alpha);
  // Silver lining toward sun
  float sdCloud=max(dot(vp,sunD),0.);
  c+=sunCol*pow(sdCloud,6.)*densAccum*.35*(1.-night)*(1.-raining*.4);
 }

 // Sun / moon disc
 float sd=max(dot(vp,sunD),0.);
 if(night<.5)c+=vec3(1.,.82,.55)*(pow(sd,900.)*2.4+pow(sd,14.)*.18+pow(sd,3.)*.04);
 else c+=vec3(.75,.82,1.)*(pow(sd,600.)*1.2+pow(sd,8.)*.08);

 if(vp.y<0.)c=hor*.85;

 // Multi-layer twinkling stars
 if(night>.5&&vp.y>.01){
  for(int layer=0;layer<3;layer++){
   float scale=mix(520.,1180.,float(layer)/2.);
   vec2 sp=floor(vp.xz/(vp.y+.18)*scale);
   float rnd=hash21(sp+float(layer)*19.7);
   float thresh=mix(.9968,.9988,float(layer)/2.);
   if(rnd>thresh){
    float tw=.65+.35*sin(time*(1.7+rnd*3.)+rnd*40.);
    float sz=mix(.55,1.4,fract(rnd*17.));
    vec3 tint=mix(vec3(.85,.9,1.),vec3(1.,.92,.78),fract(rnd*9.));
    float fall=smoothstep(.01,.22,vp.y);
    c+=tint*tw*sz*fall*mix(.55,1.1,float(layer)/2.);
   }
  }
  // faint milky haze
  c+=vec3(.55,.62,.85)*pow(max(vp.y,0.),1.6)*.04;
 }

 if(s.lighting.z>0.)c=mix(c,vec3(.018,.055,.051),.9);
 color=vec4(c,1.);
}
