"""Crush HTML rock net deform — warp was shredding stone/boulder into sheets."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

old_prof = " rock:{blend:.28,relief:.30,bevel:.08,sideWarp:.26,rock:true,mat:null}"
new_prof = " rock:{blend:.22,relief:.0,bevel:.14,sideWarp:.0,rock:true,mat:null}"
if old_prof not in s:
    # already patched or different
    if new_prof in s:
        print("profile already solid")
    else:
        raise SystemExit("rock profile not found")
else:
    s = s.replace(old_prof, new_prof, 1)

old_rb = """function roundBox(px,py,pz,x,y,z,t,P){
 let bevel=P.bevel,half=.5-bevel;if(t===64)half=.46-bevel*.3;
 const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
 let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 if(P.rock){d+=Math.sin(px*2.8+pz*1.9)*Math.cos(py*2.2)*.035+Math.sin(px*5.1-pz*4.4)*.02;if(t===64)d-=.04}
 return d;
}"""
new_rb = """function roundBox(px,py,pz,x,y,z,t,P){
 let bevel=P.bevel,half=.5-bevel;if(t===64){bevel=.18;half=.42}
 const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;
 let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;
 if(P.rock){d+=Math.sin(px*2.1+pz*1.6)*Math.cos(py*1.8)*.018+Math.sin(px*3.7-pz*3.1)*.01;if(t===64)d-=.03}
 return d;
}"""
if old_rb not in s:
    if "if(t===64){bevel=.18;half=.42}" in s:
        print("roundBox already solid")
    else:
        raise SystemExit("roundBox not found")
else:
    s = s.replace(old_rb, new_rb, 1)

old_def = """ if(P.rock){
  ridge=Math.sin(px*2.6+pz*1.2)*.55+Math.cos(pz*3.5-px*1.6)*.32+Math.sin(px*5.8+pz*5.1)*.16;
  swell=Math.sin(px*1.1-pz*.85)*Math.cos(pz*1.35)*.32;
  height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.1)/.8));
  return [px+P.sideWarp*P.relief*Math.sin(py*3.2+pz*2.4)*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*2.9+px*2.1)*topWeight];
 }"""
new_def = """ if(P.rock){
  ridge=Math.sin(px*2.2+pz*1.0)*.35+Math.cos(pz*2.8-px*1.3)*.22;
  swell=Math.sin(px*.9-pz*.7)*Math.cos(pz*1.1)*.2;
  height=(ridge+swell)*P.relief;topWeight=Math.max(0,Math.min(1,(fy-.15)/.7));
  return [px+P.sideWarp*P.relief*Math.sin(py*2.6+pz*2.0)*topWeight,py-height*topWeight,pz+P.sideWarp*P.relief*Math.cos(py*2.4+px*1.8)*topWeight];
 }"""
if old_def not in s:
    if "ridge=Math.sin(px*2.2+pz*1.0)*.35" in s:
        print("deform already solid")
    else:
        raise SystemExit("deform not found")
else:
    s = s.replace(old_def, new_def, 1)

tmp = HTML.with_name("Creating Constructs.html.rocksolid_tmp")
tmp.write_text(s, encoding="utf-8")
tmp.replace(HTML)
print("patched solid rock nets")
