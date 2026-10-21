"""Tiny-Glade HTML rocks: fill voxels, hard union, faceted cleavage + warm topo."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

# Profile
for a, b in [
    (
        " rock:{blend:.12,relief:.0,bevel:.055,sideWarp:.0,rock:true,mat:null}",
        " rock:{blend:.0,relief:.0,bevel:.02,sideWarp:.0,rock:true,mat:null}",
    ),
    (
        "rockM=new THREE.MeshStandardMaterial({map:TOPO_ROCK_LIMESTONE,bumpMap:TOPO_ROCK_LIMESTONE,bumpScale:.35,color:0xffffff,roughness:.92,flatShading:true})",
        "rockM=new THREE.MeshStandardMaterial({map:TOPO_ROCK_BROWN_SANDSTONE,bumpMap:TOPO_ROCK_BROWN_SANDSTONE,bumpScale:.4,color:0xf0e6d8,roughness:.95,flatShading:true})",
    ),
]:
    if a in s:
        s = s.replace(a, b, 1)
    elif b in s:
        pass
    else:
        # try prior limestone-only variant
        a2 = "rockM=new THREE.MeshStandardMaterial({map:TOPO_ROCK_LIMESTONE,bumpMap:null,bumpScale:0,color:0xffffff,roughness:.92,flatShading:true})"
        if a2 in s:
            s = s.replace(a2, b, 1)
        elif "rock:{blend:.0" not in s and "rock:{blend" in a:
            raise SystemExit(f"missing:\n{a[:90]}")

old_rb = None
# Find current roundBox rock section via markers
start = s.find("function rockHash(x,y,z,s)")
if start < 0:
    raise SystemExit("rockHash missing")
end = s.find("function smoothMin", start)
if end < 0:
    raise SystemExit("smoothMin missing")
new_rb = """function rockHash(x,y,z,s){const v=Math.sin((x*73+y*31+z*17+s)*12.9898)*43758.5453;return v-Math.floor(v)}
function roundBox(px,py,pz,x,y,z,t,P){
 if(!P.rock){
  let bevel=P.bevel,half=.5-bevel;
  const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
  return Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 }
 let hx=.44+.04*rockHash(x,y,z,1),hy=.42+.05*rockHash(x,y,z,2),hz=.44+.04*rockHash(x,y,z,3);
 let bevel=.018+.012*rockHash(x,y,z,4);
 if(t===64){hx=Math.min(.48,hx+.03);hy=Math.min(.48,hy+.03);hz=Math.min(.48,hz+.03);bevel+=.01}
 const cx=x+.5,cy=y+.5,cz=z+.5;
 const qx=Math.abs(px-cx)-hx,qy=Math.abs(py-cy)-hy,qz=Math.abs(pz-cz)-hz;
 let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 for(let i=0;i<3;i++){
  let ax=rockHash(x,y,z,10+i*3)*2-1,ay=rockHash(x,y,z,11+i*3)*1.4-.4,az=rockHash(x,y,z,12+i*3)*2-1;
  const inv=1/Math.max(.35,Math.hypot(ax,ay,az));ax*=inv;ay*=inv;az*=inv;
  const cut=.18+.14*rockHash(x,y,z,20+i);
  d=Math.max(d,(px-cx)*ax+(py-cy)*ay+(pz-cz)*az-cut);
 }
 return d;
}
"""
s = s[:start] + new_rb + s[end:]

# Hard min for rock blends
old_blend = "    value=man===1?smoothMin(value,other.d,P.blend):Math.min(value,other.d);"
new_blend = "    if(man===1&&!P.rock&&P.blend>.001)value=smoothMin(value,other.d,P.blend);else value=Math.min(value,other.d);"
if old_blend not in s:
    if new_blend in s:
        print("blend already hard")
    else:
        raise SystemExit("blend line missing")
else:
    s = s.replace(old_blend, new_blend, 1)

tmp = HTML.with_name("Creating Constructs.html.glade_tmp")
tmp.write_text(s, encoding="utf-8")
tmp.replace(HTML)
print("patched glade rocks")
