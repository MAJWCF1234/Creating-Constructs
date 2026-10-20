#version 450
#extension GL_GOOGLE_include_directive : require
#include "scene_shared.glsl"
void main(){
 if(mat==9){outColor=vec4(0);return;}
 vec3 N=normalize(n),V=normalize(s.eye.xyz-p),L=normalize(s.sun.xyz);
 // planar UVs like Three.js box/plane default
 vec2 uv=abs(N.y)>.7?p.xz:(abs(N.x)>.7?p.zy:p.xy);
 vec3 albedo=vec3(.5);float alpha=1.,rough=.95,metal=0.,emit=0.;int bumpLayer=-1;float bumpScale=0.;

 // Material IDs match piece() in main.cpp / HTML MeshStandardMaterial assignments
 if(mat==0){albedo=sampleMap(0,uv);bumpLayer=0;bumpScale=.8;rough=.95;}           // concrete
 if(mat==1){vec3 moss=sampleMap(1,uv);float ml=dot(moss,vec3(.299,.587,.114));albedo=mix(vec3(ml)*vec3(.76,.79,.68),moss*vec3(.65,.72,.52),.2);bumpLayer=1;bumpScale=1.5;rough=1.;} // moss — brighter foliage
 if(mat==2){albedo=vec3(.612,.769,.812);alpha=.38;rough=.08;metal=.2;emit=s.sky.w*.5;} // glass 0x9cc4cf
 if(mat==3){albedo=vec3(.173,.173,.180);rough=.45;metal=.75;}                      // dark 0x2c2c2e
 if(mat==4){albedo=vec3(1.,.957,.847);emit=.25+s.sky.w*2.75;rough=.5;}             // lamp 0xfff4d8 / emissive 0xffe0a0
 if(mat==5){albedo=sampleMap(4,uv);bumpLayer=4;bumpScale=.8;rough=.75;}            // timber
 if(mat==6){ // Optimum vine — UV in normal.xy (tiled), foliage biome tint
  vec2 vu=fract(n.xy);
  vec4 iv=texture(maps,vec3(vu,2.));
  if(iv.a<.5)discard;
  albedo=applyBiome(iv.rgb,biomeFoliage(p));rough=1.;
  N=normalize(vec3(0,1,0)+L*.2);
 }
 if(mat==7){albedo=sampleMap(3,uv);bumpLayer=3;bumpScale=1.2;rough=.9;}            // brick
 if(mat==8){albedo=sampleMap(5,uv);bumpLayer=5;bumpScale=1.5;rough=.45;metal=.5;}  // steel
 if(mat==18){albedo=sampleMap(23,uv);bumpLayer=23;bumpScale=1.35;rough=.92;}       // board-form concrete (layer 8+15)
 if(mat==10){outColor=vec4(.910,.314,.110,.35);return;}                             // ghost 0xe8501c
 if(mat==11){albedo=vec3(.467,.475,.467);rough=1.;}                                 // ground 0x777977
 if(mat==12){ // plot: concrete map * 0xcfcdc5, bumpScale .5, grid opacity .25
  albedo=sampleMap(0,uv)*vec3(.812,.804,.773);
  bumpLayer=0;bumpScale=.5;rough=.95;
  float lx=abs(p.x-round(p.x)),lz=abs(p.z-round(p.z));
  float grid=1.-smoothstep(0.,.015,min(lx,lz));
  albedo=mix(albedo,albedo*vec3(.114,.114,.106),grid*.25); // 0x1d1d1b @ 25%
 }
 if(mat==13){albedo=vec3(.082,.082,.082);rough=.6;metal=.3;}                        // cable wire 0x151515 / 0x121212
 if(mat==14){outColor=vec4(.75,.8,.85,.35);return;}                                  // rain
 if(mat==15){albedo=vec3(.290,.227,.165);rough=1.;}                                 // bark 0x4a3a2a
 if(mat==16){ // tree canopy — foliage biome tint (greyscale ivy mask)
  vec4 iv=texture(maps,vec3(uv*2.,7.));
  if(iv.a<.5)discard;
  albedo=applyBiome(iv.rgb,biomeFoliage(p));rough=1.;
 }
 if(mat==17){ // PSX grass_1 — UV in normal.xy (HTML alphaTest .45, tint 0xb4c89a)
  vec2 gu=clamp(n.xy,0.,1.);
  vec4 gr=texture(maps,vec3(gu,6.));
  if(gr.a<.45)discard;
  albedo=gr.rgb*vec3(.706,.784,.604);rough=1.;N=normalize(vec3(0,1,0)+L*.15);
 }

 if(mat==19){ // ivy leaf billboard — foliage biome tint
  vec2 fu=clamp(n.xy,0.,1.);
  vec4 frn=texture(maps,vec3(fu,7.));
  if(frn.a<.45)discard;
  albedo=applyBiome(frn.rgb,biomeFoliage(p));rough=1.;N=normalize(vec3(0,1,0)+L*.2);
 }

 // Individual ivy leaves and stems use geometric, two-sided normals.
 if(mat==20){
  float variation=fract(sin(dot(floor(p*31.),vec3(127.1,311.7,74.7)))*43758.5453);
  albedo=mix(vec3(.095,.145,.048),vec3(.23,.31,.105),variation);
  rough=.92;N=dot(N,V)<0.?-N:N;
 }
 if(mat==21){albedo=vec3(.105,.105,.052);rough=1.;}

 if(mat>=22){
  vec4 vegetation=texture(maps,vec3(foliageUv,float(mat-22+8)));
  // Bark mats from natureBarkLayer[]: 23 deciduous, 32 birch, 33/35 pine.
  bool bark=mat==23||mat==32||mat==33||mat==35;
  if(!bark&&vegetation.a<.48)discard;
  float leafLum=dot(vegetation.rgb,vec3(.299,.587,.114));albedo=bark?vegetation.rgb*vec3(.78,.79,.73):mix(vec3(leafLum),vegetation.rgb,.25)*vec3(.69,.72,.61);rough=.95;
  N=gl_FrontFacing?N:-N;
 }
 // bump from map (HTML bumpMap)
 if(bumpLayer>=0){
  float h=texture(maps,vec3(uv,float(bumpLayer))).r;
  vec3 dpdx=dFdx(p),dpdy=dFdy(p);
  vec3 T1=cross(dpdy,N),T2=cross(N,dpdx);float det=dot(dpdx,T1);
  vec3 grad=sign(det)*(dFdx(h)*T1+dFdy(h)*T2);
  N=normalize(abs(det)*N-grad*bumpScale*.35);
 }

 // HTML instanceColor multiplies albedo
 albedo*=ao;

 // Lights — numbers from HTML sun()/MOODS via uniforms:
 // sun.xyz = direction, sun.w = intensity (si * el factor * rain)
 // atmos.w = sat, cfg.x=exposure, cfg.y=fog density, cfg.z=ao on, sky.rgb=hor, atmos.rgb=top, sky.w=night
 float nd=max(dot(N,L),0.);
 float shadow=1.;
 if(nd>0.&&mat!=4&&mat!=17&&mat!=19&&mat<22&&s.sky.w<.5) shadow=softShadow(p+normalize(n)*.05,L);
 shadow=mix(.42,1.,shadow); // keep bounce light in shadows

 // HemisphereLight: day sky 0xd0d4d6 / gnd 0x6b675e ; night sky 0x4a5a90
 vec3 hemiSky=mix(vec3(.86,.88,.90),vec3(.290,.353,.565),s.sky.w);
 vec3 hemiGnd=mix(vec3(.48,.46,.42),vec3(.08,.09,.12),s.sky.w);
 float hemiI=s.lighting.x;
 vec3 ambient=mix(hemiGnd,hemiSky,N.y*.5+.5)*hemiI*1.15;

 // sun color from CPU in... we only have intensity. Approximate HTML sun color with low elevation:
 vec3 sunCol=mix(vec3(1.,.955,.88),vec3(1.,.72,.50),clamp(1.-s.sun.y*1.6,0.,1.)); // 0xfff1dc / 0xffb37a
 if(s.sky.w>.5)sunCol=vec3(.435,.525,.784); // moon 0x6f86c8

 vec3 color=albedo*(ambient+sunCol*nd*shadow*s.sun.w*1.08);

 // fill directional 0x9fb4d8 — boosted for readable form
 if(s.sky.w<.5){
  vec3 F=normalize(vec3(-L.x,.55,-L.z));
  color+=albedo*vec3(.624,.706,.847)*max(dot(N,F),0.)*.52;
 }
 // foliage wrap / rim so greens don't go flat black
 if(mat==1||mat==6||mat==16||mat==17||mat==19||(mat>=22&&mat!=23&&mat!=32&&mat!=33&&mat!=35)){
  float wrap=max(dot(N,L)*.5+.5,0.);
  color+=albedo*sunCol*wrap*.18*s.sun.w;
  color+=albedo*vec3(.45,.55,.35)*.12;
 }

 if(mat==20){color+=albedo*sunCol*max(dot(N,L)*.5+.5,0.)*.055*s.sun.w;}

 // Specular approx for MeshStandard
 float spec=pow(max(dot(reflect(-L,N),V),0.),mix(64.,4.,rough))*(.04+metal*.6);
 color+=sunCol*spec*shadow*s.sun.w;

 // Local fixture illumination, with smooth distance falloff and wall occlusion.
 for(int i=0;i<8;i++){
  if(s.lamps[i].w<=0.||mat==4)continue;
  vec3 d=s.lamps[i].xyz-p;float dist=length(d);
  if(dist>=6.||dist<.001)continue;
  vec3 lightDir=d/dist;float diffuse=max(dot(N,lightDir),0.);
  if(diffuse<=0.)continue;
  float cutoff=clamp(1.-pow(dist/6.,4.),0.,1.);
  float att=s.lamps[i].w*cutoff*cutoff/(1.+dist*dist);
  // Trace only lit, nearby fragments. Skip the emitter's own fixture cell.
  vec3 start=p+normalize(n)*.045;
  vec3 ray=s.lamps[i].xyz-start;
  int steps=int(ceil(length(ray)/.18));float visible=1.;
  ivec3 sourceCell=ivec3(floor(s.lamps[i].xyz));
  for(int j=1;j<34;j++){
   if(j>=steps)break;
   vec3 q=start+ray*(float(j)/float(steps));
   if(all(equal(ivec3(floor(q)),sourceCell)))continue;
   if(solid(q)){visible=0.;break;}
  }
  color+=albedo*vec3(1.,.851,.627)*diffuse*att*visible;
 }
 color+=albedo*emit;

 if(mat==2){
  float fres=pow(1.-max(dot(N,V),0.),5.);
  vec3 refl=mix(s.sky.rgb,s.atmos.rgb,max(reflect(-V,N).y,0.));
  color=mix(color,refl,.1+fres*.45);alpha=clamp(alpha+fres*.25,0.,.9);
 }

 // FogExp2(hor, density) — density in cfg.y
 float fogAmt=1.-exp(-length(s.eye.xyz-p)*s.cfg.y);
 color=mix(color,s.sky.rgb,clamp(fogAmt,0.,1.));
 if(s.lighting.z>0.)color=mix(color,vec3(.018,.055,.051),1.-exp(-length(s.eye.xyz-p)*.16));

 // SSAO + tonemap happen in post.frag (exact HTML PQ pass) — output linear here
 outColor=vec4(color,alpha);
}
