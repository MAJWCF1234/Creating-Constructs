"""Inset pool water inside walls; thin catalog water; less metallic waterM."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

# emitIsland rim / water y
s2 = s.replace("rim=.40,wallT=.12,wallH=.5,yo=-.25", "wallT=.12,wallH=.5,yo=-.25,rim=.5-wallT-.06", 1)
if s2 == s:
    if "rim=.5-wallT-.06" in s:
        print("rim already inset")
    else:
        raise SystemExit("rim assignment not found")
else:
    s = s2
    print("rim inset")

s2 = s.replace("else addWaterPlane(g,w,d,cx,-.14,cz);", "else if(w>.05&&d>.05)addWaterPlane(g,w,d,cx,-.18,cz);", 1)
if s2 == s:
    if "cx,-.18,cz" in s:
        print("water y already")
    else:
        raise SystemExit("addWaterPlane call not found")
else:
    s = s2
    print("water y lowered")

old_pool = (
    "[[mg([bx(1,.5,.1,0,-.25,-.45),bx(1,.5,.1,0,-.25,.45),bx(.1,.5,.8,-.45,-.25,0),"
    "bx(.1,.5,.8,.45,-.25,0),bx(.8,.1,.8,0,-.45,0)]),conc],[bx(.8,.02,.8,0,-.14,0),waterM]]"
)
new_pool = (
    "[[mg([bx(1,.5,.12,0,-.25,-.44),bx(1,.5,.12,0,-.25,.44),bx(.12,.5,1,-.44,-.25,0),"
    "bx(.12,.5,1,.44,-.25,0),bx(1,.08,1,0,-.46,0)]),conc],[bx(.64,.001,.64,0,-.18,0),waterM]]"
)
if old_pool in s:
    s = s.replace(old_pool, new_pool, 1)
    print("catalog pool")
elif "bx(.64,.001,.64,0,-.18,0),waterM" in s:
    print("catalog pool already")
else:
    print("WARN: catalog pool pattern missing")

old_wm = "waterM=new THREE.MeshStandardMaterial({color:0x2b4a52,roughness:.04,metalness:.4,transparent:true,opacity:.85})"
new_wm = "waterM=new THREE.MeshStandardMaterial({color:0x3a6a72,roughness:.12,metalness:.08,transparent:true,opacity:.78,depthWrite:false})"
if old_wm in s:
    s = s.replace(old_wm, new_wm, 1)
    print("waterM")
elif "metalness:.08" in s and "waterM=" in s:
    print("waterM already")
else:
    print("WARN: waterM not found")

HTML.write_text(s, encoding="utf-8")
print("done")
