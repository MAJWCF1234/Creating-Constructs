// Imported Retro Official props (tools/prepare_props.py). Unlike foliage they are never culled by
// neighbouring structure and cast shadows through their solidity proxies, not the foliage tracer.
#include "../generated/prop_models.inc"
template<size_t Count>void propModel(const NatureVertex(&mesh)[Count],V base=V{0,-.5f,0},float yaw=0,float scale=1){
 float ca=cosf(yaw),sa=sinf(yaw);auto rotate=[&](V p){return V{p.x*ca+p.z*sa,p.y,-p.x*sa+p.z*ca};};auto& out=ghostMode?transparent:vertices;
 for(size_t i=0;i<Count;i++){const auto& v=mesh[i];V p=transform(base+rotate(v.p)*scale);V n=transformNormal(rotate(v.n));out.push_back({p,n,(float)(ghostMode?ghostMaterial:v.material),ambient,v.u,v.v});}
}
