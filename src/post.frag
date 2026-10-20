#version 450
// Literally monolith.html PQ fragmentShader lines 144-160
layout(location=0) in vec2 vUv;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D tD;
layout(binding=1) uniform sampler2D tZ;
layout(binding=2) readonly buffer Post {
 float t,sat,ao,ex;
} u;
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
 // Tight bloom — wide radius reads as noon ghosting on hot concrete/sky
 vec3 bl=vec3(0.);
 for(int i=0;i<8;i++){
  float a=float(i)*.785;
  bl+=max(texture(tD,vUv+vec2(cos(a),sin(a))*.0055*vec2(.56,1.)).rgb-1.05,0.);
 }
 c+=bl*.06;
 c=(c*(2.51*c+.03))/(c*(2.43*c+.59)+.14);
 float l=dot(c,vec3(.299,.587,.114));
 float gr=clamp((c.g-max(c.r,c.b))*16.,0.,1.),wm=clamp((c.r-c.b)*4.-.25,0.,1.);
 c=mix(vec3(l),c*1.25,max(max(gr,wm*.8),u.sat));
 c=pow(max(c,0.),vec3(.95))*vec3(.97,1.,1.03);
 float n=fract(sin(dot(gl_FragCoord.xy+fract(u.t)*91.,vec2(12.9898,78.233)))*43758.5453);
 c+=(n-.5)*.018;
 vec2 q=vUv-.5;c*=1.-dot(q,q)*.95;
 outColor=vec4(c,1.);
}
