from pathlib import Path
s = Path(r"N:\Monolith maker\Creating Constructs.html").read_text(encoding="utf-8", errors="ignore")
i = s.find("function pick()")
print(s[i : i + 900])
print("---")
i = s.find("cables=")
# find a few cables assignments
idx = 0
while True:
    j = s.find("cables=", idx)
    if j < 0 or j > 4300000:
        break
    if j > 4200000:
        print(s[j : j + 250])
        print("---")
    idx = j + 1
    if idx > 4300000:
        break
