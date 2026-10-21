#version 450
#extension GL_GOOGLE_include_directive : require
layout(location=0) in vec2 vUv;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D tD;
layout(binding=1) uniform sampler2D tZ;
layout(binding=2) readonly buffer Post {
 float t,sat,ao,ex,sunX,sunY,rain,night,volumetrics;
} u;
layout(std430,binding=3) readonly buffer SceneState {
 mat4 vp;vec4 eye;vec4 sun;vec4 sky;vec4 cfg;vec4 lamps[16];vec4 atmos;vec4 lighting;int cells[81920];uint natureMask[];
} s;
#include "structure_occlusion.glsl"
float sunVisibility(vec3 q,vec3 L){
 vec3 safeL=vec3(abs(L.x)<.0001?.0001:L.x,abs(L.y)<.0001?.0001:L.y,abs(L.z)<.0001?.0001:L.z);
 for(int i=0;i<40;i++){
  if(q.y>20.||q.x<0.||q.z<0.||q.x>=s.cfg.w||q.z>=s.cfg.w)return 1.;
  if(q.y<0.||solid(q))return 0.;
  vec3 boundary=floor(q)+vec3(greaterThan(safeL,vec3(0.)));
  vec3 t=(boundary-q)/safeL;float advance=max(.012,min(t.x,min(t.y,t.z)));
  if(structureSolid(q+L*min(.3,advance*.5)))return 0.;
  q+=L*(advance+.008);
 }return 0.;
}
vec3 volumeLight(vec2 uv,float depth,float jitter){
 if(u.volumetrics<.5||u.night>.5||s.sun.y<.02||s.sun.w<.01)return vec3(0.);
 vec4 world=inverse(s.vp)*vec4(uv*2.-1.,min(depth,.99995),1.);
 vec3 delta=world.xyz/world.w-s.eye.xyz;float distance=min(length(delta),32.);vec3 ray=normalize(delta);
 float density=min(s.cfg.y,.06),stepSize=distance/16.,energy=0.;
 for(int i=0;i<16;i++){
  float d=(float(i)+.15+jitter*.7)*stepSize;vec3 q=s.eye.xyz+ray*d;
  energy+=sunVisibility(q,normalize(s.sun.xyz))*exp(-d*density)*density*stepSize;
 }
 float phase=.65+.65*pow(max(dot(ray,normalize(s.sun.xyz)),0.),4.);
 vec3 tint=mix(vec3(1.,.97,.90),vec3(1.,.73,.43),clamp(1.-s.sun.y*1.6,0.,1.));
 return tint*energy*s.sun.w*phase*.34*(1.-u.rain*.5);
}
float lz(float d){float z=d*2.-1.;return 2.*.1*300./(300.1-z*299.9);}
void main(){
 vec3 c=texture(tD,vUv).rgb*u.ex;
 float n0=fract(sin(dot(gl_FragCoord.xy,vec2(12.9898,78.233)))*43758.5453);
 float d0=texture(tZ,vUv).r;
 if(d0<.99999&&u.ao>.5){
  float z0=lz(d0),rd=clamp(.42/z0,.0025,.04),oc=0.;
  for(int i=0;i<16;i++){
   float a=float(i)*2.399+n0*6.283,r=rd*sqrt((float(i)+.5)/16.);
   float zs=lz(texture(tZ,vUv+vec2(cos(a),sin(a))*r*vec2(.56,1.)).r);
   float df=z0-zs;oc+=step(.04,df)*smoothstep(1.5,.2,df);
  }
  c*=clamp(1.-oc/16.*1.15,.35,1.);
 }
 // Tight bloom
 vec3 bl=vec3(0.);
 for(int i=0;i<8;i++){
  float a=float(i)*.785;
  bl+=max(texture(tD,vUv+vec2(cos(a),sin(a))*.0055*vec2(.56,1.)).rgb-1.05,0.);
 }
 c+=bl*.06;

 // Geometry-occluded scattering works when the sun is outside the screen.
 c+=volumeLight(vUv,d0,n0)*u.ex;

 // Rain streak overlay in screen space (fine mist sheets)
 if(u.rain>.5){
  vec2 rs=vUv*vec2(90.,42.);
  float streak=0.;
  for(int i=0;i<3;i++){
   float spd=18.+float(i)*7.;
   vec2 p=rs+vec2(float(i)*17.1, -u.t*spd+float(i)*5.);
   float id=floor(p.x);
   float n=fract(sin(id*127.1+float(i)*91.7)*43758.5);
   float x=fract(p.x)-.5;
   float y=fract(p.y+n);
   float drop=smoothstep(.035,0.,abs(x-(n-.5)*.2))*smoothstep(0.,.15,y)*smoothstep(1.,.55,y);
   streak+=drop*(.35+n*.65);
  }
  c=mix(c,c*vec3(.88,.91,.95)+vec3(.04,.05,.06),clamp(streak*.22,0.,.4));
 }

 c=(c*(2.51*c+.03))/(c*(2.43*c+.59)+.14);
 float l=dot(c,vec3(.299,.587,.114));
 float gr=clamp((c.g-max(c.r,c.b))*16.,0.,1.),wm=clamp((c.r-c.b)*4.-.25,0.,1.);
 c=mix(vec3(l),c*1.25,max(max(gr,wm*.8),u.sat));
 c=pow(max(c,0.),vec3(.95))*vec3(.97,1.,1.03);
 float n=fract(sin(dot(gl_FragCoord.xy,vec2(12.9898,78.233)))*43758.5453);
 c+=(n-.5)*.018;
 vec2 q=vUv-.5;c*=1.-dot(q,q)*.95;
 outColor=vec4(c,1.);
}
