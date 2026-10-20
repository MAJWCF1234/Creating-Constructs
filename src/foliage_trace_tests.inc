void foliageTraceSelfTest(){
 check(foliageRecords.size()*sizeof(F4)<=foliageCapacity,"water foliage acceleration data fits its GPU allocation");
 bool valid=true;size_t coverage=0;
 for(size_t i=0;i<foliageNodes.size();i++){auto node=foliageNodes[i];valid=valid&&node.escape>i&&node.escape<=foliageNodes.size();int count=node.packed&7;size_t first=node.packed>>3;
  if(!count){valid=valid&&i+1<foliageNodes.size();continue;}coverage+=count;valid=valid&&count<=4&&first+count<=foliageTriangles.size();
  for(size_t j=first;j<first+count&&j<foliageTriangles.size();j++)for(auto v:foliageTriangles[j].v){valid=valid&&v.p.x-.014f>=node.lo.x&&v.p.x+.014f<=node.hi.x&&v.p.y>=node.lo.y&&v.p.y<=node.hi.y&&v.p.z-.0056f>=node.lo.z&&v.p.z+.0056f<=node.hi.z;}
 }
 check(valid&&coverage==foliageTriangles.size()&&coverage>0,"water BVH covers every plant triangle with valid escape links and wind bounds");
 bool leaves=false,bark=false;for(auto t:foliageTriangles){leaves=leaves||t.v[0].m==22;bark=bark||t.v[0].m==23;}
 check(leaves&&bark,"water rays receive both leaf cutouts and actual branch meshes");
 check(foliageRecords.size()==1+foliageNodes.size()*2+foliageTriangles.size()*5+350,"water foliage records match the shader storage layout");
}
