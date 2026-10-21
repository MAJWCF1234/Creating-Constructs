"""Kill undulating HTML rock nets + tone bump."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

repls = [
    (
        " rock:{blend:.22,relief:.0,bevel:.14,sideWarp:.0,rock:true,mat:null}",
        " rock:{blend:.06,relief:.0,bevel:.045,sideWarp:.0,rock:true,mat:null}",
    ),
    (
        " let bevel=P.bevel,half=.5-bevel;if(t===64){bevel=.18;half=.42}\n"
        " const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;\n"
        " let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;\n"
        " if(P.rock){d+=Math.sin(px*2.1+pz*1.6)*Math.cos(py*1.8)*.018+Math.sin(px*3.7-pz*3.1)*.01;if(t===64)d-=.03}\n"
        " return d;",
        " let bevel=P.bevel,half=.5-bevel;if(t===64){bevel=.10;half=.44}\n"
        " const qx=Math.abs(px-(x+.5))-half,qy=Math.abs(py-(y+.5))-half,qz=Math.abs(pz-(z+.5))-half;\n"
        " let d=Math.hypot(Math.max(qx,0),Math.max(qy,0),Math.max(qz,0))+Math.min(Math.max(qx,qy,qz),0)-bevel;\n"
        " if(P.rock&&t===64)d-=.02;\n"
        " return d;",
    ),
    (
        "bumpMap:TOPO_ROCK_LIMESTONE,bumpScale:2.8,",
        "bumpMap:TOPO_ROCK_LIMESTONE,bumpScale:1.15,",
    ),
]

for a, b in repls:
    if a not in s:
        if b in s:
            continue
        raise SystemExit(f"missing block:\n{a[:80]}")
    s = s.replace(a, b, 1)

tmp = HTML.with_name("Creating Constructs.html.flat_tmp")
tmp.write_text(s, encoding="utf-8")
tmp.replace(HTML)
print("patched flat rock")
