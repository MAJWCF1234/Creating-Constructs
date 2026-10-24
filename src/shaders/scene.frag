#version 450
#extension GL_GOOGLE_include_directive : require
#include "scene_shared.glsl"
void main(){
 bool movingDoorLeaf=(mat>=128?(mat-128)%64:mat)==52;
 int recipe=mat>=128?(mat-128)/64:0;int material=mat>=128?(mat-128)%64:mat;material=surfaceBase(recipe&31,material);vec3 sp=recipe!=0?surfacePoint(p,recipe):p;vec3 sn=recipe!=0?normalize(surfacePoint(n,recipe)):n;if(movingDoorLeaf){vec3 local=vec3(foliageUv,0.);sp=recipe!=0?surfacePoint(local,recipe):local;sn=recipe!=0?normalize(surfacePoint(vec3(0,0,1),recipe)):vec3(0,0,1);}
 if(material==9){outColor=vec4(0);return;}
 vec3 N=normalize(n),V=normalize(s.eye.xyz-p),L=normalize(s.sun.xyz);
 // planar UVs like Three.js box/plane default
 vec2 uv=abs(N.y)>.7?sp.xz:(abs(sn.x)>.7?sp.zy:sp.xy);
 // Per-cell UV shift only on cast concrete — must NOT touch moss/foliage/maps that use atlas UVs
 float cellJitter=movingDoorLeaf?0.:fract(sin(dot(floor(p+vec3(.01)),vec3(12.9898,78.233,37.719)))*43758.5453);
 vec2 concreteUv=uv+vec2(cellJitter,fract(cellJitter*7.13))*.37;
 vec3 albedo=vec3(.5);float alpha=1.,rough=.95,metal=0.,emit=0.;int bumpLayer=-1;float bumpScale=0.;bool bumpFromAlpha=false;

 // Material IDs match piece() in main.cpp / HTML MeshStandardMaterial assignments
 if(material==0){
  vec2 castUv=uv*2.;vec3 castColor=sampleMap(0,castUv);
  albedo=mix(vec3(.55,.54,.51),max(castColor,vec3(.28)),.46);
  vec2 panel=fract(uv/3.);float edge=min(min(panel.x,1.-panel.x),min(panel.y,1.-panel.y));
  albedo*=mix(.68,1.,smoothstep(.001,.003,edge));
  bumpLayer=0;bumpScale=.006;rough=.98;uv=castUv;
 } // quiet cast-concrete panels with small pores, not raised metal-looking relief

 if(material==1){vec3 moss=sampleMap(1,uv);float ml=dot(moss,vec3(.299,.587,.114));albedo=mix(vec3(ml)*vec3(.76,.79,.68),moss*vec3(.65,.72,.52),.2);bumpLayer=1;bumpScale=1.5;rough=1.;} // moss — brighter foliage
 if(material==2){albedo=vec3(.612,.769,.812);alpha=.38;rough=.08;metal=.2;emit=s.sky.w*.5;} // glass 0x9cc4cf
 if(material==3){albedo=vec3(.173,.173,.180);rough=.45;metal=.75;}                      // dark 0x2c2c2e
 if(material==4){albedo=vec3(1.,.957,.847);emit=.25+s.sky.w*2.75;rough=.5;}             // lamp 0xfff4d8 / emissive 0xffe0a0
 if(material==42){albedo=vec3(1.,.72,.42);emit=.28+s.sky.w*2.6;rough=.5;}               // amber lamp
 if(material==43){albedo=vec3(.72,.86,1.);emit=.26+s.sky.w*2.6;rough=.5;}               // cool lamp
 if(material==52){albedo=vec3(.25,.31,.29);rough=.86;metal=.08;}
 if(material==48){albedo=vec3(.08,.8,1.);emit=.5+s.sky.w*2.5;rough=.8;}
 if(material==49){albedo=vec3(1.,.08,.36);emit=.5+s.sky.w*2.5;rough=.8;}
 if(material==50){albedo=vec3(.025,.18,.20);emit=.35+s.sky.w*.6;rough=.88;}
 if(material==51){albedo=vec3(.03,.38,.07);emit=.5+s.sky.w*1.4;rough=.9;}
 if(material==53){albedo=vec3(.62,.2,1.);emit=.5+s.sky.w*2.5;rough=.8;}   // violet neon
 if(material==54){albedo=vec3(.52,.40,.26);rough=1.;}                        // carpet
 if(material==55){albedo=vec3(.86,.80,.55);rough=.9;}                        // pale yellow paint
 if(material==56){albedo=vec3(.88,.88,.84);rough=.95;}                       // ceiling tile / appliance white
 if(material==57){albedo=vec3(.64,.82,.86);rough=.25;metal=.05;}             // pool tile
 if(material==58){albedo=vec3(.12,.12,.15);rough=.5;metal=.45;}              // dark cabinet
 if(material==5){albedo=sampleMap(4,uv);bumpLayer=4;bumpScale=.12;rough=.88;}            // timber
 if(material==6){ // Optimum vine — UV in normal.xy (tiled), foliage biome tint
  vec2 vu=fract(n.xy);
  vec4 iv=textureLod(maps,vec3(vu,2.),0.);
  if(iv.a<.5)discard;
  albedo=applyBiome(iv.rgb,biomeFoliage(p));rough=1.;
  N=normalize(vec3(0,1,0)+L*.2);
 }
 if(material==7){ // red clay: matte relief from the same continuous projection as color
  vec4 brick=sampleBrick(sp,normalize(sn),recipe);
  albedo=brick.rgb*mix(.94,1.,brick.a);rough=.97;metal=0.;
  // Differentiate the blended height itself: never switch UV axes mid-surface.
  vec3 dpdx=dFdx(p),dpdy=dFdy(p);
  vec3 T1=cross(dpdy,N),T2=cross(N,dpdx);float det=dot(dpdx,T1);
  vec3 gradient=sign(det)*(dFdx(brick.a)*T1+dFdy(brick.a)*T2);
  N=normalize(max(abs(det),1e-10)*N-gradient*.012);
 }
 if(material==8){albedo=sampleMap(5,uv);bumpLayer=5;bumpScale=1.5;rough=.45;metal=.5;}  // steel
 if(material==18){albedo=sampleMap(24,concreteUv);bumpLayer=24;bumpScale=.08;rough=.98;uv=concreteUv;}       // board-form concrete
 if(material==19){albedo=sampleMap(0,concreteUv)*vec3(.72,.69,.64);bumpLayer=0;bumpScale=.065;rough=.99;uv=concreteUv;} // exposed aggregate
 if(material==41){albedo=vec3(.92,.78,.12);rough=.55;metal=.05;emit=.04;}               // hazard stripe paint
 if(material==38){ // Tiny-Glade rock — topo albedo + bump normals, 2× UV scale
  // topo layers start at 25; 0 sandstone,1 top,2 limestone,3 mossy lim,4 granite,5 brown,6 schist
  float cellH=fract(sin(dot(floor(p),vec3(12.9,78.2,45.1)))*43758.5);
  int pick=int(cellH*5.); // bias warm stone: sandstone, brown, granite, limestone, schist
  int rockVar=pick==0?0:pick==1?5:pick==2?4:pick==3?2:6;
  int sideLayer=25+rockVar;
  int topLayer=(rockVar==0)?26:sideLayer;
  vec3 bw=abs(sn);bw=bw/(bw.x+bw.y+bw.z+1e-4);
  float sc=1.35; // denser than 1×, not so tight it reads as voxels
  vec3 tx=textureLod(maps,vec3(sp.zy*sc,float(sideLayer)),0.).rgb;
  vec3 ty=textureLod(maps,vec3(sp.xz*sc,float(topLayer)),0.).rgb;
  vec3 tz=textureLod(maps,vec3(sp.xy*sc,float(sideLayer)),0.).rgb;
  albedo=tx*bw.x+ty*bw.y+tz*bw.z;
  albedo*=vec3(1.08,1.02,.94); // warm tan lift like Tiny Glade cliffs
  if(cellH>.78){vec3 mossR=textureLod(maps,vec3(sp.xz*sc,31.),0.).rgb;albedo=mix(albedo,mossR,.28);} // thick moss in crevices
  float facet=pow(max(max(bw.x,bw.y),bw.z),1.6);
  albedo*=mix(.82,1.08,facet);
  // Bump from topo height (luma) — drives normals; UV matches albedo scale
  uv=abs(sn.y)>.7?sp.xz*sc:(abs(sn.x)>.7?sp.zy*sc:sp.xy*sc);
  bumpLayer=(abs(N.y)>.7)?topLayer:sideLayer;bumpScale=.18;rough=.98;
 }
 if(material==39){ // wet-moss dirt — topography dirt / rocky / mossy soil
  float cellH=fract(sin(dot(floor(p),vec3(19.1,47.3,91.7)))*43758.5);
  int dirtLayer=32+int(cellH*3.); // 31 earth, 32 rocky, 33 mossy  (24+7..)
  vec3 bw=abs(sn);bw=bw/(bw.x+bw.y+bw.z+1e-4);float sc=2.4; // doubled
  vec3 tx=textureLod(maps,vec3(sp.zy*sc,float(dirtLayer)),0.).rgb;
  vec3 ty=textureLod(maps,vec3(sp.xz*sc,float(dirtLayer)),0.).rgb;
  vec3 tz=textureLod(maps,vec3(sp.xy*sc,float(dirtLayer)),0.).rgb;
  albedo=tx*bw.x+ty*bw.y+tz*bw.z;
  uv=abs(sn.y)>.7?sp.xz*sc:(abs(sn.x)>.7?sp.zy*sc:sp.xy*sc);
  bumpLayer=dirtLayer;bumpScale=.12;rough=.99;
 }
 if(material==44){outColor=vec4(.95,.12,.08,.38);return;}
 if(material==45){outColor=vec4(.18,.75,.48,.28);return;}
 if(material==46){outColor=vec4(.2,.55,1.,.65);return;}
 if(material==10){outColor=vec4(.910,.314,.110,.35);return;}                             // ghost 0xe8501c
 if(material==11){albedo=vec3(.467,.475,.467);rough=1.;}                                 // ground 0x777977
 if(material==12){ // plot: concrete map * 0xcfcdc5, bumpScale .5, grid opacity .25
  albedo=sampleMap(0,uv)*vec3(.812,.804,.773);
  bumpLayer=0;bumpScale=.5;rough=.95;
  float lx=abs(p.x-round(p.x)),lz=abs(p.z-round(p.z));
  float grid=1.-smoothstep(0.,.015,min(lx,lz));
  albedo=mix(albedo,albedo*vec3(.114,.114,.106),grid*.25); // 0x1d1d1b @ 25%
 }
 if(material==13){albedo=vec3(.082,.082,.082);rough=.6;metal=.3;}                        // cable wire 0x151515 / 0x121212
 if(material==14){ // rain streak — soft blue-gray, camera-facing alpha
  float shimmer=.25+.2*fract(sin(dot(p.xz,vec2(12.9,78.2))+s.eye.w*3.)*43758.5);
  outColor=vec4(.68,.76,.88,.18+shimmer*.28);return;
 }
 if(material==15){albedo=vec3(.290,.227,.165);rough=1.;}                                 // bark 0x4a3a2a
 if(material==16){ // tree canopy — foliage biome tint (greyscale ivy mask)
  vec4 iv=textureLod(maps,vec3(uv*2.,7.),0.);
  if(iv.a<.5)discard;
  albedo=applyBiome(iv.rgb,biomeFoliage(p));rough=1.;
 }
 if(material==17){ // PSX grass_1 — UV in normal.xy (HTML alphaTest .45, tint 0xb4c89a)
  vec2 gu=clamp(n.xy,0.,1.);
  vec4 gr=textureLod(maps,vec3(gu,6.),0.);
  if(gr.a<.45)discard;
  albedo=gr.rgb*vec3(.706,.784,.604);rough=1.;N=normalize(vec3(0,1,0)+L*.15);
 }

 // Ivy leaves use material 20 (geometric); material 19 is aggregate only — do not override.

 // Individual ivy leaves and stems use geometric, two-sided normals.
 if(material==20){
  float variation=fract(sin(dot(floor(p*31.),vec3(127.1,311.7,74.7)))*43758.5453);
  albedo=mix(vec3(.095,.145,.048),vec3(.23,.31,.105),variation);
  rough=.92;N=dot(N,V)<0.?-N:N;
 }
 if(material==21){albedo=vec3(.105,.105,.052);rough=1.;}

 // Nature plant mats only (22..37). Topo rock/dirt are 38/39 — must not hit foliage path.
 if(material>=22&&material<38){
  vec4 vegetation=textureLod(maps,vec3(foliageUv,float(material-22+8)),0.);
  // Bark mats from natureBarkLayer[]: 23 deciduous, 32 birch, 33/35 pine.
  bool bark=material==23||material==32||material==33||material==35;
  if(!bark&&vegetation.a<.48)discard;
  float leafLum=dot(vegetation.rgb,vec3(.299,.587,.114));albedo=bark?vegetation.rgb*vec3(.78,.79,.73):mix(vec3(leafLum),vegetation.rgb,.25)*vec3(.69,.72,.61);rough=.95;
  N=gl_FrontFacing?N:-N;
 }
 // bump from map (HTML bumpMap); brick uses smooth height in alpha
 if(bumpLayer>=0){
  float h=bumpFromAlpha?textureLod(maps,vec3(uv,float(bumpLayer)),0.).a:textureLod(maps,vec3(uv,float(bumpLayer)),0.).r;
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
 if(nd>0.&&material!=4&&material!=17&&material!=20&&(material<22||material==52)&&s.sky.w<.5) shadow=softShadow(p+normalize(n)*.05,L);
 shadow=mix(.10,1.,shadow); // keep bounce light in shadows

 // HemisphereLight: day sky 0xd0d4d6 / gnd 0x6b675e ; night sky 0x4a5a90
 vec3 hemiSky=mix(vec3(.86,.88,.90),vec3(.290,.353,.565),s.sky.w);
 vec3 hemiGnd=mix(vec3(.48,.46,.42),vec3(.08,.09,.12),s.sky.w);
 float hemiI=s.lighting.x;
 vec3 ambient=mix(hemiGnd,hemiSky,N.y*.5+.5)*hemiI*1.15;
 // Give tall interiors shelter from the hemisphere fill, not just immediate cell AO.
 float skyAccess=1.;vec3 probe=p+normalize(n)*.065;
 // Moving thin leaves use hemisphere fill directly; cell-column shelter produces tile seams on a swinging panel.
 if(s.sky.w<.5&&!movingDoorLeaf)for(int y=int(floor(probe.y))+1;y<20;y++){
  if(structureSolid(vec3(probe.x,float(y)+.93,probe.z))){skyAccess=.28;break;}
 }
 ambient*=skyAccess;

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
 if(material==1||material==6||material==16||material==17||(material>=22&&material<38&&material!=23&&material!=32&&material!=33&&material!=35)){
  float wrap=max(dot(N,L)*.5+.5,0.);
  color+=albedo*sunCol*wrap*.18*s.sun.w;
  color+=albedo*vec3(.45,.55,.35)*.12;
 }

 if(material==20){color+=albedo*sunCol*max(dot(N,L)*.5+.5,0.)*.055*s.sun.w;}

 // Specular approx for MeshStandard
 float spec=pow(max(dot(reflect(-L,N),V),0.),mix(64.,4.,rough))*(.04+metal*.6);
 color+=sunCol*spec*shadow*s.sun.w*(material==7?.22:1.);

 // Local fixture illumination, with smooth distance falloff and wall occlusion.
 for(int i=0;i<16;i++){
  if(s.lamps[i].w<=0.||material==4||material==42||material==43)continue;
  float lampW=s.lamps[i].w;bool angled=lampW>=100.&&lampW<200.;float inten=fixturePower(lampW);
  vec3 d=s.lamps[i].xyz-p;float dist=length(d);
  if(dist>=6.||dist<.001)continue;
  vec3 lightDir=d/dist;float diffuse=max(dot(N,lightDir),0.);
  if(diffuse<=0.)continue;
  // Angle fixtures only wash surfaces below the aperture (wall grazing).
  if(angled){float down=clamp(lightDir.y,0.,1.);if(down<.08)continue;diffuse*=down*down;}
  float cutoff=clamp(1.-pow(dist/6.,4.),0.,1.);
  float att=inten*cutoff*cutoff/(1.+dist*dist);
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
  color+=albedo*fixtureColor(lampW)*diffuse*att*visible;
 }
 color+=albedo*emit;

 if(material==2){
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
