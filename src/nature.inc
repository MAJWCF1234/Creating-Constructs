#include "nature_models.inc"
#include "nature_textures.inc"
bool bakeNatureMask=false;
bool natureIsBark(int material){int layer=material-22;return layer>=0&&layer<NatureTextureCount&&natureBarkLayer[layer];}
void natureVoxel(V p){int x=(int)floorf(p.x),y=(int)floorf(p.y),z=(int)floorf(p.z);if(!inb(x,y,z))return;
 int fx=std::clamp((int)((p.x-x)*4),0,3),fy=std::clamp((int)((p.y-y)*4),0,3),fz=std::clamp((int)((p.z-z)*4),0,3),bit=fx+fz*4+fy*16;
 gpu.natureMask[idx(x,y,z)*2+bit/32]|=1u<<(bit%32);
}
void stampNature(V a,V b,V c,const NatureVertex& A,const NatureVertex& B,const NatureVertex& C){
 if(!bakeNatureMask||ghostMode)return;
 int steps=std::min(32,std::max(1,(int)ceilf(std::max(len(b-a),std::max(len(c-a),len(c-b)))/.11f)));
 for(int i=0;i<=steps;i++)for(int j=0;j<=steps-i;j++){float u=i/(float)steps,v=j/(float)steps,w=1-u-v;
  float tx=A.u*w+B.u*u+C.u*v,ty=A.v*w+B.v*u+C.v*v;int ix=((int)floorf(tx*128)%128+128)%128,iy=((int)floorf(ty*128)%128+128)%128;
  if(naturePixels[A.material-22][(iy*128+ix)*4+3]>=128)natureVoxel(a*w+b*u+c*v);
 }
}
template<size_t Count>void natureModel(const NatureVertex(&mesh)[Count],V base,float scale,float angle,bool shadow=true,float width=1,bool hanging=false){
 float ca=cosf(angle),sa=sinf(angle);auto rotate=[&](V p){return V{p.x*ca+p.z*sa,p.y,-p.x*sa+p.z*ca};};
 for(size_t i=0;i<Count;i+=3){V pos[3],normal[3];for(int j=0;j<3;j++){float flip=hanging?-1.f:1.f;pos[j]=transform(base+rotate(V{mesh[i+j].p.x*width*flip,mesh[i+j].p.y*flip,mesh[i+j].p.z*width})*scale);V n=mesh[i+j].n;n.x*=flip;n.y*=flip;normal[j]=transformNormal(rotate(n));}
  // Suppress foliage buried in adjacent authored structure, preserving its openings.
  if(!natureIsBark(mesh[i].material)&&!ghostMode&&solidAt((pos[0]+pos[1]+pos[2])*(1.f/3)))continue;
  if(shadow)stampNature(pos[0],pos[1],pos[2],mesh[i],mesh[i+1],mesh[i+2]);
  for(int j=0;j<3;j++){const auto& v=mesh[i+j];vertices.push_back({pos[j],normal[j],(float)(ghostMode?10:v.material),ambient,v.u,v.v});}
 }
}
uint32_t natureSeed(){return (uint32_t)((int)(origin.x*2)*73856093u)^((int)(origin.z*2)*19349663u)^((int)(origin.y*2)*83492791u)^(uint32_t)(turn*2654435761u);}
int natureTreeVariant(uint32_t seed){IvyRandom rng{seed+59};return std::min(7,(int)(rng.next()*8));}
void natureTree(){
 uint32_t seed=natureSeed();IvyRandom rng{seed+59};
 float scale=rng.range(.64f,.77f),angle=rng.range(0,2*PI);
 switch(natureTreeVariant(seed)){
 case 0:case 1:natureModel(nature_tree_5,{0,-.5f,0},scale,angle,true,1.25f);break;
 case 2:case 3:natureModel(nature_tree_6,{0,-.5f,0},scale,angle,true,1.25f);break;
 case 4:case 5:natureModel(nature_tree_7,{0,-.5f,0},scale*.86f,angle,true,1.15f);break;
 case 6:natureModel(nature_tree_8,{0,-.5f,0},scale*.9f,angle,true,1.15f);break;
 default:natureModel(nature_tree_9,{0,-.5f,0},scale*1.15f,angle,true,1.25f);break;
 }
}
void natureBirch(){
 uint32_t seed=natureSeed();IvyRandom rng{seed+71};
 float scale=rng.range(.52f,.64f),angle=rng.range(0,2*PI);
 if(rng.next()<.5f)natureModel(nature_birch_1,{0,-.5f,0},scale,angle,true,1.1f);
 else natureModel(nature_birch_2,{0,-.5f,0},scale*1.15f,angle,true,1.15f);
}
void naturePine(){
 uint32_t seed=natureSeed();IvyRandom rng{seed+83};
 float scale=rng.range(.52f,.64f),angle=rng.range(0,2*PI);
 if(rng.next()<.5f)natureModel(nature_pine_1,{0,-.5f,0},scale,angle,true,1.15f);
 else natureModel(nature_pine_2,{0,-.5f,0},scale,angle,true,1.15f);
}
void natureLog(){IvyRandom rng{natureSeed()+91};natureModel(nature_log,{0,-.35f,0},rng.range(.85f,1.05f),rng.range(0,2*PI),true,1.f);}
void natureStump(){IvyRandom rng{natureSeed()+97};natureModel(nature_stump,{0,-.5f,0},rng.range(.9f,1.15f),rng.range(0,2*PI),true,1.f);}
void natureMushroom(){IvyRandom rng{natureSeed()+103};float s=rng.range(.55f,.85f),a=rng.range(0,2*PI);
 if(rng.next()<.5f)natureModel(nature_mushroom_1,{0,-.5f,0},s,a,false);else natureModel(nature_mushroom_2,{0,-.5f,0},s,a,false);}
void natureFern(int variant,V root,float scale,float angle){
 switch(variant%3){case 0:natureModel(nature_bracken,root,scale,angle,false);break;case 1:natureModel(nature_bracken_2,root,scale,angle,false);break;default:natureModel(nature_bracken_3,root,scale,angle,false);break;}
}
void natureGrass(int variant,V root,float scale,float angle,bool tall){
 if(tall){if(variant&1)natureModel(nature_tallgrass_2,root,scale,angle,false);else natureModel(nature_tallgrass,root,scale,angle,false);}
 else {if(variant&1)natureModel(nature_meadow_2,root,scale,angle,false);else natureModel(nature_meadow,root,scale,angle,false);}
}
void natureVariantMesh(int variant){
 V root{0,-.49f,0};switch(variant){
 case 0:natureModel(nature_bracken,root,1,0,false);break;case 1:natureModel(nature_bracken_2,root,1,0,false);break;case 2:natureModel(nature_bracken_3,root,1,0,false);break;
 case 3:natureModel(nature_meadow,root,1,0,false);break;case 4:natureModel(nature_meadow_2,root,1,0,false);break;case 5:natureModel(nature_tallgrass,root,1,0,false);break;case 6:natureModel(nature_tallgrass_2,root,1,0,false);break;
 case 7:natureModel(nature_dogbane,root,1,0,false);break;case 8:natureModel(nature_dogbane_2,root,1,0,false);break;case 9:natureModel(nature_clover,root,1,0,false);break;case 10:natureModel(nature_aster,root,1,0,false);break;case 11:natureModel(nature_goldenrod,root,1,0,false);break;case 12:natureModel(nature_shrub_1,root,1,0,false);break;case 13:natureModel(nature_shrub_2,root,1,0,false);break;case 14:natureModel(nature_shrub_3,root,1,0,false);break;case 15:natureModel(nature_birch_1,root,.6f,0,false);break;case 16:natureModel(nature_birch_2,root,.6f,0,false);break;case 17:natureModel(nature_pine_1,root,.6f,0,false);break;case 18:natureModel(nature_pine_2,root,.6f,0,false);break;case 19:natureModel(nature_mushroom_1,root,.7f,0,false);break;default:natureModel(nature_mushroom_2,root,.7f,0,false);break;}
}
void natureUnderstory(float height,bool planter){
 IvyRandom rng{natureSeed()^319u};
 float spread=planter?.29f:.44f;
 // Every tile gets a stable plant community; nearby tiles no longer repeat one recipe.
 int community=std::min(5,(int)(rng.next()*6));float fertility=rng.range(.38f,1.f);
 for(int i=0;i<(planter?4:9);i++){if(i>0&&rng.next()>fertility)continue;
  V root{rng.range(-spread,spread),height,rng.range(-spread,spread)};float a=rng.range(0,2*PI),s=rng.range(.72f,1.17f);
  int variant=std::min(5,(int)(rng.next()*6));
  if(i==0)natureFern(variant,root,s*(planter?.48f:.68f),a);
  else if(i==1)natureGrass(variant,root,s,a,false);
  else if(i==3||i==6){
   float h=s*(planter?.48f:.72f);
   switch(community){
   case 1:if(variant&1)natureModel(nature_dogbane,root,h,a,false);else natureModel(nature_dogbane_2,root,h,a,false);break;
   case 2:natureModel(nature_aster,root,h,a,false);break;
   case 3:natureModel(nature_goldenrod,root,h*.8f,a,false);break;
   default:natureGrass(variant,root,h,a,true);break;
   }
  }else if(community==4)natureModel(nature_clover,root,s*(planter?.48f:.68f),a,false);
  else if(community==5)natureFern(variant,root,s*(planter?.42f:.6f),a);
  else natureGrass(variant,root,s,a,false);
 }
}
// Water-edge / flooded moss plant set — reeds, lily pads, emergent fringe (reuses pack meshes).
void natureWaterUnderstory(float height){
 IvyRandom rng{natureSeed()^521u};
 int count=3+(int)(rng.next()*5);
 for(int i=0;i<count;i++){
  V root{rng.range(-.42f,.42f),height,rng.range(-.42f,.42f)};
  float a=rng.range(0,2*PI),s=rng.range(.75f,1.25f);int variant=std::min(5,(int)(rng.next()*6));
  int kind=(int)(rng.next()*4);
  if(kind==0)natureGrass(variant,root,s*rng.range(1.15f,1.55f),a,true); // reed cluster
  else if(kind==1)natureModel(nature_clover,{root.x,root.y-rng.range(.02f,.08f),root.z},s*rng.range(.95f,1.35f),a,false); // lily pad
  else if(kind==2){if(variant&1)natureModel(nature_dogbane,root,s*.7f,a,false);else natureModel(nature_dogbane_2,root,s*.7f,a,false);}
  else natureFern(variant,root,s*.55f,a);
 }
}
