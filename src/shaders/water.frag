#version 450
#extension GL_GOOGLE_include_directive : require
#include "scene_shared.glsl"
#include "foliage_trace.glsl"
#include "water.glsl"
void main(){outColor=vec4(foliageUv.x>1.5?renderFlowWater(p,normalize(n)):renderWater(p,normalize(n)),1.);}
