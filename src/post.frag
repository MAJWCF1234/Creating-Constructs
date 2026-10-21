#version 450
layout(location=0) in vec2 vUv;
layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D tD;
layout(binding=1) uniform sampler2D tZ;
layout(binding=2) readonly buffer Post {
 float t,sat,ao,ex,sunX,sunY,rain,night;
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
 // Tight bloom
 vec3 bl=vec3(0.);
 for(int i=0;i<8;i++){
  float a=float(i)*.785;
  bl+=max(texture(tD,vUv+vec2(cos(a),sin(a))*.0055*vec2(.56,1.)).rgb-1.05,0.);
 }
 c+=bl*.06;

 // Godrays — radial occlusion march toward sun (sky depth = unoccluded shaft)
 if(u.night<.5){
  vec2 sun=vec2(u.sunX,u.sunY);
  float sunVis=smoothstep(1.35,0.15,length(sun-vec2(.5)));
  if(sunVis>0.01&&u.sunY>-0.2&&u.sunY<1.35&&u.sunX>-0.25&&u.sunX<1.25){
   const int SAMPLES=18;
   vec2 delta=(vUv-sun)/float(SAMPLES);
   vec2 uv=vUv;
   float decay=1.,shaft=0.;
   for(int i=0;i<SAMPLES;i++){
    uv-=delta;
    if(uv.x<0.||uv.x>1.||uv.y<0.||uv.y>1.)break;
    float zd=texture(tZ,uv).r;
    float sky=smoothstep(.995,.9998,zd);
    vec3 sCol=texture(tD,uv).rgb;
    float lum=dot(sCol,vec3(.3,.5,.2));
    shaft+=sky*max(lum-0.55,0.)*decay;
    decay*=.92;
   }
   float dens=mix(.011,.006,u.rain);
   vec3 rayCol=mix(vec3(1.,.9,.7),vec3(.75,.82,.9),u.rain);
   c+=rayCol*shaft*dens*sunVis*(1.-u.rain*.35);
  }
 }

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
 float n=fract(sin(dot(gl_FragCoord.xy+fract(u.t)*91.,vec2(12.9898,78.233)))*43758.5453);
 c+=(n-.5)*.018;
 vec2 q=vUv-.5;c*=1.-dot(q,q)*.95;
 outColor=vec4(c,1.);
}
