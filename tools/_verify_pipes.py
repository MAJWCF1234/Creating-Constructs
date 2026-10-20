from pathlib import Path
s = Path(r"N:\Monolith maker\Creating Constructs.html").read_text(encoding="utf-8", errors="ignore")
checks = [
    "/*PIPE_SYSTEM*/",
    "'Pipe','Valve','Gauge','Outlet'",
    "['Pipes',[43,44,45,46]]",
    "isPipeTool(type)",
    "pipeLoad(d.pipes",
    "pipes:pipeSerialize",
    "pipeStroke",
    "pipeDrawLine",
    "pipeManual",
    "function wdata()",
]
for c in checks:
    print(("OK" if c in s else "MISS"), c)
# wdata body
i = s.find("function wdata()")
print(s[i : i + 220] if i >= 0 else "no wdata")
# cats length
import re
print("CATS", re.search(r"CATS=\[(.*?)\];", s, re.S).group(0)[-80:])
