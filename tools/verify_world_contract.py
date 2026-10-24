"""One-shot sync/docs/footprint sanity check."""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
html = (ROOT / "Creating Constructs.html").read_text(encoding="utf-8", errors="replace")
cat = (ROOT / "src" / "generated/catalog.inc").read_text(encoding="utf-8")
doc = (ROOT / "docs" / "World-API.md").read_text(encoding="utf-8")

tn = re.findall(r"'([^']+)'", re.search(r"TN=\[(.*?)\];", html, re.S).group(1))
names = re.findall(r'"([^"]+)"', re.search(r"names\[\]=\{(.*?)\};", cat, re.S).group(1))
assert len(tn) == 124 and tn == names, f"catalog mismatch TN={len(tn)} names={len(names)}"

fw = re.search(r"function footprintW\(t\)\{[^}]+\}", html).group(0)
fd = re.search(r"function footprintD\(t\)\{[^}]+\}", html).group(0)
fh = re.search(r"function footprintH\(t\)\{[^}]+\}", html).group(0)
assert "94" not in fw + fd + fh, "Quadpier must be 1-cell in HTML"
assert "78" in fw and "58" in fd

rot = re.search(r"function eachFootprintCell\(.*?\{.*?for\(let i=0;i<r;i\+\+\)\{([^}]+)\}", html, re.S)
assert rot and "nx=dz" in rot.group(1) and "dz=-dx" in rot.group(1), "HTML footprint rotation must match native"

for needle in (
    "PieceCount = 124",
    "MON4",
    "MNS4",
    "MaxPlot = 64",
    "0…123",
    "stride:64",
    "Quadpier",
    "81920",
):
    assert needle in doc, f"missing doc marker: {needle}"

assert "currently **0â€¦71**" not in doc
assert "PieceCount = 68" not in doc
assert "emits `N` / `c` / `cab` only" not in doc
print("OK: catalog 124 synced, footprints match native, World-API markers present")
