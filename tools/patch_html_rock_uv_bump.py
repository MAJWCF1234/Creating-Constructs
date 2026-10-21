"""Double rock UVs and strengthen topo bump on HTML rock nets."""
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"
s = HTML.read_text(encoding="utf-8")

a = (
    "rockM=new THREE.MeshStandardMaterial({map:TOPO_ROCK_BROWN_SANDSTONE,"
    "bumpMap:TOPO_ROCK_BROWN_SANDSTONE,bumpScale:.4,color:0xf0e6d8,roughness:.95,flatShading:true})"
)
b = (
    "rockM=new THREE.MeshStandardMaterial({map:TOPO_ROCK_BROWN_SANDSTONE,"
    "bumpMap:TOPO_ROCK_BROWN_SANDSTONE,bumpScale:1.85,color:0xf0e6d8,roughness:.92,flatShading:false})"
)
if a not in s:
    raise SystemExit("rockM miss")
s = s.replace(a, b, 1)

old_uv = """   if(ay>=ax&&ay>=az){uv[i*2]=x;uv[i*2+1]=z;}
   else if(ax>=az){uv[i*2]=z;uv[i*2+1]=y;}
   else{uv[i*2]=x;uv[i*2+1]=y;}"""
new_uv = """   if(ay>=ax&&ay>=az){uv[i*2]=x*2;uv[i*2+1]=z*2;}
   else if(ax>=az){uv[i*2]=z*2;uv[i*2+1]=y*2;}
   else{uv[i*2]=x*2;uv[i*2+1]=y*2;}"""
if old_uv not in s:
    if "uv[i*2]=x*2" in s:
        print("uv already doubled")
    else:
        raise SystemExit("uv miss")
else:
    s = s.replace(old_uv, new_uv, 1)

old2 = (
    " if(P.rock&&m.map){m.map.wrapS=m.map.wrapT=THREE.RepeatWrapping;"
    "m.map.magFilter=THREE.NearestFilter;m.map.minFilter=THREE.NearestFilter;"
    "m.map.needsUpdate=true;m.color.set(0xffffff);}"
)
new2 = (
    " if(P.rock&&m.map){m.map.wrapS=m.map.wrapT=THREE.RepeatWrapping;m.map.repeat.set(2,2);"
    "m.map.magFilter=THREE.NearestFilter;m.map.minFilter=THREE.NearestFilter;m.map.needsUpdate=true;"
    "if(m.bumpMap){m.bumpMap.wrapS=m.bumpMap.wrapT=THREE.RepeatWrapping;m.bumpMap.repeat.set(2,2);"
    "m.bumpMap.magFilter=THREE.NearestFilter;m.bumpMap.minFilter=THREE.NearestFilter;m.bumpMap.needsUpdate=true;}"
    "m.color.set(0xf0e6d8);}"
)
if old2 not in s:
    if "m.map.repeat.set(2,2)" in s:
        print("repeat already")
    else:
        raise SystemExit("map wrap miss")
else:
    s = s.replace(old2, new2, 1)

tmp = HTML.with_name("Creating Constructs.html.uvbump_tmp")
tmp.write_text(s, encoding="utf-8")
tmp.replace(HTML)
print("patched rock uv+bump")
