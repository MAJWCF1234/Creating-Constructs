#version 450
layout(location=0) out vec2 vUv;
void main(){
 // fullscreen triangle
 vec2 p=vec2((gl_VertexIndex<<1)&2,gl_VertexIndex&2);
 vUv=p;
 gl_Position=vec4(p*2.-1.,0.,1.);
}
