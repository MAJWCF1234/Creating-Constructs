// World-space water shading: continuous ripples, geometric reflections/refraction,
// absorption and a thin shoreline. Bounded grid traces reuse the scene collision mask.
int waterCellType(vec3 q){ivec3 c=ivec3(floor(q));if(any(lessThan(c,ivec3(0)))||c.x>=int(s.cfg.w)||c.z>=int(s.cfg.w)||c.y>=20)return -1;int v=s.cells[c.x*640+c.z*20+c.y];return v==0?-1:(v-1)%128;} // PieceEncodingStride=128
bool waterStructure(vec3 q){
 int t=waterCellType(q);if(t<0||t==14)return false;
 // Pool basins (24): only rim/floor collide — open water volume must not self-hit
 // (that painted warm specular through the concrete walls).
 if(t==24){
  vec3 f=fract(q);
  return f.y<.12||(f.y<.52&&max(abs(f.x-.5),abs(f.z-.5))>.42);
 }
 return structureSolid(q);
}
vec3 waterSky(vec3 d){vec3 sky=mix(s.sky.rgb,s.atmos.rgb,smoothstep(0.,.85,d.y));vec3 sunColor=mix(vec3(1.,.95,.86),vec3(.45,.52,.75),s.sky.w);return sky+sunColor*pow(max(dot(d,normalize(s.sun.xyz)),0.),320.)*s.sun.w;}
bool waterStructureTrace(vec3 start,vec3 direction,float limit,out vec3 hit,out vec3 normal,out float distance){
 float t=.045,previous=0.;
 for(int i=0;i<64;i++){
  vec3 q=start+direction*t;
  if(waterStructure(q)){
   float lo=previous,hi=t;for(int j=0;j<4;j++){float mid=(lo+hi)*.5;if(waterStructure(start+direction*mid))hi=mid;else lo=mid;}
   distance=hi;hit=start+direction*hi;
   const float e=.13;normal=vec3(float(waterStructure(hit-vec3(e,0,0)))-float(waterStructure(hit+vec3(e,0,0))),float(waterStructure(hit-vec3(0,e,0)))-float(waterStructure(hit+vec3(0,e,0))),float(waterStructure(hit-vec3(0,0,e)))-float(waterStructure(hit+vec3(0,0,e))));
   normal=dot(normal,normal)>.01?normalize(normal):-direction;return true;
  }
  if(q.y<-.01){distance=t;hit=q;normal=vec3(0,1,0);return true;}
  previous=t;t+=.09+t*.045;if(t>limit||q.y>23.)break;
 }
 distance=limit;hit=start+direction*limit;normal=vec3(0,1,0);return false;
}
vec3 waterHitColor(vec3 q,vec3 N){
 int type=waterCellType(q);vec2 uv=abs(N.y)>.65?q.xz:abs(N.x)>.65?q.zy:q.xy;
 vec3 base;
 if(type<0&&q.y>.05)base=vec3(.095,.12,.077); // foliage shadow voxels above occupied cells
 else if(type==5||type==17||type==14||type==15)base=vec3(.095,.14,.075);
 else if(type==19||type==39||type==35)base=sampleMap(4,uv)*.8;
 else if(type==20||type==11||type==31)base=sampleMap(5,uv)*.5;
 else base=sampleMap(type==18?3:0,uv);
 vec3 light=vec3(.27,.29,.30)*s.lighting.x+vec3(.8,.78,.7)*max(dot(N,normalize(s.sun.xyz)),0.)*s.sun.w;
 vec3 color=base*light;
 for(int i=0;i<8;i++){if(s.lamps[i].w<=0.)continue;vec3 d=s.lamps[i].xyz-q;float ds=dot(d,d);color+=base*vec3(1.,.85,.63)*s.lamps[i].w/(1.+ds)*.65;}
 return color;
}
bool waterTrace(vec3 start,vec3 direction,float limit,out vec3 hit,out vec3 normal,out float distance,out vec3 color){
 bool structure=waterStructureTrace(start,direction,limit,hit,normal,distance);color=structure?waterHitColor(hit,normal):vec3(0);
 float nearest=structure?distance:limit;vec3 plantHit,plantNormal,plantColor;float ambient;
 if(waterPlantTrace(start,direction,nearest,plantHit,plantNormal,plantColor,ambient)){
  hit=plantHit;normal=plantNormal;distance=nearest;
  float diffuse=abs(dot(normal,normalize(s.sun.xyz)));vec3 light=vec3(.27,.29,.30)*s.lighting.x+vec3(.8,.78,.7)*diffuse*s.sun.w;
  color=plantColor*light*ambient;for(int i=0;i<8;i++){if(s.lamps[i].w<=0.)continue;vec3 d=s.lamps[i].xyz-hit;color+=plantColor*vec3(1.,.85,.63)*s.lamps[i].w/(1.+dot(d,d))*.65;}
  return true;
 }
 return structure;
}
vec3 renderWater(vec3 point,vec3 geometricNormal){
 vec3 view=normalize(s.eye.xyz-point);float time=s.eye.w;
 vec2 wave=vec2(cos(dot(point.xz,vec2(2.1,1.2))+time*.75)*.026+cos(dot(point.xz,vec2(-4.3,5.1))-time*1.1)*.014,
                sin(dot(point.xz,vec2(1.3,-2.4))+time*.63)*.026+sin(dot(point.xz,vec2(5.7,3.9))+time*1.3)*.014);
 vec2 rainCell=fract(point.xz*1.4)-.5;float rainId=fract(sin(dot(floor(point.xz*1.4),vec2(127.1,311.7)))*43758.5453);
 float life=fract(time*.75+rainId),radius=length(rainCell);float ring=sin((radius-life*.55)*70.)*exp(-pow((radius-life*.4)*19.,2.))*(1.-life);
 wave+=rainCell/max(radius,.01)*ring*.024*s.lighting.y;
 vec3 N=normalize(geometricNormal+vec3(wave.x,0,wave.y));bool below=dot(geometricNormal,view)<0.;if(below)N=-N;
 vec3 reflectionDirection=reflect(-view,N),refractionDirection=refract(-view,N,below?1.333:1./1.333);
 vec3 hit,normal,rayColor;float distance;
 vec3 reflected=waterSky(reflectionDirection);
 if(waterTrace(point+N*.055,reflectionDirection,28.,hit,normal,distance,rayColor)){
  vec3 object=rayColor;float fade=exp(-distance*.025);
  reflected=mix(waterSky(reflectionDirection),object,fade*.9);
 }
 float depth=3.;vec3 bottom=vec3(.13,.17,.15);
 if(dot(refractionDirection,refractionDirection)>.01&&waterTrace(point-N*.055,refractionDirection,8.,hit,normal,distance,rayColor)){
  depth=distance;bottom=rayColor;
  float caustic=pow(1.-abs(sin(hit.x*5.1+sin(hit.z*3.7+time*.7))),9.);
  bottom*=1.+caustic*.12*exp(-depth*.5)*min(s.sun.w,2.);
 }
 vec3 transmittance=exp(-vec3(.62,.28,.20)*depth);
 vec3 body=mix(vec3(.025,.072,.069)*(s.lighting.x+.16),bottom,transmittance);
 float facing=clamp(dot(N,view),0.,1.);float fresnel=.025+.975*pow(1.-facing,5.);
 vec3 color=mix(body,reflected,clamp(fresnel+.10,0.,1.));
 vec3 halfVector=normalize(normalize(s.sun.xyz)+view);
 color+=vec3(1.,.96,.86)*pow(max(dot(N,halfVector),0.),220.)*s.sun.w*.65;
 for(int i=0;i<8;i++){if(s.lamps[i].w<=0.)continue;vec3 d=s.lamps[i].xyz-point;float ds=dot(d,d);vec3 H=normalize(normalize(d)+view);color+=vec3(1.,.85,.63)*pow(max(dot(N,H),0.),90.)*s.lamps[i].w/(1.+ds)*.45;}
 // Only actual solid banks create an edge; adjacent water tiles share one surface.
 if(geometricNormal.y>.7){
  float bank=max(max(float(waterStructure(point+vec3(.13,0,0))),float(waterStructure(point-vec3(.13,0,0)))),max(float(waterStructure(point+vec3(0,0,.13))),float(waterStructure(point-vec3(0,0,.13)))));
  float broken=.5+.5*sin(point.x*24.+point.z*19.+time*.4);
  color=mix(color,vec3(.29,.34,.31)*(s.lighting.x+.2),bank*broken*.16);
 }
 float fog=1.-exp(-length(s.eye.xyz-point)*s.cfg.y);color=mix(color,s.sky.rgb,fog);if(s.lighting.z>0.)color=mix(color,vec3(.018,.055,.051),1.-exp(-length(s.eye.xyz-point)*.16));return color;
}
