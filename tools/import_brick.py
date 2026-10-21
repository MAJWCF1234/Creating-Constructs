"""Import lime-mortar red clay brick textures into native + HTML.

Sources are 32×32 palette PNGs (and an arch folder). Upsamples with Lanczos
to 128×128 (smooth — NEAREST made bump maps read as voxels). Writes:
  assets/brick/*.png
  src/brick_textures.inc
  patches Creating Constructs.html brick map (BT) + brickM top/side
"""
from __future__ import annotations

import json
import re
from pathlib import Path

from PIL import Image, ImageFilter, ImageOps

ROOT = Path(__file__).resolve().parents[1]
CLAY = Path(r"O:\Assets\Textures\1_basic_refined\1_stone\1_clay")
TUDOR = Path(r"O:\Assets\Textures\2_advanced_refined\4_tudor")
OUT_INC = ROOT / "src" / "brick_textures.inc"
OUT_DIR = ROOT / "assets" / "brick"
HTML = ROOT / "Creating Constructs.html"

# (logical_name, absolute_or_relative path under CLAY/TUDOR)
PICKS: list[tuple[str, Path]] = [
    ("side", CLAY / "lime_mortar_red_clay_brick_xt646.png"),
    ("top", CLAY / "lime_mortar_red_clay_brick_topbottom_7o0c4.png"),
    ("mossy_side", CLAY / "mossy_lime_mortar_red_clay_brick_cf9t2.png"),
    ("mossy_top", CLAY / "mossy_lime_mortar_red_clay_brick_topbottom_odt0j.png"),
    ("stairs", CLAY / "lime_mortar_red_clay_brick_stairs_vbjk3.png"),
    ("stairs_top", CLAY / "lime_mortar_red_clay_brick_stairs_top_wep2z.png"),
    ("arch_1", CLAY / "lime_mortar_red_clay_brick_arch" / "1_rxa86.png"),
    ("arch_2", CLAY / "lime_mortar_red_clay_brick_arch" / "2_1evza.png"),
    ("arch_3", CLAY / "lime_mortar_red_clay_brick_arch" / "3_5fusg.png"),
    ("arch_keystone", CLAY / "lime_mortar_red_clay_brick_arch" / "keystones_gb5vk.png"),
    ("arch_mortar", CLAY / "lime_mortar_red_clay_brick_arch" / "mortar_jmk9h.png"),
    ("tudor_narrow_1", TUDOR / "lime_mortar_red_clay_brick_with_tudor_narrow_diagonal_frames_1_f47z7.png"),
    ("tudor_narrow_2", TUDOR / "lime_mortar_red_clay_brick_with_tudor_narrow_diagonal_frames_2_ywpk0.png"),
    ("tudor_diag_up", TUDOR / "lime_mortar_red_clay_brick_with_tudor_diagonal_upwards_frame_j2lmo.png"),
    ("tudor_diag_1", TUDOR / "lime_mortar_red_clay_brick_with_tudor_diagonal_frame_1_g91tn.png"),
    ("tudor_diag_2", TUDOR / "lime_mortar_red_clay_brick_with_tudor_diagonal_frame_2_yz3px.png"),
    ("tudor_diag_down", TUDOR / "lime_mortar_red_clay_brick_with_tudor_diagonal_downwards_frame_us00f.png"),
    ("tudor_cross", TUDOR / "lime_mortar_red_clay_brick_with_tudor_cross_frame_tui94.png"),
    ("tudor_box", TUDOR / "lime_mortar_red_clay_brick_with_tudor_box_frame_jvzvr.png"),
]

CONSTS = {
    "side": "BrickSide",
    "top": "BrickTop",
    "mossy_side": "BrickMossySide",
    "mossy_top": "BrickMossyTop",
    "stairs": "BrickStairs",
    "stairs_top": "BrickStairsTop",
    "arch_1": "BrickArch1",
    "arch_2": "BrickArch2",
    "arch_3": "BrickArch3",
    "arch_keystone": "BrickArchKeystone",
    "arch_mortar": "BrickArchMortar",
    "tudor_narrow_1": "BrickTudorNarrow1",
    "tudor_narrow_2": "BrickTudorNarrow2",
    "tudor_diag_up": "BrickTudorDiagUp",
    "tudor_diag_1": "BrickTudorDiag1",
    "tudor_diag_2": "BrickTudorDiag2",
    "tudor_diag_down": "BrickTudorDiagDown",
    "tudor_cross": "BrickTudorCross",
    "tudor_box": "BrickTudorBox",
}


def load128(path: Path) -> Image.Image:
    if not path.exists():
        raise SystemExit(f"missing texture: {path}")
    # Lanczos — same as topography. NEAREST keeps 4×4 flat plateaus that bump into voxels.
    return Image.open(path).convert("RGBA").resize((128, 128), Image.Resampling.LANCZOS)


def with_height_alpha(im: Image.Image) -> Image.Image:
    """Pack a smooth relief height into alpha: bright lime mortar → recessed (low)."""
    rgb = im.convert("RGB")
    # Invert luma so mortar (light) is low; blur so close-up dFdx stays stable.
    height = ImageOps.invert(im.convert("L")).filter(ImageFilter.GaussianBlur(radius=1.4))
    height = height.point(lambda v: 48 + (v * 207) // 255)
    out = rgb.copy()
    out.putalpha(height)
    return out.convert("RGBA")


def c_array(name: str, im: Image.Image) -> str:
    data = ",".join(str(b) for b in im.tobytes())
    return f"const unsigned char brick_{name}_rgba[128*128*4]={{{data}}};"


def js_texture(name: str, im: Image.Image, *, rgba: bool = True) -> str:
    if rgba:
        data = im.tobytes()
    else:
        # height-only RGB for Three bumpMap (luminance)
        h = im.split()[3]
        data = Image.merge("RGBA", (h, h, h, Image.new("L", im.size, 255))).tobytes()
    arr = ",".join(str(b) for b in data)
    key = name.upper()
    return (
        f"const BRICK_{key}=(function(){{"
        f"const c=document.createElement('canvas');c.width=c.height=128;"
        f"const g=c.getContext('2d'),sid=g.createImageData(128,128);sid.data.set([{arr}]);g.putImageData(sid,0,0);"
        f"const t=new THREE.CanvasTexture(c);t.magFilter=THREE.LinearFilter;t.minFilter=THREE.LinearMipmapLinearFilter;"
        f"t.generateMipmaps=true;t.wrapS=t.wrapT=THREE.RepeatWrapping;t.repeat.set(2,2);t.needsUpdate=true;return t}})();"
    )


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    images: list[tuple[str, Image.Image, str]] = []
    for name, path in PICKS:
        im = with_height_alpha(load128(path))
        images.append((name, im, str(path)))
        im.save(OUT_DIR / f"{name}.png")
        print(f"  {name} <- {path.name}")

    lines = [
        "// Auto-generated by tools/import_brick.py — 32→128 NEAREST lime-mortar red clay brick pack.",
        f"constexpr int BrickTextureCount={len(images)};",
    ]
    for name, im, _ in images:
        lines.append(c_array(name, im))
    lines.append(
        "const unsigned char* brickPixels[BrickTextureCount]={"
        + ",".join(f"brick_{name}_rgba" for name, _, _ in images)
        + "};"
    )
    idx = {name: i for i, (name, _, _) in enumerate(images)}
    for key, const in CONSTS.items():
        lines.append(f"constexpr int {const}={idx[key]};")
    lines.append(
        "// Runtime atlas: layer 3 = side (legacy brick slot); "
        "extra layers start at 9+NatureTextureCount+TopoTextureCount (BrickAtlasBase)."
    )
    OUT_INC.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"wrote {OUT_INC} ({OUT_INC.stat().st_size} bytes)")

    # --- HTML: embed side+top (and mossy) and retarget BT / brickM ---
    s = HTML.read_text(encoding="utf-8")
    side_im = images[idx["side"]][1]
    top_im = images[idx["top"]][1]
    brick_block = (
        js_texture("side", side_im)
        + js_texture("top", top_im)
        + js_texture("height", side_im, rgba=False)
        + js_texture("height_top", top_im, rgba=False)
    )

    # Replace procedural bakeBrick body usage: BT=ktgTex(bakeBrick) → BT=BRICK_SIDE
    if "BT=ktgTex(bakeBrick)" not in s and "BT=BRICK_SIDE" not in s:
        raise SystemExit("HTML BT=ktgTex(bakeBrick) not found")
    s = s.replace("BT=ktgTex(bakeBrick)", "BT=BRICK_SIDE", 1)

    # Insert BRICK_* consts just before const tex=ktgTex(bakeConcrete)
    marker = "const tex=ktgTex(bakeConcrete)"
    if marker not in s:
        raise SystemExit("HTML ktgTex block marker not found")
    if "const BRICK_SIDE=" in s:
        s = re.sub(
            r"const BRICK_SIDE=\(function\(\)\{.*?\}\)\(\);(?:const BRICK_[A-Z0-9_]+=\(function\(\)\{.*?\}\)\(\);)*",
            brick_block,
            s,
            count=1,
            flags=re.S,
        )
    else:
        s = s.replace(marker, brick_block + marker, 1)

    # brickM: use side map + onBeforeCompile for top faces
    brick_mat = (
        "brickM=(function(){"
        "const m=new THREE.MeshStandardMaterial({map:BRICK_SIDE,bumpMap:BRICK_HEIGHT,bumpScale:.45,roughness:.88});"
        "const prev=m.onBeforeCompile;m.onBeforeCompile=sh=>{"
        "if(prev)prev(sh);"
        "sh.uniforms.brickTop={value:BRICK_TOP};sh.uniforms.brickHeightTop={value:BRICK_HEIGHT_TOP};"
        "sh.fragmentShader='uniform sampler2D brickTop,brickHeightTop;\\n'+sh.fragmentShader;"
        "sh.fragmentShader=sh.fragmentShader.replace("
        "'#include <map_fragment>',"
        "'#include <map_fragment>\\n"
        "#ifdef USE_MAP\\n"
        "if(abs(normalize(vNormal).y)>.7){"
        "vec4 bt=texture2D(brickTop,vUv);float ht=texture2D(brickHeightTop,vUv).r;"
        "diffuseColor.rgb=bt.rgb*mix(.76,1.,ht);}\\n"
        "#endif\\n'"
        ");"
        "};m.customProgramCacheKey=()=>'brickHeightRelief';return m;})(),"
    )
    if re.search(r"brickM=new THREE\.MeshStandardMaterial\(\{[^}]+\}\),", s):
        s = re.sub(
            r"brickM=new THREE\.MeshStandardMaterial\(\{[^}]+\}\),",
            brick_mat,
            s,
            count=1,
        )
    elif "brickM=(function(){" in s:
        s = re.sub(
            r"brickM=\(function\(\)\{.*?\}\)\(\),",
            brick_mat,
            s,
            count=1,
            flags=re.S,
        )
    else:
        raise SystemExit("HTML brickM assignment not found")

    HTML.write_text(s, encoding="utf-8")

    (OUT_DIR / "manifest.json").write_text(
        json.dumps([{"name": n, "source": src} for n, _, src in images], indent=2),
        encoding="utf-8",
    )
    print("patched HTML brick textures")
    print("done")


if __name__ == "__main__":
    main()
