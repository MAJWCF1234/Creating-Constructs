#version 450
layout(location=0) in vec3 pos;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 info;
layout(location=3) in vec2 natureUv;
layout(location=4) out vec2 foliageUv;
layout(location=0) out vec3 p;
layout(location=1) out vec3 n;
layout(location=2) flat out int mat;
layout(location=3) out float ao;
layout(std430,binding=0) readonly buffer State {mat4 vp;vec4 eye;vec4 sun;vec4 sky;vec4 cfg;vec4 lamps[16];vec4 atmos;vec4 lighting;int cells[81920];uint natureMask[];} s;
void main(){p=pos;foliageUv=natureUv;n=normal;
 // Foliage wind only — never topo rock/dirt (38/39) or bark
 int mid=int(info.x);bool bark=mid==23||mid==32||mid==33||mid==35;
 if(mid>=22&&mid<38&&!bark){float amp=.014*(s.lighting.y>.4?2.6:1.)*(1.+s.lighting.w*.4);float wind=sin(s.eye.w*1.3+pos.x*.7+pos.z)*amp;p.x+=wind;p.z+=wind*.4;}
mat=mid;ao=info.y;gl_Position=s.vp*vec4(p,1);}
