"""Restore stable HTML rock nets (ellipsoid/smoothMin exploded meshes)."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

s = s.replace(
    " rock:{blend:.48,relief:.62,bevel:.018,sideWarp:.55,rock:true,mat:null}",
    " rock:{blend:.28,relief:.30,bevel:.08,sideWarp:.26,rock:true,mat:null}",
    1,
)

old_rb = """function rockHash(x,y,z,s){const v=Math.sin((x*73+y*31+z*17+s)*12.9898)*43758.5453;return v-Math.floor(v)}
function roundBox(px,py,pz,x,y,z,t,P){
 if(P.rock){
  let hx=.38+.14*rockHash(x,y,z,1),hy=.34+.16*rockHash(x,y,z,2),hz=.40+.14*rockHash(x,y,z,3);
  if(t===64){hx+=.08;hy+=.06;hz+=.10}
  const qx=(px-(x+.5))/hx,qy=(py-(y+.5))/hy,qz=(pz-(z+.5))/hz;
  let d=Math.hypot(qx,qy,qz)-1;
  const n=Math.sin(px*6.2+pz*4.1)*Math.cos(py*5.3+px*2.7)*.11+Math.sin(px*11-pz*9)*.06+Math.cos((px+pz)*3.3+py*2.1)*.05;
  d+=n*(t===64?.85:.7);
  const under=Math.max(0,(y+.12)-py);
  return smoothMin(d,under-.02,.08);
 }
 let bevel=P.bevel,half=.5-bevel;
 const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
 return Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
}"""

new_rb = """function roundBox(px,py,pz,x,y,z,t,P){
 let bevel=P.bevel,half=.5-bevel;if(t===64)half=.46-bevel*.3;
 const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
 let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 if(P.rock){d+=Math.sin(px*2.8+pz*1.9)*Math.cos(py*2.2)*.035+Math.sin(px*5.1-pz*4.4)*.02;if(t===64)d-=.04}
 return d;
}"""

if old_rb not in s:
    raise SystemExit("roundBox block not found")
s = s.replace(old_rb, new_rb, 1)

old_def = """ if(P.rock){
  ridge=Math.sin(px*2.4+pz*1.1)*.85+Math.cos(pz*3.8-px*1.7)*.55+Math.sin(px*8.4+pz*7.1)*.38+Math.cos(px*13-pz*11)*.18;
  swell=Math.sin(px*1.05-pz*.9)*Math.cos(pz*1.4)*.55;
  height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.05)/.9));
  return [px+P.sideWarp*P.relief*Math.sin(py*3.4+pz*2.6+px)*.9*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*3.1+px*2.2-pz)*.9*topWeight];
 }"""
new_def = """ if(P.rock){
  ridge=Math.sin(px*2.6+pz*1.2)*.55+Math.cos(pz*3.5-px*1.6)*.32+Math.sin(px*5.8+pz*5.1)*.16;
  swell=Math.sin(px*1.1-pz*.85)*Math.cos(pz*1.35)*.32;
  height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.1)/.8));
  return [px+P.sideWarp*P.relief*Math.sin(py*3.2+pz*2.4)*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*2.9+px*2.1)*topWeight];
 }"""
if old_def not in s:
    raise SystemExit("deform not found")
s = s.replace(old_def, new_def, 1)

tmp = HTML.with_name("Creating Constructs.html.rockfix_tmp")
tmp.write_text(s, encoding="utf-8")
tmp.replace(HTML)
print("patched stable rock nets")
