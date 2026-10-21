"""Apply topography textures to Creating Constructs.html (retry after locked write)."""
from pathlib import Path
import re
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SRC = Path(r"O:\Assets\Textures\8_topography")
HTML = ROOT / "Creating Constructs.html"

PICKS = [
    ("rock_sandstone", "1_stone/mossy_faded_orange_sandstone_2a06a.png"),
    ("rock_sandstone_top", "1_stone/mossy_faded_orange_sandstone_top_35w73.png"),
    ("rock_limestone", "1_stone/light_limestone_boulders_pufrc.png"),
    ("rock_limestone_mossy", "1_stone/mossy_light_limestone_boulders_194qi.png"),
    ("rock_granite", "1_stone/dark_gray_granite_d9p2d.png"),
    ("rock_brown_sandstone", "1_stone/brown_sandstone_6q5oe.png"),
    ("rock_schist_mossy", "1_stone/mossy_blue_schist_0y61i.png"),
    ("dirt_earth", "5_dirt/brown_earth_118xl.png"),
    ("dirt_rocky", "5_dirt/rocky_soil_mkrqh.png"),
    ("dirt_mossy", "5_dirt/mossy_soil_m41dz.png"),
    ("moss_thick", "4_grass_leaves/thick_moss_rdb9p.png"),
    ("gravel_gray", "6_debris_gravel/gray_gravel_0jd6n.png"),
    ("gravel_stones", "6_debris_gravel/small_stones_0yoa9.png"),
    ("gravel_mossy", "6_debris_gravel/mossy_sandstone_debris_1dib6.png"),
]


def js_texture(name: str, im128: Image.Image) -> str:
    im32 = im128.resize((32, 32), Image.Resampling.NEAREST)
    arr = ",".join(str(b) for b in im32.tobytes())
    return (
        f"const TOPO_{name.upper()}=(function(){{"
        f"const s=document.createElement('canvas');s.width=s.height=32;"
        f"const sg=s.getContext('2d'),sid=sg.createImageData(32,32);sid.data.set([{arr}]);sg.putImageData(sid,0,0);"
        f"const c=document.createElement('canvas');c.width=c.height=128;const g=c.getContext('2d');"
        f"g.imageSmoothingEnabled=false;g.drawImage(s,0,0,128,128);"
        f"const t=new THREE.CanvasTexture(c);t.magFilter=THREE.NearestFilter;t.minFilter=THREE.NearestFilter;"
        f"t.wrapS=t.wrapT=THREE.RepeatWrapping;t.repeat.set(1.15,1.15);t.needsUpdate=true;return t}})();"
    )


def main() -> None:
    js_parts = []
    for name, rel in PICKS:
        im = Image.open(SRC / rel).convert("RGBA").resize((128, 128), Image.Resampling.NEAREST)
        js_parts.append(js_texture(name, im))
    rock_js = (
        "\n".join(js_parts)
        + "\n"
        + "rockM=new THREE.MeshStandardMaterial({map:TOPO_ROCK_LIMESTONE,bumpMap:TOPO_ROCK_LIMESTONE,"
        "bumpScale:2.8,color:0xffffff,roughness:.92,flatShading:true}),"
        "dirtM=new THREE.MeshStandardMaterial({map:TOPO_DIRT_EARTH,bumpMap:TOPO_DIRT_EARTH,"
        "bumpScale:2.0,color:0xffffff,roughness:.98,flatShading:true}),"
        "topoMossM=new THREE.MeshStandardMaterial({map:TOPO_MOSS_THICK,bumpMap:TOPO_MOSS_THICK,"
        "bumpScale:2.4,color:0xffffff,roughness:1,flatShading:true}),"
        "topoGravelM=new THREE.MeshStandardMaterial({map:TOPO_GRAVEL_GRAY,bumpMap:TOPO_GRAVEL_GRAY,"
        "bumpScale:2.2,color:0xffffff,roughness:.95,flatShading:true}),"
        "waterM=new THREE.MeshStandardMaterial({color:0x3a6a72,roughness:.12,metalness:.08,transparent:true,opacity:.78,depthWrite:false});"
    )
    s = HTML.read_text(encoding="utf-8")
    if "TOPO_ROCK_LIMESTONE" in s:
        print("HTML already has TOPO textures")
        return
    pat = re.compile(
        r"const ROCK_TEX=\(function\(\)\{.*?\}\)\(\);\s*"
        r"rockM=new THREE\.MeshStandardMaterial\(\{[^}]+\}\),"
        r"dirtM=new THREE\.MeshStandardMaterial\(\{[^}]+\}\),"
        r"waterM=new THREE\.MeshStandardMaterial\(\{[^}]+\}\);",
        re.S,
    )
    if not pat.search(s):
        pat = re.compile(
            r"rockM=new THREE\.MeshStandardMaterial\(\{[^}]+\}\),"
            r"dirtM=new THREE\.MeshStandardMaterial\(\{[^}]+\}\),"
            r"waterM=new THREE\.MeshStandardMaterial\(\{[^}]+\}\);",
            re.S,
        )
        if not pat.search(s):
            raise SystemExit("rockM block not found")
    s = pat.sub(rock_js, s, count=1)
    s = s.replace(
        "if(type===17)addBox(g,w,.36,d,cx,-.30,cz,moss);",
        "if(type===17)addBox(g,w,.36,d,cx,-.30,cz,typeof topoMossM!=='undefined'?topoMossM:moss);",
        1,
    )
    s = s.replace(
        "rng.next()<.55?rockM:dirtM",
        "rng.next()<.4?rockM:rng.next()<.5?topoGravelM:dirtM",
        1,
    )
    tmp = HTML.with_name("Creating Constructs.html.topo_tmp")
    tmp.write_text(s, encoding="utf-8")
    tmp.replace(HTML)
    print(f"patched HTML ({HTML.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
