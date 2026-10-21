void spherePart(V center,float radius,int material,bool hemisphere,int slices,int rings){
 float low=hemisphere?0.f:-PI/2.f;
 for(int j=0;j<rings;j++)for(int i=0;i<slices;i++){
  float u=low+(PI/2-low)*j/rings,v=low+(PI/2-low)*(j+1)/rings,a=i*2*PI/slices,b=(i+1)*2*PI/slices;
  auto p=[&](float th,float ph){return center+V{cosf(th)*cosf(ph),sinf(ph),sinf(th)*cosf(ph)}*radius;};
  if(!hemisphere&&j==0)tri(p(b,u),p(a,v),p(b,v),material);
  else if(j==rings-1)tri(p(b,u),p(a,u),p(a,v),material);
  else quad(p(b,u),p(a,u),p(a,v),p(b,v),material);
 }
}
void roundVault(){
 box(1,.5f,1,0,-.25f);
 for(int i=0;i<32;i++){
  float a=PI*i/32,b=PI*(i+1)/32;
  V p{.5f*cosf(a),.5f*sinf(a),-.5f},q{.5f*cosf(b),.5f*sinf(b),-.5f};
  V P{p.x,p.y,.5f},Q{q.x,q.y,.5f};quad(p,q,Q,P,0);
  tri({0,0,-.5f},q,p,0);tri({0,0,.5f},P,Q,0);
 }
}
void taperedObelisk(){
 for(int i=0;i<4;i++){
  float a=PI/4+PI*.5f*i,b=a+PI*.5f;
  V p{.3f*cosf(a),-.5f,.3f*sinf(a)},q{.3f*cosf(b),-.5f,.3f*sinf(b)};
  V P{.14f*cosf(a),.5f,.14f*sinf(a)},Q{.14f*cosf(b),.5f,.14f*sinf(b)};
  quad(q,p,P,Q,0);tri({0,.5f,0},Q,P,0);tri({0,-.5f,0},p,q,0);
 }
}
