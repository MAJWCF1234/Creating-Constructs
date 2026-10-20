from pathlib import Path
s = Path(r"N:\Monolith maker\Creating Constructs.html").read_text(encoding="utf-8", errors="ignore")
i = s.find("addEventListener('mousemove'")
chunk = s[i : i + 1200]
print(chunk)
print("==== drawing refs")
idx = 0
while True:
    j = chunk.find("drawing", idx)
    if j < 0:
        break
    print(j, chunk[j : j + 80])
    idx = j + 1
# also search whole file for drawing&& 
import re
for m in re.finditer(r"drawing.{0,40}act\(", s):
    if m.start() > 4200000:
        print("HIT", m.start(), m.group(0))
