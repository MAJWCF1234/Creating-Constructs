// Scene/sky output LINEAR color — post.frag does the HTML tonemap/grain/vignette
vec3 finishImage(vec3 c,vec2 pixel,vec2 uv,float time,float sat,float exposure){
 return c; // identity — real finish is in post.frag
}
