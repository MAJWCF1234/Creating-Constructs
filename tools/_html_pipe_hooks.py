from pathlib import Path
s = Path(r"N:\Monolith maker\Creating Constructs.html").read_text(encoding="utf-8", errors="ignore")
needles = [
    "function pick(",
    "function cabClick",
    "function exportWorld",
    "function loadWorld",
    "function refresh(",
    "cells.forEach",
    "\"cab\"",
    "cab:",
    "function put(",
    "const cells=",
    "drawing=",
    "stroke=",
]
for n in needles:
    i = s.find(n)
    print(f"\n=== {n} @ {i} ===")
    if i >= 0:
        print(s[i : i + 450])
