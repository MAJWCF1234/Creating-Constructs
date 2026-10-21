"""Sync src/catalog.inc from Creating Constructs.html TN + CATS arrays."""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HTML = ROOT / "Creating Constructs.html"
OUT = ROOT / "src" / "catalog.inc"


def main() -> None:
    s = HTML.read_text(encoding="utf-8", errors="ignore")
    tn = re.search(r"TN=\[(.*?)\];", s, re.S)
    cats = re.search(r"CATS=\[(.*?)\];", s, re.S)
    if not tn or not cats:
        raise SystemExit("TN or CATS not found in HTML")
    names = re.findall(r"'([^']*)'", tn.group(1))
    # CATS entries: ['Name',[ids...]]
    cat_blocks = re.findall(r"\['([^']+)',\[([^\]]*)\]\]", cats.group(1))
    if not names or not cat_blocks:
        raise SystemExit("failed to parse catalog")
    piece_count = len(names)
    category_count = len(cat_blocks)
    parsed = []
    max_slots = 12
    for _, ids in cat_blocks:
        nums = [int(x) for x in ids.split(",") if x.strip()]
        max_slots = max(max_slots, len(nums))
        parsed.append(nums)
    lines = [
        "// Generated from Creating Constructs.html by tools/sync_catalog.py.",
        f"constexpr int PieceCount={piece_count},CategoryCount={category_count},PieceEncodingStride=128;",
        "const char* names[]={" + ",".join(f'"{n}"' for n in names) + "};",
        "const char* cats[]={" + ",".join(f'"{c[0]}"' for c in cat_blocks) + "};",
        f"constexpr int CategorySlots={max_slots};",
        "const int catTypes[CategoryCount][CategorySlots]={",
    ]
    for nums in parsed:
        padded = nums + [-1] * (max_slots - len(nums))
        lines.append("{" + ",".join(str(n) for n in padded) + "},")
    lines.append("};")
    lines.append('static_assert(PieceCount<=PieceEncodingStride,"GPU piece encoding must fit all types");')
    OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"wrote {OUT} PieceCount={piece_count} categories={category_count}")


if __name__ == "__main__":
    main()
