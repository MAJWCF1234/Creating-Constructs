"""HTML rock nets: Blender-style anisotropic slabs that fuse at contacts."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

old_prof = " rock:{blend:.06,relief:.0,bevel:.045,sideWarp:.0,rock:true,mat:null}"
new_prof = " rock:{blend:.16,relief:.0,bevel:.08,sideWarp:.0,rock:true,mat:null}"
if old_prof in s:
    s = s.replace(old_prof, new_prof, 1)
elif new_prof not in s:
    raise SystemExit("rock profile not found")

old_rb = """function roundBox(px,py,pz,x,y,z,t,P){
 let bevel=P.bevel,half=.5-bevel;if(t===64){bevel=.10;half=.44}
 const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
 let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 if(P.rock&&t===64)d-=.02;
 return d;
}"""
new_rb = """function rockHash(x,y,z,s){const v=Math.sin((x*73+y*31+z*17+s)*12.9898)*43758.5453;return v-Math.floor(v)}
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
if old_rb not in s:
    if "function rockHash" in s and ".72*box+.28*octa" in s:
        print("roundBox already blender")
    else:
        raise SystemExit("roundBox not found")
else:
    s = s.replace(old_rb, new_rb, 1)

old_def = """ if(P.rock){
  ridge=Math.sin(px*2.2+pz*1.0)*.35+Math.cos(pz*2.8-px*1.3)*.22;
  swell=Math.sin(px*.9-pz*.7)*Math.cos(pz*1.1)*.2;
  height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.15)/.7));
  return [px+P.sideWarp*P.relief*Math.sin(py*2.6+pz*2.0)*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*2.4+px*1.8)*topWeight];
 }"""
new_def = """ if(P.rock)return [px,py,pz];"""
if old_def in s:
    s = s.replace(old_def, new_def, 1)
elif "if(P.rock)return [px,py,pz];" not in s:
    raise SystemExit("deform not found")

# Albedo only — bump was the fleshy shimmer
s = s.replace(
    "bumpMap:TOPO_ROCK_LIMESTONE,bumpScale:1.15,",
    "bumpMap:null,bumpScale:0,",
    1,
)

tmp = HTML.with_name("Creating Constructs.html.blender_tmp")
tmp.write_text(s, encoding="utf-8")
tmp.replace(HTML)
print("patched blender rock nets")
