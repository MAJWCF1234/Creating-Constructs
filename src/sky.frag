#version 450
#extension GL_GOOGLE_include_directive : require
#include "style.glsl"
layout(location=0) in vec2 uv;
layout(location=0) out vec4 color;
// State layout must match C++ struct State
layout(std430,binding=0) readonly buffer State {
 mat4 vp;vec4 eye;vec4 sun;vec4 sky;vec4 cfg;vec4 lamps[8];vec4 atmos;vec4 lighting;int cells[20480];uint natureMask[];
} s;
// Literally monolith.html dome fragmentShader (line 98), vp = view direction
void main(){
 vec4 a=inverse(s.vp)*vec4(uv*2.-1.,1.,1.);
 vec3 vp=normalize(a.xyz/a.w-s.eye.xyz);
 float h=clamp(vp.y,0.,1.);
 float cl=0.,am=.5;
 vec2 q=vp.xz/(vp.y+.35)*1.5;
 for(int i=0;i<4;i++){
  vec2 ip=floor(q);vec2 f=fract(q);f=f*f*(3.-2.*f);
  float a0=fract(sin(dot(ip,vec2(12.9,78.2)))*43758.5);
  float b=fract(sin(dot(ip+vec2(1.,0.),vec2(12.9,78.2)))*43758.5);
  float c2=fract(sin(dot(ip+vec2(0.,1.),vec2(12.9,78.2)))*43758.5);
  float d=fract(sin(dot(ip+vec2(1.,1.),vec2(12.9,78.2)))*43758.5);
  cl+=mix(mix(a0,b,f.x),mix(c2,d,f.x),f.y)*am;am*=.5;q*=2.1;
 }
 vec3 top=s.atmos.rgb,hor=s.sky.rgb;
 vec3 c=mix(hor,top,pow(h,.5));
 c*=1.+(cl-.5)*.35*smoothstep(0.,.3,vp.y);
 float sd=max(dot(vp,normalize(s.sun.xyz)),0.);
 c+=vec3(1.,.8,.55)*(pow(sd,800.)*2.+pow(sd,12.)*.12)*(1.-s.sky.w); // HTML sunI: 0 night else 1
 if(vp.y<0.)c=hor*.85;
 // stars when night (sky.w)
 if(s.sky.w>.5&&vp.y>.02){
  float st=fract(sin(dot(floor(vp.xz/(vp.y+.2)*700.),vec2(12.9898,78.233)))*43758.5453);
  c+=step(.9975,st)*vec3(1.);
 }
 if(s.lighting.z>0.)c=mix(c,vec3(.018,.055,.051),.9);
 color=vec4(c,1.); // linear — post.frag applies HTML finish
}
