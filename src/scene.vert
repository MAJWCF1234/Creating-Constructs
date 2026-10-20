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
layout(std430,binding=0) readonly buffer State {mat4 vp;vec4 eye;vec4 sun;vec4 sky;vec4 cfg;vec4 lamps[8];vec4 atmos;vec4 lighting;int cells[20480];uint natureMask[];} s;
void main(){p=pos;foliageUv=natureUv;n=normal;
 if(int(info.x)>=22&&int(info.x)!=23){float wind=sin(s.eye.w*1.3+pos.x*.7+pos.z)*.014;p.x+=wind;p.z+=wind*.4;}
mat=int(info.x);ao=info.y;gl_Position=s.vp*vec4(p,1);}
