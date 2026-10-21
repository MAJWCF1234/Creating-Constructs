"""HTML: hard rock SDF + UVs for topo texture; match native wind-safe look."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

repls = [
    (
        " rock:{blend:.16,relief:.0,bevel:.08,sideWarp:.0,rock:true,mat:null}",
        " rock:{blend:.12,relief:.0,bevel:.055,sideWarp:.0,rock:true,mat:null}",
    ),
]

old_rb = """function rockHash(x,y,z,s){const v=Math.sin((x*73+y*31+z*17+s)*12.9898)*43758.5453;return v-Math.floor(v)}
function roundBox(px,py,pz,x,y,z,t,P){
 if(!P.rock){
  let bevel=P.bevel,half=.5-bevel;
  const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
  return Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 }
 let hx=.34+.11*rockHash(x,y,z,1),hy=.30+.13*rockHash(x,y,z,2),hz=.36+.11*rockHash(x,y,z,3);
 let bevel=.06+.04*rockHash(x,y,z,4);
 if(t===64){hx+=.07;hy+=.06;hz+=.08;bevel+=.04}
 hx=Math.min(hx,.47);hy=Math.min(hy,.47);hz=Math.min(hz,.47);
 const cx=x+.5,cy=y+.5,cz=z+.5;
 const qx=Math.abs(px-cx)-hx,qy=Math.abs(py-cy)-hy,qz=Math.abs(pz-cz)-hz;
 const box=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 const octa=(Math.abs(px-cx)+Math.abs(py-cy)+Math.abs(pz-cz))*.58-(t===64?.48:.42);
 return .72*box+.28*octa;
}"""

new_rb = """function rockHash(x,y,z,s){const v=Math.sin((x*73+y*31+z*17+s)*12.9898)*43758.5453;return v-Math.floor(v)}
function roundBox(px,py,pz,x,y,z,t,P){
 if(!P.rock){
  let bevel=P.bevel,half=.5-bevel;
  const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
  return Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 }
 let hx=.36+.09*rockHash(x,y,z,1),hy=.33+.10*rockHash(x,y,z,2),hz=.37+.09*rockHash(x,y,z,3);
 let bevel=.04+.03*rockHash(x,y,z,4);
 if(t===64){hx+=.05;hy+=.04;hz+=.06;bevel+=.025}
 hx=Math.min(hx,.48);hy=Math.min(hy,.48);hz=Math.min(hz,.48);
 const cx=x+.5+(.04*rockHash(x,y,z,5)-.02),cy=y+.5,cz=z+.5+(.04*rockHash(x,y,z,6)-.02);
 const qx=Math.abs(px-cx)-hx,qy=Math.abs(py-cy)-hy,qz=Math.abs(pz-cz)-hz;
 const box=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 const nx=rockHash(x,y,z,7)*2-1,nz=rockHash(x,y,z,8)*2-1;
 const plane=(px-cx)*nx*.55+(pz-cz)*nz*.55+(py-cy)*.35-(t===64?.28:.22);
 return Math.max(box,plane);
}"""

if old_rb not in s:
    if "Math.max(box,plane)" in s:
        print("roundBox already fixed")
    else:
        raise SystemExit("roundBox block missing")
else:
    s = s.replace(old_rb, new_rb, 1)

for a, b in repls:
    if a in s:
        s = s.replace(a, b, 1)

# Project UVs so topo map actually shows on rock nets
old_build = """ if(!indices.length)return;
 const geo=new THREE.BufferGeometry();
 geo.setAttribute('position',new THREE.Float32BufferAttribute(positions,3));
 geo.setIndex(indices);geo.computeVertexNormals();
 const m=mat.clone();m.flatShading=true;m.side=THREE.DoubleSide;if(!P.rock)foliageWrap(m);
 holder.mesh=new THREE.Mesh(geo,m);holder.mesh.castShadow=true;holder.mesh.receiveShadow=true;S.add(holder.mesh);
}"""
new_build = """ if(!indices.length)return;
 const geo=new THREE.BufferGeometry();
 geo.setAttribute('position',new THREE.Float32BufferAttribute(positions,3));
 geo.setIndex(indices);geo.computeVertexNormals();
 if(P.rock){
  const pos=geo.attributes.position,nor=geo.attributes.normal,uv=new Float32Array(pos.count*2);
  for(let i=0;i<pos.count;i++){
   const x=pos.getX(i),y=pos.getY(i),z=pos.getZ(i);
   const ax=Math.abs(nor.getX(i)),ay=Math.abs(nor.getY(i)),az=Math.abs(nor.getZ(i));
   if(ay>=ax&&ay>=az){uv[i*2]=x;uv[i*2+1]=z;}
   else if(ax>=az){uv[i*2]=z;uv[i*2+1]=y;}
   else{uv[i*2]=x;uv[i*2+1]=y;}
  }
  geo.setAttribute('uv',new THREE.Float32BufferAttribute(uv,2));
 }
 const m=mat.clone();m.flatShading=true;m.side=THREE.DoubleSide;if(!P.rock)foliageWrap(m);
 if(P.rock&&m.map){m.map.wrapS=m.map.wrapT=THREE.RepeatWrapping;m.map.magFilter=THREE.NearestFilter;m.map.minFilter=THREE.NearestFilter;m.map.needsUpdate=true;m.color.set(0xffffff);}
 holder.mesh=new THREE.Mesh(geo,m);holder.mesh.castShadow=true;holder.mesh.receiveShadow=true;S.add(holder.mesh);
}"""
if old_build not in s:
    if "if(P.rock){" in s and "geo.setAttribute('uv'" in s:
        print("UV build already")
    else:
        raise SystemExit("buildFamily tail missing")
else:
    s = s.replace(old_build, new_build, 1)

# Ensure rock material uses topo map with nearest (chunky)
s = s.replace(
    "rockM=new THREE.MeshStandardMaterial({map:TOPO_ROCK_LIMESTONE,bumpMap:null,bumpScale:0,color:0xffffff,roughness:.92,flatShading:true})",
    "rockM=new THREE.MeshStandardMaterial({map:TOPO_ROCK_LIMESTONE,bumpMap:TOPO_ROCK_LIMESTONE,bumpScale:.35,color:0xffffff,roughness:.92,flatShading:true})",
    1,
)

tmp = HTML.with_name("Creating Constructs.html.fix_tmp")
tmp.write_text(s, encoding="utf-8")
tmp.replace(HTML)
print("patched rock fix")
