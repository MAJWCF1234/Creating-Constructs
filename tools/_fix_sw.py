from pathlib import Path
p = Path(r"N:\Monolith maker\Creating Constructs.html")
s = p.read_text(encoding="utf-8")
old = "'#2b6a78','7a9a55','3d6b3a']"
new = "'#2b6a78','#7a9a55','#3d6b3a']"
if old not in s:
    raise SystemExit("pattern missing")
p.write_text(s.replace(old, new, 1), encoding="utf-8")
print("fixed")
