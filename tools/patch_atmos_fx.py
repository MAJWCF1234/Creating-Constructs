"""Patch Creating Constructs.html: volumetric clouds, godrays, stars, rain."""
from __future__ import annotations
import re
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

DOME = r"""const dome=new THREE.Mesh(new THREE.SphereGeometry(250,32,16),new THREE.ShaderMaterial({side:THREE.BackSide,depthWrite:false,fog:false,uniforms:{top:{value:new THREE.Color()},hor:{value:new THREE.Color()},sd:{value:new THREE.Vector3(0,1,0)},sunI:{value:1},time:{value:0},night:{value:0},rain:{value:0}},
vertexShader:'varying vec3 vp;void main(){vp=normalize(position);gl_Position=projectionMatrix*modelViewMatrix*vec4(position,1.);}',
fragmentShader:`uniform vec3 top,hor,sd;uniform float sunI,time,night,rain;varying vec3 vp;
float hash21(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
float noise2(vec2 p){vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);return mix(mix(hash21(i),hash21(i+vec2(1,0)),f.x),mix(hash21(i+vec2(0,1)),hash21(i+vec2(1,1)),f.x),f.y);}
float fbm2(vec2 p){float v=0.,a=.5;for(int i=0;i<5;i++){v+=a*noise2(p);p=p*2.13+vec2(17.1,9.3);a*=.5;}return v;}
float hash31(vec3 p){return fract(sin(dot(p,vec3(127.1,311.7,74.7)))*43758.5453);}
float noise3(vec3 p){vec3 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);
 float n000=hash31(i),n100=hash31(i+vec3(1,0,0)),n010=hash31(i+vec3(0,1,0)),n110=hash31(i+vec3(1,1,0));
 float n001=hash31(i+vec3(0,0,1)),n101=hash31(i+vec3(1,0,1)),n011=hash31(i+vec3(0,1,1)),n111=hash31(i+vec3(1,1,1));
 return mix(mix(mix(n000,n100,f.x),mix(n010,n110,f.x),f.y),mix(mix(n001,n101,f.x),mix(n011,n111,f.x),f.y),f.z);}
float fbm3(vec3 p){float v=0.,a=.55;for(int i=0;i<4;i++){v+=a*noise3(p);p=p*2.07+vec3(1.7,9.2,3.1);a*=.5;}return v;}
void main(){
 float h=clamp(vp.y,0.,1.);float cl=0.,am=.5;vec2 q=vp.xz/(vp.y+.35)*1.5;
 for(int i=0;i<4;i++){vec2 ip=floor(q);vec2 f=fract(q);f=f*f*(3.-2.*f);
  float a=fract(sin(dot(ip,vec2(12.9,78.2)))*43758.5),b=fract(sin(dot(ip+vec2(1.,0.),vec2(12.9,78.2)))*43758.5),c2=fract(sin(dot(ip+vec2(0.,1.),vec2(12.9,78.2)))*43758.5),d=fract(sin(dot(ip+vec2(1.,1.),vec2(12.9,78.2)))*43758.5);
  cl+=mix(mix(a,b,f.x),mix(c2,d,f.x),f.y)*am;am*=.5;q*=2.1;}
 vec3 c=mix(hor,top,pow(h,.5));c*=1.+(cl-.5)*.22*smoothstep(0.,.25,vp.y);
 if(vp.y>.02){
  float cloudH=140.+rain*40.,thick=55.+rain*35.,tEnter=cloudH/max(vp.y,.02),tExit=(cloudH+thick)/max(vp.y,.02);
  tEnter=clamp(tEnter,0.,900.);tExit=clamp(tExit,tEnter,950.);
  float dens=0.,lit=0.,dt=(tExit-tEnter)/12.,cover=mix(.42,.72,rain)*(1.-night*.75);
  vec3 wind=vec3(time*.018,0.,time*.011);
  for(int i=0;i<12;i++){float t=tEnter+(float(i)+.5)*dt;vec3 p=vp*t+wind*40.;
   float n=fbm3(p*.0075);float shape=smoothstep(1.-cover,1.05-cover*.35,n+fbm2(p.xz*.004)*.15);
   float h01=clamp((t*vp.y-cloudH)/thick,0.,1.);shape*=smoothstep(0.,.18,h01)*smoothstep(1.,.55,h01);
   dens+=shape*(1.-dens)*.22;lit+=shape*exp(-smoothstep(.2,.9,n)*2.2)*(1.-dens);}
  vec3 sunCol=mix(vec3(1.,.86,.68),vec3(.55,.62,.72),rain);
  vec3 cloudLit=mix(hor*.55,sunCol,clamp(lit*1.6,0.,1.));cloudLit=mix(cloudLit,top*.7,.25);
  float alpha=clamp(dens*mix(1.15,1.45,rain),0.,.92)*(1.-night*.55)*smoothstep(.02,.18,vp.y);
  c=mix(c,cloudLit,alpha);float sdC=max(dot(vp,sd),0.);c+=sunCol*pow(sdC,6.)*dens*.35*sunI*(1.-rain*.4);
 }
 float s=max(dot(vp,sd),0.);
 if(night<.5)c+=vec3(1.,.82,.55)*(pow(s,900.)*2.4+pow(s,14.)*.18+pow(s,3.)*.04)*sunI;
 else c+=vec3(.75,.82,1.)*(pow(s,600.)*1.2+pow(s,8.)*.08);
 if(vp.y<0.)c=hor*.85;
 if(night>.5&&vp.y>.01){
  for(int layer=0;layer<3;layer++){float scale=mix(520.,1180.,float(layer)/2.);vec2 sp=floor(vp.xz/(vp.y+.18)*scale);
   float rnd=hash21(sp+float(layer)*19.7);float thresh=mix(.9968,.9988,float(layer)/2.);
   if(rnd>thresh){float tw=.65+.35*sin(time*(1.7+rnd*3.)+rnd*40.);float sz=mix(.55,1.4,fract(rnd*17.));
    vec3 tint=mix(vec3(.85,.9,1.),vec3(1.,.92,.78),fract(rnd*9.));c+=tint*tw*sz*smoothstep(.01,.22,vp.y)*mix(.55,1.1,float(layer)/2.);}}
  c+=vec3(.55,.62,.85)*pow(max(vp.y,0.),1.6)*.04;
 }
 gl_FragColor=vec4(c,1.);}`}));dome.renderOrder=-1;S.add(dome);
"""

# Replace dome block through S.add(dome);
d0 = s.find("const dome=new THREE.Mesh(new THREE.SphereGeometry(250,32,16)")
d1 = s.find("S.add(dome);", d0)
if d0 < 0 or d1 < 0:
    raise SystemExit("dome block not found")
s = s[:d0] + DOME + s[d1 + len("S.add(dome);") :]

# Stars: denser field + twinkle shader (keep Points as secondary sparkle layer)
old_stars = """const sp=[];for(let i=0;i<900;i++){const a=Math.random()*6.283,b=Math.random()*1.5+.05;sp.push(Math.cos(a)*Math.cos(b)*240,Math.sin(b)*240,Math.sin(a)*Math.cos(b)*240)}
const stars=new THREE.Points(new THREE.BufferGeometry().setAttribute('position',new THREE.Float32BufferAttribute(sp,3)),new THREE.PointsMaterial({size:1.8,sizeAttenuation:false,fog:false,color:0xffffff}));stars.visible=false;S.add(stars);"""
new_stars = """const sp=[],sc=[];for(let i=0;i<2200;i++){const a=Math.random()*6.283,b=Math.pow(Math.random(),.55)*1.45+.04;sp.push(Math.cos(a)*Math.cos(b)*240,Math.sin(b)*240,Math.sin(a)*Math.cos(b)*240);sc.push(.75+Math.random()*.35,.8+Math.random()*.25,.9+Math.random()*.1)}
const starGeo=new THREE.BufferGeometry();starGeo.setAttribute('position',new THREE.Float32BufferAttribute(sp,3));starGeo.setAttribute('color',new THREE.Float32BufferAttribute(sc,3));
const stars=new THREE.Points(starGeo,new THREE.ShaderMaterial({fog:false,transparent:true,depthWrite:false,vertexColors:true,uniforms:{t:{value:0}},
vertexShader:'uniform float t;varying vec3 vc;varying float tw;void main(){vc=color;tw=.55+.45*sin(t*1.9+position.x*.02+position.z*.015);vec4 mv=modelViewMatrix*vec4(position,1.);gl_PointSize=(1.2+fract(sin(dot(position.xy,vec2(12.9,78.2)))*43758.5)*2.2)*tw;gl_Position=projectionMatrix*mv;}',
fragmentShader:'varying vec3 vc;varying float tw;void main(){vec2 p=gl_PointCoord*2.-1.;float d=dot(p,p);if(d>1.)discard;float a=(1.-smoothstep(.05,.95,d))*tw;gl_FragColor=vec4(vc,a*.95);}'
}));stars.visible=false;S.add(stars);"""
if old_stars not in s:
    raise SystemExit("stars block not found")
s = s.replace(old_stars, new_stars, 1)

# Post PQ with godrays + rain mist
old_pq = """const PS=new THREE.Scene(),PC=new THREE.OrthographicCamera(-1,1,1,-1,0,1),PQ=new THREE.ShaderMaterial({uniforms:{tD:{value:RT.texture},tZ:{value:RT.depthTexture},t:{value:0},sat:{value:0},ao:{value:1},ex:{value:1.12}},depthTest:false,
vertexShader:'varying vec2 vUv;void main(){vUv=uv;gl_Position=vec4(position.xy,0.,1.);}',
fragmentShader:`uniform sampler2D tD,tZ;uniform float t,sat,ao,ex;varying vec2 vUv;float lz(float d){float z=d*2.-1.;return 2.*.1*300./(300.1-z*299.9);}
void main(){vec3 c=texture2D(tD,vUv).rgb*ex;
float n0=fract(sin(dot(gl_FragCoord.xy,vec2(12.9898,78.233)))*43758.5453);
float d0=texture2D(tZ,vUv).r;
if(d0<.99999&&ao>.5){float z0=lz(d0),rd=clamp(.42/z0,.0025,.04),oc=0.;
for(int i=0;i<16;i++){float a=float(i)*2.399+n0*6.283,r=rd*sqrt((float(i)+.5)/16.);float zs=lz(texture2D(tZ,vUv+vec2(cos(a),sin(a))*r*vec2(.56,1.)).r);float df=z0-zs;oc+=step(.04,df)*smoothstep(1.5,.2,df);}
c*=clamp(1.-oc/16.*1.15,.35,1.);}
vec3 bl=vec3(0.);for(int i=0;i<8;i++){float a=float(i)*.785;bl+=max(texture2D(tD,vUv+vec2(cos(a),sin(a))*.0055*vec2(.56,1.)).rgb-1.05,0.);}c+=bl*.06;
c=(c*(2.51*c+.03))/(c*(2.43*c+.59)+.14);
float l=dot(c,vec3(.299,.587,.114));
float gr=clamp((c.g-max(c.r,c.b))*16.,0.,1.),wm=clamp((c.r-c.b)*4.-.25,0.,1.);
c=mix(vec3(l),c*1.25,max(max(gr,wm*.8),sat));
c=pow(max(c,0.),vec3(.95))*vec3(.97,1.,1.03);
float n=fract(sin(dot(gl_FragCoord.xy+fract(t)*91.,vec2(12.9898,78.233)))*43758.5453);
c+=(n-.5)*.018;
vec2 q=vUv-.5;c*=1.-dot(q,q)*.95;
gl_FragColor=vec4(c,1.);}`});PS.add(new THREE.Mesh(new THREE.PlaneGeometry(2,2),PQ));"""

new_pq = """const PS=new THREE.Scene(),PC=new THREE.OrthographicCamera(-1,1,1,-1,0,1),PQ=new THREE.ShaderMaterial({uniforms:{tD:{value:RT.texture},tZ:{value:RT.depthTexture},t:{value:0},sat:{value:0},ao:{value:1},ex:{value:1.12},sun:{value:new THREE.Vector2(.5,.5)},rain:{value:0},night:{value:0}},depthTest:false,
vertexShader:'varying vec2 vUv;void main(){vUv=uv;gl_Position=vec4(position.xy,0.,1.);}',
fragmentShader:`uniform sampler2D tD,tZ;uniform float t,sat,ao,ex,rain,night;uniform vec2 sun;varying vec2 vUv;float lz(float d){float z=d*2.-1.;return 2.*.1*300./(300.1-z*299.9);}
void main(){vec3 c=texture2D(tD,vUv).rgb*ex;
float n0=fract(sin(dot(gl_FragCoord.xy,vec2(12.9898,78.233)))*43758.5453);
float d0=texture2D(tZ,vUv).r;
if(d0<.99999&&ao>.5){float z0=lz(d0),rd=clamp(.42/z0,.0025,.04),oc=0.;
for(int i=0;i<16;i++){float a=float(i)*2.399+n0*6.283,r=rd*sqrt((float(i)+.5)/16.);float zs=lz(texture2D(tZ,vUv+vec2(cos(a),sin(a))*r*vec2(.56,1.)).r);float df=z0-zs;oc+=step(.04,df)*smoothstep(1.5,.2,df);}
c*=clamp(1.-oc/16.*1.15,.35,1.);}
vec3 bl=vec3(0.);for(int i=0;i<8;i++){float a=float(i)*.785;bl+=max(texture2D(tD,vUv+vec2(cos(a),sin(a))*.0055*vec2(.56,1.)).rgb-1.05,0.);}c+=bl*.06;
if(night<.5){float sunVis=smoothstep(1.35,.15,length(sun-vec2(.5)));
 if(sunVis>.01&&sun.y>-.2&&sun.y<1.35&&sun.x>-.25&&sun.x<1.25){vec2 delta=(vUv-sun)/18.;vec2 uv=vUv;float decay=1.,shaft=0.;
  for(int i=0;i<18;i++){uv-=delta;if(uv.x<0.||uv.x>1.||uv.y<0.||uv.y>1.)break;float zd=texture2D(tZ,uv).r;float sky=smoothstep(.995,.9998,zd);float lum=dot(texture2D(tD,uv).rgb,vec3(.3,.5,.2));shaft+=sky*max(lum-.55,0.)*decay;decay*=.92;}
  vec3 rayCol=mix(vec3(1.,.9,.7),vec3(.75,.82,.9),rain);c+=rayCol*shaft*mix(.011,.006,rain)*sunVis*(1.-rain*.35);}}
if(rain>.5){vec2 rs=vUv*vec2(90.,42.);float streak=0.;
 for(int i=0;i<3;i++){float spd=18.+float(i)*7.;vec2 p=rs+vec2(float(i)*17.1,-t*spd+float(i)*5.);float id=floor(p.x);float n=fract(sin(id*127.1+float(i)*91.7)*43758.5);float x=fract(p.x)-.5,y=fract(p.y+n);
  streak+=smoothstep(.035,0.,abs(x-(n-.5)*.2))*smoothstep(0.,.15,y)*smoothstep(1.,.55,y)*(.35+n*.65);}
 c=mix(c,c*vec3(.88,.91,.95)+vec3(.04,.05,.06),clamp(streak*.22,0.,.4));}
c=(c*(2.51*c+.03))/(c*(2.43*c+.59)+.14);
float l=dot(c,vec3(.299,.587,.114));
float gr=clamp((c.g-max(c.r,c.b))*16.,0.,1.),wm=clamp((c.r-c.b)*4.-.25,0.,1.);
c=mix(vec3(l),c*1.25,max(max(gr,wm*.8),sat));
c=pow(max(c,0.),vec3(.95))*vec3(.97,1.,1.03);
float n=fract(sin(dot(gl_FragCoord.xy+fract(t)*91.,vec2(12.9898,78.233)))*43758.5453);
c+=(n-.5)*.018;
vec2 q=vUv-.5;c*=1.-dot(q,q)*.95;
gl_FragColor=vec4(c,1.);}`});PS.add(new THREE.Mesh(new THREE.PlaneGeometry(2,2),PQ));"""
if old_pq not in s:
    raise SystemExit("PQ block not found")
s = s.replace(old_pq, new_pq, 1)

# Rain streaks
old_rain = """let rain=false;const rp=[];for(let i=0;i<3500;i++){const x=Math.random()*60-20,y=Math.random()*25,z=Math.random()*60-20;rp.push(x,y,z,x-.1,y+.5,z)}
const RM=new THREE.ShaderMaterial({transparent:true,depthWrite:false,fog:false,uniforms:{t:{value:0}},vertexShader:'uniform float t;void main(){vec3 p=position;p.y=mod(p.y-t*26.,25.);gl_Position=projectionMatrix*modelViewMatrix*vec4(p,1.);}',fragmentShader:'void main(){gl_FragColor=vec4(.75,.8,.85,.35);}'});
const rainL=new THREE.LineSegments(new THREE.BufferGeometry().setAttribute('position',new THREE.Float32BufferAttribute(rp,3)),RM);rainL.frustumCulled=false;rainL.visible=false;S.add(rainL);"""
new_rain = """let rain=false;const rp=[],rspeed=[];for(let i=0;i<7000;i++){const x=Math.random()*70-30,y=Math.random()*32,z=Math.random()*70-30,len=.35+Math.random()*.45,wind=.1+Math.random()*.1;rp.push(x,y,z,x-wind,y+len,z+.02);rspeed.push(16+Math.random()*10)}
const RM=new THREE.ShaderMaterial({transparent:true,depthWrite:false,fog:false,uniforms:{t:{value:0},cam:{value:new THREE.Vector3()}},
vertexShader:'uniform float t;uniform vec3 cam;attribute float spd;void main(){vec3 p=position;float y=mod(p.y-t*spd+cam.y,32.);p.x=mod(p.x-cam.x+35.,70.)-35.+cam.x;p.z=mod(p.z-cam.z+35.,70.)-35.+cam.z;p.y=y+cam.y-12.;gl_Position=projectionMatrix*modelViewMatrix*vec4(p,1.);}',
fragmentShader:'void main(){gl_FragColor=vec4(.68,.76,.88,.28);}'});
const rainGeo=new THREE.BufferGeometry();rainGeo.setAttribute('position',new THREE.Float32BufferAttribute(rp,3));rainGeo.setAttribute('spd',new THREE.Float32BufferAttribute(rspeed.concat(rspeed),1));
const rainL=new THREE.LineSegments(rainGeo,RM);rainL.frustumCulled=false;rainL.visible=false;S.add(rainL);"""
# Note: LineSegments uses pairs - spd attribute needs one value per vertex. Fix: duplicate speeds for both ends of each segment.
# Actually rspeed has 7000 values for 7000 segments = 14000 vertices. Need spd per vertex.
new_rain = """let rain=false;const rp=[],rspeed=[];for(let i=0;i<7000;i++){const x=Math.random()*70-30,y=Math.random()*32,z=Math.random()*70-30,len=.35+Math.random()*.45,wind=.1+Math.random()*.1,spd=16+Math.random()*10;rp.push(x,y,z,x-wind,y+len,z+.02);rspeed.push(spd,spd)}
const RM=new THREE.ShaderMaterial({transparent:true,depthWrite:false,fog:false,uniforms:{t:{value:0},cam:{value:new THREE.Vector3()}},
vertexShader:'uniform float t;uniform vec3 cam;attribute float spd;void main(){vec3 p=position;float y=mod(p.y-t*spd+cam.y,32.);p.x=mod(p.x-cam.x+35.,70.)-35.+cam.x;p.z=mod(p.z-cam.z+35.,70.)-35.+cam.z;p.y=y+cam.y-12.;gl_Position=projectionMatrix*modelViewMatrix*vec4(p,1.);}',
fragmentShader:'void main(){gl_FragColor=vec4(.68,.76,.88,.28);}'});
const rainGeo=new THREE.BufferGeometry();rainGeo.setAttribute('position',new THREE.Float32BufferAttribute(rp,3));rainGeo.setAttribute('spd',new THREE.Float32BufferAttribute(rspeed,1));
const rainL=new THREE.LineSegments(rainGeo,RM);rainL.frustumCulled=false;rainL.visible=false;S.add(rainL);"""
if old_rain not in s:
    raise SystemExit("rain block not found")
s = s.replace(old_rain, new_rain, 1)

# Update sun() to drive new uniforms
old_sun_tail = """U.top.value.set(night?0x03050c:M.top);U.hor.value.copy(hor);U.sd.value.copy(sunL.position).sub(t).normalize();U.sunI.value=night?0:1;stars.visible=night;
S.fog=new THREE.FogExp2(hor.getHex(),(rain?.012:0)+f*f*.09+.003);
sunL.intensity=night?0:M.si*Math.min(1,el*2.4)*(rain?.55:1)*1.08;sunL.color.set(low>.4?0xffb37a:M.sun);moon.intensity=night?.9:0;fill.intensity=night?0:.52;
hemi.intensity=night?.28:M.hi*1.25*1.15;hemi.color.set(night?0x4a5a90:0xd0d4d6);hemi.groundColor.set(night?0x14161e:0x7a756b);glass.emissiveIntensity=night?.5:0;PQ.uniforms.sat.value=night?.1:(rain?M.sat*.4:M.sat);PQ.uniforms.ex.value=$('ex').value/100;rainL.visible=rain;lightsSync()}"""
new_sun_tail = """U.top.value.set(night?0x03050c:M.top);U.hor.value.copy(hor);U.sd.value.copy(sunL.position).sub(t).normalize();U.sunI.value=night?0:1;U.night.value=night?1:0;U.rain.value=rain?1:0;stars.visible=night;
S.fog=new THREE.FogExp2(hor.getHex(),(rain?.012:0)+f*f*.09+.003);
sunL.intensity=night?0:M.si*Math.min(1,el*2.4)*(rain?.55:1)*1.08;sunL.color.set(low>.4?0xffb37a:M.sun);moon.intensity=night?.9:0;fill.intensity=night?0:.52;
hemi.intensity=night?.28:M.hi*1.25*1.15;hemi.color.set(night?0x4a5a90:0xd0d4d6);hemi.groundColor.set(night?0x14161e:0x7a756b);glass.emissiveIntensity=night?.5:0;PQ.uniforms.sat.value=night?.1:(rain?M.sat*.4:M.sat);PQ.uniforms.ex.value=$('ex').value/100;PQ.uniforms.rain.value=rain?1:0;PQ.uniforms.night.value=night?1:0;rainL.visible=rain;lightsSync()}"""
if old_sun_tail not in s:
    raise SystemExit("sun() tail not found")
s = s.replace(old_sun_tail, new_sun_tail, 1)

# Animate: update time uniforms + sun screen + rain cam
old_anim = "cabUpdate(clk.elapsedTime);R.setRenderTarget(RT);R.render(S,cam);R.setRenderTarget(null);PQ.uniforms.t.value=RM.uniforms.t.value=clk.elapsedTime;R.render(PS,PC)})();"
new_anim = """cabUpdate(clk.elapsedTime);const et=clk.elapsedTime;dome.material.uniforms.time.value=et;if(stars.material.uniforms)stars.material.uniforms.t.value=et;RM.uniforms.t.value=et;RM.uniforms.cam.value.copy(cam.position);
const sunWorld=sunL.position.clone(),sp=sunWorld.project(cam);PQ.uniforms.sun.value.set(sp.x*.5+.5,sp.y*.5+.5);if(sp.z>1)PQ.uniforms.sun.value.set(-10,-10);
R.setRenderTarget(RT);R.render(S,cam);R.setRenderTarget(null);PQ.uniforms.t.value=et;R.render(PS,PC)})();"""
if old_anim not in s:
    raise SystemExit("animate tail not found")
s = s.replace(old_anim, new_anim, 1)

HTML.write_text(s, encoding="utf-8")
print("patched atmos FX into Creating Constructs.html")
