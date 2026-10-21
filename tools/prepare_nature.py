"""Import PSX nature meshes/textures into src/nature_*.inc."""
from pathlib import Path
import numpy as np, json, struct, io, hashlib
from PIL import Image

R = Path(__file__).resolve().parents[1]
OUT = R / "assets/nature"
OUT.mkdir(exist_ok=True)
PSX = Path(
    r"O:\retro official\PSX Nature v1.7.3b\PSX Nature v1.7.3b\PSX Nature (Tree branches separated)"
)
PLANTS = Path(r"O:\retro official\Nature\psx-plants\psx-plants_v1.0\models")
TREES = Path(r"O:\retro official\Nature\psx-trees\psx-trees_v1.0 - Copy\models")
textures = []  # (name, Image, is_bark)
sources = []
bark_layers = []


def _harden_alpha(im: Image.Image) -> Image.Image:
    """Keep cutouts binary so resize does not chew opaque bark/leaf islands."""
    im = im.convert("RGBA")
    arr = np.array(im)
    a = arr[:, :, 3]
    if a.min() >= 250:
        return im
    arr[:, :, 3] = np.where(a >= 128, 255, 0).astype(np.uint8)
    return Image.fromarray(arr)


def texture(im, name, source, is_bark=False):
    im = _harden_alpha(im)
    # RGB lanczos, alpha nearest — then re-harden
    rgb = im.convert("RGB").resize((128, 128), Image.Resampling.LANCZOS)
    a = im.getchannel("A").resize((128, 128), Image.Resampling.NEAREST)
    a = Image.fromarray(np.where(np.array(a) >= 128, 255, 0).astype(np.uint8))
    im = Image.merge("RGBA", (*rgb.split(), a))
    sources.append(str(source))
    raw = im.tobytes()
    for i, (_, existing, _) in enumerate(textures):
        if existing.tobytes() == raw and bark_layers[i] == is_bark:
            return 8 + i
    im.save(OUT / (name + ".png"))
    textures.append((name, im, is_bark))
    bark_layers.append(is_bark)
    return 8 + len(textures) - 1


leaf = texture(Image.open(PSX / "Textures/branch_1.png"), "branch", PSX / "Textures/branch_1.png", False)
bark = texture(Image.open(PSX / "Textures/tree_bark_6.png"), "bark", PSX / "Textures/tree_bark_6.png", True)
models = []


def obj(name, resolve=None):
    """resolve(usemtl) -> texture layer index (8+). Default: branch* → leaf else bark."""
    p = PSX / f"Models/other-formats/OBJ/{name}.obj"
    sources.append(str(p))
    pos, uv, norm, out = [], [], [], []
    if resolve is None:
        resolve = lambda m: leaf if m.lower().startswith("branch") or "branch" in m.lower() else bark
    mat = bark
    for line in p.read_text().splitlines():
        a = line.split()
        if not a:
            continue
        if a[0] == "v":
            pos.append(list(map(float, a[1:4])))
        elif a[0] == "vt":
            uv.append([float(a[1]), 1 - float(a[2])])
        elif a[0] == "vn":
            norm.append(list(map(float, a[1:4])))
        elif a[0] == "usemtl":
            mat = resolve(a[1])
        elif a[0] == "f":
            corners = [tuple(int(x) - 1 if x else -1 for x in v.split("/")) for v in a[1:]]
            for k in range(1, len(corners) - 1):
                for v in [corners[0], corners[k], corners[k + 1]]:
                    out.append([*pos[v[0]], *norm[v[2]], *uv[v[1]], 22 + (mat - 8)])
    models.append((name, np.array(out, dtype=np.float32)))


def _glb_json(path: Path):
    blob = path.read_bytes()
    size = struct.unpack_from("<I", blob, 12)[0]
    j = json.loads(blob[20 : 20 + size])
    binary = blob[20 + size + 8 :]
    sources.append(str(path))
    return j, binary


def _accessor(j, binary, index):
    a = j["accessors"][index]
    v = j["bufferViews"][a["bufferView"]]
    dtype = {5126: "<f4", 5123: "<u2", 5125: "<u4"}[a["componentType"]]
    n = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4}[a["type"]]
    return np.frombuffer(
        binary,
        dtype=dtype,
        count=a["count"] * n,
        offset=v.get("byteOffset", 0) + a.get("byteOffset", 0),
    ).reshape(-1, n)


def _image(j, binary, texture_index):
    ti = j["textures"][texture_index]["source"]
    im = j["images"][ti]
    v = j["bufferViews"][im["bufferView"]]
    return Image.open(io.BytesIO(binary[v["byteOffset"] : v["byteOffset"] + v["byteLength"]]))


def glb(filename, name):
    """Single-primitive plant GLB from psx-plants."""
    p = PLANTS / filename
    j, binary = _glb_json(p)
    primitive = j["meshes"][0]["primitives"][0]
    material = j["materials"][primitive.get("material", 0)]
    ti = material["pbrMetallicRoughness"]["baseColorTexture"]["index"]
    im = _image(j, binary, ti)
    layer = texture(im, name, p, False)
    a = primitive["attributes"]
    pos = _accessor(j, binary, a["POSITION"])
    normal = _accessor(j, binary, a["NORMAL"])
    uv = _accessor(j, binary, a["TEXCOORD_0"])
    idx = _accessor(j, binary, primitive["indices"]).ravel()
    out = np.concatenate(
        [pos[idx], normal[idx], uv[idx], np.full((len(idx), 1), 22 + layer - 8)], axis=1
    )
    models.append((name, out.astype(np.float32)))


def glb_aspen_birch(path: Path, name: str):
    """Aspen GLB → Birch: harden atlas, split tris into birch bark vs leaf by UV alpha."""
    j, binary = _glb_json(path)
    # base color texture
    mat0 = j["materials"][0]
    ti = mat0["pbrMetallicRoughness"]["baseColorTexture"]["index"]
    atlas = _harden_alpha(_image(j, binary, ti))
    leaf_layer = texture(atlas, f"{name}_leaf", path, False)
    bark_layer = texture(atlas, f"{name}_bark", path, True)
    alpha = np.array(_harden_alpha(atlas).resize((128, 128), Image.Resampling.NEAREST).getchannel("A"))

    def sample_a(u, v):
        x = int(np.clip(float(u), 0, 1 - 1e-6) * 127)
        y = int(np.clip(float(v), 0, 1 - 1e-6) * 127)
        return int(alpha[y, x])

    out = []
    for mesh in j.get("meshes", []):
        for primitive in mesh.get("primitives", []):
            a = primitive["attributes"]
            pos = _accessor(j, binary, a["POSITION"])
            normal = (
                _accessor(j, binary, a["NORMAL"])
                if "NORMAL" in a
                else np.tile([0.0, 1.0, 0.0], (len(pos), 1)).astype(np.float32)
            )
            uv = _accessor(j, binary, a["TEXCOORD_0"])
            idx = (
                _accessor(j, binary, primitive["indices"]).ravel().astype(np.int64)
                if "indices" in primitive
                else np.arange(len(pos), dtype=np.int64)
            )
            for t in range(0, len(idx), 3):
                ids = idx[t : t + 3]
                au = float(np.mean([sample_a(uv[i][0], uv[i][1]) for i in ids]))
                layer = bark_layer if au >= 200 else leaf_layer
                for i in ids:
                    out.append([*pos[i], *normal[i], *uv[i], 22 + (layer - 8)])
    models.append((name, np.array(out, dtype=np.float32)))
    print(f"  {name}: bark/leaf split from aspen atlas")


# Existing deciduous trees + understory
obj("tree_5")
obj("tree_6")
glb("bracken_01.glb", "bracken")
glb("grass_01_small.glb", "meadow")
glb("tall_grass_01_small.glb", "tallgrass")
for name in ["tree_7", "tree_8", "tree_9"]:
    obj(name)
for filename, name in [
    ("bracken_02.glb", "bracken_2"),
    ("bracken_03.glb", "bracken_3"),
    ("grass_01_medium.glb", "meadow_2"),
    ("tall_grass_01_medium.glb", "tallgrass_2"),
    ("dogbane_01.glb", "dogbane"),
    ("dogbane_02.glb", "dogbane_2"),
    ("white_clover_medium.glb", "clover"),
    ("savory-leaf_aster_01.glb", "aster"),
    ("old-field_goldenrod_01.glb", "goldenrod"),
]:
    glb(filename, name)

# Birch: fixed aspen stand-ins (real Birch/*.glb is 200k–500k verts)
glb_aspen_birch(TREES / "aspen_tree_01.glb", "birch_1")
glb_aspen_birch(TREES / "aspen_tree_02.glb", "birch_2")

# Pine: PSX Nature pines (same pack style as Tree), not the heavy Trees/Pine pack
def import_pine(obj_name, out_name, bark_file, branch_file):
    btex = texture(Image.open(PSX / "Textures" / bark_file), out_name + "_bark", PSX / "Textures" / bark_file, True)
    ltex = texture(Image.open(PSX / "Textures" / branch_file), out_name + "_branch", PSX / "Textures" / branch_file, False)

    def resolve(m):
        m = m.lower()
        if "branch" in m or "leaf" in m or "needle" in m:
            return ltex
        return btex

    obj(obj_name, resolve)
    models[-1] = (out_name, models[-1][1])


import_pine("pine_tree_n_1", "pine_1", "pine_bark_2.png", "pine_branch_3.png")
import_pine("pine_tree_n_2", "pine_2", "pine_bark_1.png", "pine_branch_1.png")

# Fallen timber + mushrooms (reuse bark atlas — no new texture layers / boardform shift)
obj("tree_log_1", lambda m: bark)
models[-1] = ("log", models[-1][1])
obj("tree_stump_1", lambda m: bark)
models[-1] = ("stump", models[-1][1])
obj("mushroom_n_1", lambda m: bark)
models[-1] = ("mushroom_1", models[-1][1])
obj("mushroom_n_2", lambda m: bark)
models[-1] = ("mushroom_2", models[-1][1])

# Three compact sumac bushes from the same licensed local plant pack.
for i in range(1,4):
    glb(f"rhus_typhina_small_0{i}.glb", f"shrub_{i}")
assert len(textures)==16, "Update shader texture-layer constants if the nature atlas count changes"

header = [
    "// Converted from the user local Retro Official packs; see assets/nature/manifest.json.",
    "struct NatureVertex {V p,n;float u,v;int material;};",
]
for name, a in models:
    triangles=a.reshape(-1,3,9)
    area=np.linalg.norm(np.cross(triangles[:,1,:3]-triangles[:,0,:3],triangles[:,2,:3]-triangles[:,0,:3]),axis=1)
    a=triangles[area>1e-6].reshape(-1,9)
    header.append(f"const NatureVertex nature_{name}[] = {{")
    for v in a:
        f = (
            lambda x: format(float(x), ".7g") + "f"
            if "." in format(float(x), ".7g") or "e" in format(float(x), ".7g")
            else str(int(x)) + ".0f"
        )
        header.append(
            "{{"
            + ",".join(f(x) for x in v[:3])
            + "},{"
            + ",".join(f(x) for x in v[3:6])
            + "},"
            + ",".join(f(x) for x in v[6:8])
            + ","
            + str(int(v[8]))
            + "},"
        )
    header.append("};")
    print(name, len(a) // 3, "triangles", np.round(a[:, :3].min(0), 2), np.round(a[:, :3].max(0), 2))

(R / "src/nature_models.inc").write_text("\n".join(header), encoding="utf-8")

lines = ["// Imported foliage albedo and alpha."]
for name, im, _ in textures:
    lines.append(
        "const unsigned char nature_"
        + name
        + "_rgba[] = {"
        + ",".join(map(str, im.tobytes()))
        + "};"
    )
lines.append(f"constexpr int NatureTextureCount={len(textures)};")
lines.append(
    "const unsigned char* naturePixels[NatureTextureCount] = {"
    + ",".join(f"nature_{name}_rgba" for name, _, _ in textures)
    + "};"
)
lines.append(
    "constexpr bool natureBarkLayer[NatureTextureCount]={"
    + ",".join("true" if b else "false" for b in bark_layers)
    + "};"
)
(R / "src/nature_textures.inc").write_text("\n".join(lines), encoding="utf-8")

manifest = [
    {"source": p, "sha256": hashlib.sha256(Path(p).read_bytes()).hexdigest()}
    for p in dict.fromkeys(sources)
]
(OUT / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
print("textures:", [(n, "bark" if b else "leaf") for n, _, b in textures])
bark_ids = [22 + i for i, b in enumerate(bark_layers) if b]
print("bark material ids:", bark_ids)
print("bark layers:", [i for i, b in enumerate(bark_layers) if b])
