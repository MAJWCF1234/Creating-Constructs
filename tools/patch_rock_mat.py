"""Replace concrete-atlas rock/dirt materials with seamless procedural look."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

old = "rockM=new THREE.MeshStandardMaterial({map:tex,bumpMap:tex,bumpScale:2.1,color:0x8f8a82,roughness:.97})"
helper = (
    'const ROCK_TEX=(function(){const c=document.createElement("canvas");c.width=c.height=128;'
    "const g=c.getContext(\"2d\"),id=g.createImageData(128,128);"
    "for(let y=0;y<128;y++)for(let x=0;x<128;x++){const i=(y*128+x)*4;"
    "const n1=Math.sin(x*.17+y*.11)*Math.cos(x*.09-y*.19)*.5+.5;"
    "const n2=((Math.sin(x*1.7+y*2.3)*43758.5453)%1+1)%1;"
    "const n3=((Math.sin(x*.41+y*.73+11)*24571.3)%1+1)%1;"
    "const vein=Math.abs(((x*1.7+y*2.1)/128)%1-.5)*2;"
    "let v=.42+n1*.18+n2*.12+n3*.08-vein*.06;v=Math.max(.05,Math.min(.95,v));"
    "id.data[i]=(v*145+35)|0;id.data[i+1]=(v*136+34)|0;id.data[i+2]=(v*124+32)|0;id.data[i+3]=255}"
    "g.putImageData(id,0,0);const t=new THREE.CanvasTexture(c);"
    "t.wrapS=t.wrapT=THREE.RepeatWrapping;t.repeat.set(2.2,2.2);t.needsUpdate=true;return t})();\n"
)
new = "rockM=new THREE.MeshStandardMaterial({map:ROCK_TEX,bumpMap:ROCK_TEX,bumpScale:1.6,color:0xc4beb6,roughness:.97,flatShading:true})"

if old not in s:
    if "ROCK_TEX=" in s and "rockM=new THREE.MeshStandardMaterial({map:ROCK_TEX" in s:
        print("HTML rock already patched")
    else:
        raise SystemExit("rockM definition not found")
else:
    s = s.replace(old, helper + new, 1)
    print("patched rockM")

old_d = "dirtM=new THREE.MeshStandardMaterial({map:tex,bumpMap:tex,bumpScale:1.9,color:0x735438,roughness:.99})"
new_d = "dirtM=new THREE.MeshStandardMaterial({color:0x6b4e32,roughness:.99,flatShading:true})"
if old_d in s:
    s = s.replace(old_d, new_d, 1)
    print("patched dirtM")

HTML.write_text(s, encoding="utf-8")
print("done")
