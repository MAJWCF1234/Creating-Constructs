"""Run native catalog previews/contracts and build a browsable lossless preview gallery.
Requires Pillow. Pass --summarize-only for an already completed native audit.
"""
from pathlib import Path
import argparse, csv, json, subprocess, html, shutil, re
from PIL import Image, ImageDraw
R=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser()
a.add_argument('--exe',type=Path,default=R/'Play/CreatingConstructs.exe')
a.add_argument('--output',type=Path,default=R/'output/catalog-audit')
a.add_argument('--summarize-only',action='store_true')
args=a.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=True)
if not args.summarize_only:
 subprocess.run([str(args.exe),'--catalog-audit','--catalog-contracts','--validate','--audit-dir='+str(out)],check=True)
 log=Path.home()/'AppData/Local/CreatingConstructs/runtime.log'
 shutil.copy2(log,out/'runtime.log')
cases=[json.loads(x) for x in (out/'cases.jsonl').read_text().splitlines()]
piece_count=len(re.findall(r'"([^"]+)"',re.search(r'names\[\]=\{(.*?)\};',(R/'src/catalog.inc').read_text()).group(1)))
assert len(cases)==piece_count*4+197,'Incomplete visual audit'
assert all(x['nonfinite']==0 and x['degenerate']==0 for x in cases),'Invalid mesh geometry'
previews=out/'previews';previews.mkdir(exist_ok=True)
for item in cases:
 stem=f"{item['case']:04d}-t{item['type']:02d}-r{item['rotation']}-v{item['variant']}"
 bmp=out/(stem+'.bmp');png=previews/(stem+'.png')
 if bmp.exists():
  with Image.open(bmp) as im:
   same=False
   if png.exists():
    with Image.open(png) as previous:same=previous.size==im.size and previous.convert('RGB').tobytes()==im.convert('RGB').tobytes()
   if not same:im.save(png,compress_level=1)
   with Image.open(png) as check:assert check.size==im.size and check.convert('RGB').tobytes()==im.convert('RGB').tobytes()
  bmp.unlink()
 assert png.exists(),png
 item['preview']='previews/'+png.name
# This directory contains only generated native previews; remove superseded case filenames.
expected={Path(c['preview']).name for c in cases}
for stale in previews.glob('*.png'):
 if stale.name not in expected and re.fullmatch(r'\d{4}-t\d{2}-r[0-3]-v[0-3]',stale.stem):stale.unlink()
sheets=out/'sheets';sheets.mkdir(exist_ok=True)
for first in range(0,piece_count*4,24):
 sheet=Image.new('RGB',(1120,1080),'#17171a');d=ImageDraw.Draw(sheet)
 for i,item in enumerate(cases[first:first+24]):
  im=Image.open(out/item['preview']).convert('RGB');im.thumbnail((280,155));x=(i%4)*280;y=(i//4)*180
  sheet.paste(im,(x,y));d.text((x+8,y+157),f"Part {item['type']}  rotation {item['rotation']}",fill='white')
 sheet.save(sheets/f"parts-{first//4:02d}-{first//4+5:02d}.jpg",quality=92)
contracts=list(csv.DictReader((out/'contracts.csv').open()))
interactions=list(csv.DictReader((out/'interactions.csv').open()))
assert len(contracts)==piece_count*4 and len(interactions)==piece_count*1920
assert all(int(x['nonfinite'])==0 and int(x['footprint_misses'])==0 for x in contracts)
assert all(int(x['nonfinite'])==0 for x in interactions)
summary={'visualPreviews':len(cases),'partRotations':len(contracts),'directedNeighborCases':len(interactions),'nonfinite':0,'degenerate':0,'footprintMisses':0,'meshProxyDifferences':sum(int(x['mesh_physics_mismatches']) for x in contracts)}
(out/'summary.json').write_text(json.dumps(summary,indent=2))
page=['<!doctype html><meta charset="utf-8"><title>Creating Constructs catalog review</title><style>body{background:#17171a;color:#eee;font:16px system-ui;margin:30px}main{display:grid;grid-template-columns:repeat(auto-fill,minmax(260px,1fr));gap:16px}img{width:100%}figure{margin:0}figcaption{padding:8px}</style><h1>Native Vulkan catalog review</h1>',f'<p>{len(cases)} previews; {len(interactions):,} directed neighbor geometry cases. Collision proxies deliberately allow player movement; mesh differences are review data, not automatic failures.</p><main>']
for c in cases:
 label=f"Part {c['type']} / rotation {c['rotation']} / variant {c['variant']} / selection {c['selection']} / face {c['face']}"
 page.append(f'<figure><a href="{c["preview"]}"><img loading="lazy" src="{c["preview"]}"></a><figcaption>{html.escape(label)}</figcaption></figure>')
page.append('</main>');(out/'index.html').write_text(''.join(page),encoding='utf-8')
print(json.dumps(summary,indent=2))
