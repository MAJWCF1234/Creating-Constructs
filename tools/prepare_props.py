"""Import Retro Official props and tileable textures for the Cyber / Liminal parts.

Writes src/generated/prop_models.inc (NatureVertex arrays, material = PROP_BASE + layer),
src/generated/prop_textures.inc (128x128 RGBA layers, thumbnail colours, tiling flags),
src/generated/prop_shader.glsl (layer constants and per-material tiling) and
assets/props/ (PNG copies + manifest.json with source hashes).

Models are normalised into cell space: the footprint spans x,z in [-.5, W-.5] / [-.5, D-.5]
with the base at y = 0, so native code places them with natureModel(mesh, {0,-.5,0}).
Run `py -3 tools/embed_props.py` afterwards to refresh the web build.
"""
from __future__ import annotations
from pathlib import Path
import hashlib, io, json, math, struct
import numpy as np
from PIL import Image

R = Path(__file__).resolve().parents[1]
OUT = R / "assets" / "props"
OUT.mkdir(parents=True, exist_ok=True)
RETRO = Path(r"O:\retro official")
TEX = Path(r"O:\Assets\Textures")
PROP_BASE = 59  # first prop material id == first prop texture layer (follows the brick pack)

textures: list[tuple[str, Image.Image, bool, float]] = []  # name, image, tileable, tiling scale
sources: list[str] = []
models: list[tuple[str, np.ndarray]] = []


def texture(im: Image.Image, name: str, source: Path, tileable: bool, tiling: float = 1.0) -> int:
    im = im.convert("RGBA")
    rgb = im.convert("RGB").resize((128, 128), Image.Resampling.LANCZOS)
    a = im.getchannel("A").resize((128, 128), Image.Resampling.NEAREST)
    a = Image.fromarray(np.where(np.array(a) >= 128, 255, 0).astype(np.uint8))
    im = Image.merge("RGBA", (*rgb.split(), a))
    sources.append(str(source))
    raw = im.tobytes()
    for i, (_, existing, _, _) in enumerate(textures):
        if existing.tobytes() == raw:
            return i
    im.save(OUT / (name + ".png"))
    textures.append((name, im, tileable, tiling))
    return len(textures) - 1


def tile(name: str, source: Path, tiling: float = 1.0) -> int:
    return texture(Image.open(source), name, source, True, tiling)


def stacked(name: str, top: Path, bottom: Path) -> int:
    a, b = Image.open(top).convert("RGBA"), Image.open(bottom).convert("RGBA")
    w = max(a.width, b.width)
    a = a.resize((w, w), Image.Resampling.NEAREST); b = b.resize((w, w), Image.Resampling.NEAREST)
    im = Image.new("RGBA", (w, 2 * w)); im.paste(a, (0, 0)); im.paste(b, (0, w))
    sources.append(str(bottom))
    return texture(im, name, top, True, 1.0)


def _glb(path: Path):
    blob = path.read_bytes()
    size = struct.unpack_from("<I", blob, 12)[0]
    j = json.loads(blob[20:20 + size])
    binary = blob[20 + size + 8:]
    sources.append(str(path))
    return j, binary


def _accessor(j, binary, index):
    a = j["accessors"][index]
    v = j["bufferViews"][a["bufferView"]]
    dtype = {5126: "<f4", 5123: "<u2", 5125: "<u4", 5121: "<u1"}[a["componentType"]]
    n = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}[a["type"]]
    return np.frombuffer(binary, dtype=dtype, count=a["count"] * n,
                         offset=v.get("byteOffset", 0) + a.get("byteOffset", 0)).reshape(-1, n)


def _image(j, binary, texture_index):
    im = j["images"][j["textures"][texture_index]["source"]]
    v = j["bufferViews"][im["bufferView"]]
    return Image.open(io.BytesIO(binary[v["byteOffset"]:v["byteOffset"] + v["byteLength"]]))


def _quat_matrix(q):
    x, y, z, w = q
    return np.array([
        [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
        [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
        [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
    ])


def _node_world(j, node_index, parent=np.eye(4)):
    nd = j["nodes"][node_index]
    m = np.eye(4)
    if "matrix" in nd:
        m = np.array(nd["matrix"]).reshape(4, 4).T
    else:
        t = np.array(nd.get("translation", [0, 0, 0]))
        r = _quat_matrix(nd.get("rotation", [0, 0, 0, 1]))
        s = np.array(nd.get("scale", [1, 1, 1]))
        m[:3, :3] = r * s
        m[:3, 3] = t
    return parent @ m


def _mesh_nodes(j):
    out = []
    def walk(index, parent):
        world = _node_world(j, index, parent)
        nd = j["nodes"][index]
        if "mesh" in nd:
            out.append((nd["mesh"], world))
        for child in nd.get("children", []):
            walk(child, world)
    scene = j["scenes"][j.get("scene", 0)]
    for root in scene["nodes"]:
        walk(root, np.eye(4))
    return out


def rot_y(a):
    c, s = math.cos(a), math.sin(a)
    return np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]])


def rot_x(a):
    c, s = math.cos(a), math.sin(a)
    return np.array([[1, 0, 0], [0, c, -s], [0, s, c]])


def glb(rel: str, name: str, fit=(1, 1, 1), footprint=(1, 1, 1), yaw=0.0, pitch=0.0, offset=(0, 0, 0), layer_name=None):
    """Normalise a GLB into the footprint (W,H,D cells), base on y=0, centred in x/z."""
    path = RETRO / rel
    j, binary = _glb(path)
    verts = []
    for mesh_index, world in _mesh_nodes(j):
        for prim in j["meshes"][mesh_index]["primitives"]:
            material = j["materials"][prim.get("material", 0)]
            ti = material["pbrMetallicRoughness"]["baseColorTexture"]["index"]
            tname = layer_name or (material.get("name") or name).lower().replace(" ", "_")
            layer = texture(_image(j, binary, ti), f"{name}_{tname}" if layer_name is None else layer_name, path, False)
            a = prim["attributes"]
            pos = _accessor(j, binary, a["POSITION"]).astype(np.float64)
            nrm = _accessor(j, binary, a["NORMAL"]).astype(np.float64)
            uv = _accessor(j, binary, a["TEXCOORD_0"]).astype(np.float64)
            idx = _accessor(j, binary, prim["indices"]).ravel() if "indices" in prim else np.arange(len(pos))
            m3 = world[:3, :3]
            pos = pos @ m3.T + world[:3, 3]
            nrm = nrm @ np.linalg.inv(m3).T
            verts.append(np.concatenate([pos[idx], nrm[idx], uv[idx], np.full((len(idx), 1), PROP_BASE + layer)], axis=1))
    v = np.concatenate(verts)
    rot = rot_y(yaw) @ rot_x(pitch)
    v[:, :3] = v[:, :3] @ rot.T
    v[:, 3:6] = v[:, 3:6] @ rot.T
    lo, hi = v[:, :3].min(0), v[:, :3].max(0)
    size = np.maximum(hi - lo, 1e-6)
    scale = min(fit[0] / size[0], fit[1] / size[1], fit[2] / size[2])
    W, H, D = footprint
    centre = (lo + hi) * .5
    v[:, 0] = (v[:, 0] - centre[0]) * scale + (W - 1) * .5 + offset[0]
    v[:, 1] = (v[:, 1] - lo[1]) * scale + offset[1]
    v[:, 2] = (v[:, 2] - centre[2]) * scale + (D - 1) * .5 + offset[2]
    n = v[:, 3:6]
    v[:, 3:6] = n / np.maximum(np.linalg.norm(n, axis=1, keepdims=True), 1e-6)
    tri = v.reshape(-1, 3, 9)
    area = np.linalg.norm(np.cross(tri[:, 1, :3] - tri[:, 0, :3], tri[:, 2, :3] - tri[:, 0, :3]), axis=1)
    v = tri[area > 4e-6].reshape(-1, 9)  # the catalog audit rejects slivers below 1e-6
    models.append((name, v.astype(np.float32)))
    print(f"{name:12s} {len(v)//3:5d} tris  size {np.round((hi-lo)*scale,2)}  scale {scale:.3f}")


# ---- tileable surfaces (one texture per cell unless tiling says otherwise) ----
tile("carpet", RETRO / "Environments/Retro Office Pack/textures/Carpet_BaseColor.jpg", 1.0)
tile("plaster_yellow", TEX / "Plaster/Plaster_17-256x256.png", 1.0)
tile("plaster_cream", TEX / "Plaster/Plaster_15-256x256.png", 1.0)
tile("pool_tile", TEX / "Tile/Tile_15-256x256.png", 2.0)
tile("grating", TEX / "Grating/Grating_15-256x256_p3.png", 2.0)
tile("metal_plate", TEX / "Metal/Metal_18-256x256.png", 1.0)
tile("brushed_metal", TEX / "Metal/Metal_10-256x256.png", 1.0)
stacked("office_door", TEX / "2_advanced_refined/2_metal/blue_office_door_top_xa1r0.png", TEX / "2_advanced_refined/2_metal/blue_office_door_bottom_nh1qa.png")

# ---- props (W,H,D footprint; fit = usable extent inside it) ----
glb("Environments/OFFICE/computers/TV.glb", "tv", fit=(.9, .9, .6), offset=(0, 0, -.18))
glb("Environments/OFFICE/file_cabinets/file_cabinet_largest.glb", "cabinet", fit=(.96, .98, .9))
glb("Environments/URBAN/Litter/TrashCan/trash_can.glb", "trashcan", fit=(.8, .95, .8))
glb("Environments/URBAN/Cones & Barriers/Concrete Barriers/Jersey_barrier.glb", "barrier", fit=(1.96, 1, .9), footprint=(2, 1, 1))
glb("Environments/URBAN/Vents/duct.glb", "duct", fit=(1, .98, .98))
glb("Environments/URBAN/Vents/vent_cover.glb", "vent", fit=(.96, .96, .96), pitch=math.pi / 2, offset=(0, .02, -.42))
glb("Environments/URBAN/Traffic lights/street_light.glb", "streetlight", fit=(.9, 1.98, .9), footprint=(1, 2, 1))
glb("Environments/URBAN/Cones & Barriers/Bollards/concrete_retro_bollard_light.glb", "bollard", fit=(.5, .9, .5))
glb("Environments/OFFICE/extras/water_cooler_bottle_attached.glb", "cooler", fit=(.6, .98, .6))
glb("Environments/OFFICE/dividers/divider_black_blue_half.glb", "divider", fit=(1, .98, .5))
glb("Environments/OFFICE/desks/desk1.glb", "desk", fit=(1.96, .95, .98), footprint=(2, 1, 1))
glb("Environments/OFFICE/chairs/office_chair_black.glb", "chair", fit=(.9, .95, .9))
glb("Environments/OFFICE/file_cabinets/file_cabinet_larger.glb", "filecabinet", fit=(.96, .98, .9))

assert len(textures) <= 32, "shader material range 59..90 is the budget"

# ---- generated includes ----
def fmt(x):
    s = format(float(x), ".6g")
    return s + "f" if ("." in s or "e" in s) else s + ".0f"

lines = ["// Generated by tools/prepare_props.py from the local Retro Official packs; see assets/props/manifest.json."]
for name, a in models:
    lines.append(f"const NatureVertex prop_{name}[] = {{")
    for v in a:
        lines.append("{{" + ",".join(fmt(x) for x in v[:3]) + "},{" + ",".join(fmt(x) for x in v[3:6]) + "}," + fmt(v[6]) + "," + fmt(v[7]) + "," + str(int(v[8])) + "},")
    lines.append("};")
(R / "src/generated/prop_models.inc").write_text("\n".join(lines) + "\n", encoding="utf-8")

lines = ["// Generated by tools/prepare_props.py: prop albedo layers (material id = PropMaterialBase + index)."]
for name, im, _, _ in textures:
    lines.append(f"const unsigned char prop_{name}_rgba[] = {{" + ",".join(map(str, im.tobytes())) + "};")
lines.append(f"constexpr int PropMaterialBase={PROP_BASE},PropTextureCount={len(textures)};")
lines.append("const unsigned char* propPixels[PropTextureCount] = {" + ",".join(f"prop_{n}_rgba" for n, _, _, _ in textures) + "};")
colours = []
for _, im, _, _ in textures:
    arr = np.array(im.convert("RGB")).reshape(-1, 3).mean(0)
    colours.append("0x%02x%02x%02x" % tuple(int(c) for c in arr))
lines.append("const unsigned int propThumbColors[PropTextureCount] = {" + ",".join(colours) + "};")
lines.append("const char* propTextureNames[PropTextureCount] = {" + ",".join(f'"{n}"' for n, _, _, _ in textures) + "};")
(R / "src/generated/prop_textures.inc").write_text("\n".join(lines) + "\n", encoding="utf-8")

glsl = ["// Generated by tools/prepare_props.py.", f"#define PROP_BASE {PROP_BASE}", f"#define PROP_COUNT {len(textures)}",
        "// Tiling scale for planar-mapped prop surfaces; 0 means the mesh carries its own UVs.",
        "float propTiling(int material){switch(material-PROP_BASE){"]
for i, (_, _, tileable, tiling) in enumerate(textures):
    if tileable:
        glsl.append(f" case {i}:return {tiling:.2f};")
glsl.append(" default:return 0.;}}")
(R / "src/generated/prop_shader.glsl").write_text("\n".join(glsl) + "\n", encoding="utf-8")

manifest = {
    "textures": [{"name": n, "tileable": t, "tiling": s} for n, _, t, s in textures],
    "models": [{"name": n, "triangles": len(a) // 3} for n, a in models],
    "sources": [{"source": p, "sha256": hashlib.sha256(Path(p).read_bytes()).hexdigest()} for p in dict.fromkeys(sources)],
}
(OUT / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
print("textures:", [(i, n) for i, (n, _, _, _) in enumerate(textures)])
