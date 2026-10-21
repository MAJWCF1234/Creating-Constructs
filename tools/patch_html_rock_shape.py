"""Match HTML rock surface nets to native ellipsoid/chunky deform."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

old_prof = " rock:{blend:.22,relief:.34,bevel:.045,sideWarp:.22,rock:true,mat:null}"
new_prof = " rock:{blend:.48,relief:.62,bevel:.018,sideWarp:.55,rock:true,mat:null}"
if old_prof in s:
    s = s.replace(old_prof, new_prof, 1)
    print("profile")
elif "relief:.62" in s and "rock:{" in s:
    print("profile already")
else:
    raise SystemExit("rock profile not found")

old_rb = """function roundBox(px,py,pz,x,y,z,t,P){
 let bevel=P.bevel,half=.5-bevel;if(t===64)half=.48-bevel*.35;
 const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
 let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 if(t===64)d-=.06;return d;
}"""
new_rb = """function rockHash(x,y,z,s){const v=Math.sin((x*73+y*31+z*17+s)*12.9898)*43758.5453;return v-Math.floor(v)}
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
if old_rb in s:
    s = s.replace(old_rb, new_rb, 1)
    print("roundBox")
elif "function rockHash" in s:
    print("roundBox already")
else:
    raise SystemExit("roundBox not found")

old_def = """ if(P.rock){
  ridge=Math.sin(px*3.1+pz*1.4)*.7+Math.cos(pz*4.6-px*2.2)*.45+Math.sin(px*7.1+pz*6.3)*.28;
  swell=Math.sin(px*1.3-pz*1.1)*Math.cos(pz*1.7)*.4;
  height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.08)/.85));
  return [px+P.sideWarp*P.relief*Math.sin(py*4.1+pz*3.2)*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*3.7+px*2.8)*topWeight];
 }"""
new_def = """ if(P.rock){
  ridge=Math.sin(px*2.4+pz*1.1)*.85+Math.cos(pz*3.8-px*1.7)*.55+Math.sin(px*8.4+pz*7.1)*.38+Math.cos(px*13-pz*11)*.18;
  swell=Math.sin(px*1.05-pz*.9)*Math.cos(pz*1.4)*.55;
  height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.05)/.9));
  return [px+P.sideWarp*P.relief*Math.sin(py*3.4+pz*2.6+px)*.9*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*3.1+px*2.2-pz)*.9*topWeight];
 }"""
if old_def in s:
    s = s.replace(old_def, new_def, 1)
    print("deform")
elif "px*2.4+pz*1.1" in s:
    print("deform already")
else:
    raise SystemExit("deform not found")

tmp = HTML.with_name("Creating Constructs.html.rock_tmp")
tmp.write_text(s, encoding="utf-8")
tmp.replace(HTML)
print("done")
