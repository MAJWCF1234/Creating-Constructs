"""Pack C++ nature meshes + textures into Creating Constructs.html (single-file offline).

Regenerate after changing src/nature_models.inc or nature_textures.inc.
"""
from __future__ import annotations
import re, struct, base64
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MODELS = (ROOT / "src" / "nature_models.inc").read_text(encoding="utf-8", errors="ignore")
TEX = (ROOT / "src" / "nature_textures.inc").read_text(encoding="utf-8", errors="ignore")
HTML = ROOT / "Creating Constructs.html"
THREE = ROOT / "three.min.js"

MESH_NAMES = [
    "nature_tree_5", "nature_tree_6", "nature_tree_7", "nature_tree_8", "nature_tree_9",
    "nature_birch_1", "nature_birch_2", "nature_pine_1", "nature_pine_2",
    "nature_bracken", "nature_bracken_2", "nature_bracken_3",
    "nature_meadow", "nature_meadow_2", "nature_tallgrass", "nature_tallgrass_2",
    "nature_dogbane", "nature_dogbane_2", "nature_clover", "nature_aster", "nature_goldenrod",
]
MARKER_START = "/*NATURE_TREES_PACK*/"
MARKER_END = "/*NATURE_TREES_PACK_END*/"


def parse_rgba_names() -> list[str]:
    return re.findall(r"const unsigned char (nature_\w+_rgba)\[\]", TEX)


def parse_bark_layers() -> list[int]:
    m = re.search(r"constexpr bool natureBarkLayer\[NatureTextureCount\]=\{([^}]+)\}", TEX)
    if not m:
        return [1]
    return [i for i, tok in enumerate(m.group(1).split(",")) if tok.strip() == "true"]


def parse_rgba(name: str) -> bytes:
    m = re.search(rf"const unsigned char {name}\[\] = \{{([^}}]+)\}}", TEX)
    if not m:
        raise SystemExit(f"texture {name} not found")
    nums = [int(x) for x in m.group(1).split(",") if x.strip()]
    if len(nums) != 128 * 128 * 4:
        raise SystemExit(f"{name} size {len(nums)}")
    return bytes(nums)


def parse_mesh(name: str) -> list[list[float]]:
    start = MODELS.find(f"const NatureVertex {name}[]")
    if start < 0:
        raise SystemExit(f"missing {name}")
    end = MODELS.find("const NatureVertex ", start + 10)
    body = MODELS[start : end if end > 0 else len(MODELS)]
    verts = re.findall(
        r"\{\{([-\d.eE+]+f?),([-\d.eE+]+f?),([-\d.eE+]+f?)\},"
        r"\{([-\d.eE+]+f?),([-\d.eE+]+f?),([-\d.eE+]+f?)\},"
        r"([-\d.eE+]+f?),([-\d.eE+]+f?),(\d+)\}",
        body,
    )
    return [[float(x.rstrip("f")) for x in v[:8]] + [int(v[8])] for v in verts]


def pack_mesh(verts: list[list[float]]) -> str:
    buf = bytearray()
    mats = bytearray()
    for px, py, pz, nx, ny, nz, u, vv, mat in verts:
        buf += struct.pack("<8f", px, py, pz, nx, ny, nz, u, vv)
        mats.append(mat & 255)
    raw = struct.pack("<I", len(verts)) + bytes(buf) + bytes(mats)
    return base64.b64encode(raw).decode("ascii")


def js_blob(meshes: dict[str, str], textures: list[str], bark_layers: list[int]) -> str:
    lines = [MARKER_START, "// Embedded from src/nature_models.inc + nature_textures.inc"]
    lines.append("const NATURE_MESH_PACK={")
    for i, name in enumerate(MESH_NAMES):
        comma = "," if i + 1 < len(MESH_NAMES) else ""
        lines.append(f" {name}:{meshes[name]!r}{comma}")
    lines.append("};")
    lines.append("const NATURE_TREE_PACK=NATURE_MESH_PACK;")
    lines.append("const NATURE_TEX_PACK=[")
    for i, t in enumerate(textures):
        comma = "," if i + 1 < len(textures) else ""
        lines.append(f" {t!r}{comma}")
    lines.append("];")
    lines.append(f"const NATURE_BARK_LAYERS=new Set([{','.join(str(i) for i in bark_layers)}]);")
    lines.append("const NATURE_BRANCH_TEX=NATURE_TEX_PACK[0],NATURE_BARK_TEX=NATURE_TEX_PACK[1];")
    lines.append(MARKER_END)
    return "\n".join(lines)


def inject_pack(html: str, blob: str) -> str:
    if MARKER_START in html and MARKER_END in html:
        a = html.index(MARKER_START)
        b = html.index(MARKER_END) + len(MARKER_END)
        return html[:a] + blob + html[b:]
    needle = "// openktg-style texture recipes"
    if needle not in html:
        raise SystemExit("inject point missing")
    return html.replace(needle, blob + "\n" + needle, 1)


def inline_three(html: str) -> str:
    tag = '<script src="three.min.js"></script>'
    if tag not in html:
        print("three already inlined or absent; leaving scripts untouched")
        return html
    if not THREE.exists():
        raise SystemExit("three.min.js missing — download r128 first")
    js = THREE.read_text(encoding="utf-8", errors="ignore")
    html = html.replace(tag, "<script>\n" + js + "\n</script>", 1)
    print(f"inlined three.min.js ({len(js)} chars)")
    return html


def main() -> None:
    tex_names = parse_rgba_names()
    bark_layers = parse_bark_layers()
    meshes = {}
    for name in MESH_NAMES:
        verts = parse_mesh(name)
        meshes[name] = pack_mesh(verts)
        print(f"{name}: {len(verts)} verts, {len(meshes[name])} b64")
    textures = [base64.b64encode(parse_rgba(n)).decode("ascii") for n in tex_names]
    for n, t in zip(tex_names, textures):
        print(f"{n}: {len(t)} b64")
    print("bark layers", bark_layers)
    blob = js_blob(meshes, textures, bark_layers)
    html = HTML.read_text(encoding="utf-8")
    html = inject_pack(html, blob)
    html = inline_three(html)
    HTML.write_text(html, encoding="utf-8")
    print(f"updated {HTML} ({HTML.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
