"""Inject moss surface-nets module into Creating Constructs.html and wire refresh()."""
from __future__ import annotations
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HTML = ROOT / "Creating Constructs.html"
MOD = ROOT / "tools" / "moss_nets_module.js"


def main() -> None:
    s = HTML.read_text(encoding="utf-8")
    mod = MOD.read_text(encoding="utf-8")
    if "/*MOSS_NETS*/" in s:
        # replace existing block
        start = s.index("/*MOSS_NETS*/")
        end = s.index("/*MOSS_NETS_END*/") + len("/*MOSS_NETS_END*/")
        s = s[:start] + mod + s[end:]
        print("ok replaced moss nets block")
    else:
        needle = "// refresh instances"
        if needle not in s:
            raise SystemExit("refresh marker missing")
        s = s.replace(needle, mod + "\n" + needle, 1)
        print("ok inserted moss nets block")

    old = "if(v.t!==15&&v.t!==14&&v.t!==41&&v.t!==42&&!(v.t>=43&&v.t<=46)){meshes[v.t].forEach(m=>{m.setMatrixAt(n[v.t],D.matrix);m.setColorAt(n[v.t],cc)});n[v.t]++;}"
    new = "if(v.t!==5&&v.t!==15&&v.t!==14&&v.t!==41&&v.t!==42&&!(v.t>=43&&v.t<=46)){meshes[v.t].forEach(m=>{m.setMatrixAt(n[v.t],D.matrix);m.setColorAt(n[v.t],cc)});n[v.t]++;}"
    if old not in s:
        if "v.t!==5&&v.t!==15" in s:
            print("ok refresh already skips moss cubes")
        else:
            raise SystemExit("refresh instance line missing")
    else:
        s = s.replace(old, new, 1)
        print("ok skip moss instances")

    old2 = "meshes.forEach((ps,t)=>ps.forEach(m=>{m.count=n[t];m.instanceMatrix.needsUpdate=true;m.instanceColor.needsUpdate=true}));lampPos=lamps;lightsSync();buildAdaptiveTrees();buildAdaptiveIvy();"
    new2 = "meshes.forEach((ps,t)=>ps.forEach(m=>{m.count=n[t];m.instanceMatrix.needsUpdate=true;m.instanceColor.needsUpdate=true}));lampPos=lamps;lightsSync();buildMossSurfaceNets();buildAdaptiveTrees();buildAdaptiveIvy();"
    if "buildMossSurfaceNets()" in s and old2 not in s:
        print("ok buildMoss already wired")
    elif old2 not in s:
        raise SystemExit("refresh footer missing")
    else:
        s = s.replace(old2, new2, 1)
        print("ok wired buildMossSurfaceNets")

    # Dock thumb still uses cube; mark moss material flat-friendly for nets
    if "moss.flatShading" not in s and "flatShading:true" not in s.split("moss=")[1][:120]:
        s = s.replace(
            "moss=new THREE.MeshStandardMaterial({map:MT,bumpMap:LT,bumpScale:1.5,roughness:1})",
            "moss=new THREE.MeshStandardMaterial({map:MT,bumpMap:LT,bumpScale:1.5,roughness:1,flatShading:true})",
            1,
        )
        print("ok moss flatShading")

    HTML.write_text(s, encoding="utf-8")
    print("wrote", HTML)


if __name__ == "__main__":
    main()
