"""Inject brutalism first-wave pieces, board-form material, mood, and starter into Creating Constructs.html."""
from __future__ import annotations
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"


def sub(s: str, old: str, new: str, label: str) -> str:
    if old not in s:
        raise SystemExit(f"MISSING: {label}\n{old[:120]!r}")
    print("ok", label)
    return s.replace(old, new, 1)


def main() -> None:
    s = HTML.read_text(encoding="utf-8")

    s = sub(
        s,
        "TN=['Block','Slab','Pillar','Glass','Slope','Moss','Stairs','Column','Window','Panel','Arch','Railing','Beam','Light','Tree','Ivy','Cable','Planter','Brick','Timber','Steel','Fins','Cobogó','Doorway','Pool','Ladder','Bollard','Sconce','Dome','Silo','Vault','Fence','Slit','Ribbon','Floor','Pergola','Fine stairs','Pendant','Obelisk','Bench','Water','Birch','Pine','Pipe','Valve','Gauge','Outlet']",
        "TN=['Block','Slab','Pillar','Glass','Slope','Moss','Stairs','Column','Window','Panel','Arch','Railing','Beam','Light','Tree','Ivy','Cable','Planter','Brick','Timber','Steel','Fins','Cobogó','Doorway','Pool','Ladder','Bollard','Sconce','Dome','Silo','Vault','Fence','Slit','Ribbon','Floor','Pergola','Fine stairs','Pendant','Obelisk','Bench','Water','Birch','Pine','Pipe','Valve','Gauge','Outlet','Boardform','Cantilever','Batter','Waffle','Portal','Clerestory','Plinth','Chase','Spout']",
        "TN",
    )
    s = sub(
        s,
        "'#8f98a2','#c52828','#c8963e','#6b7378']",
        "'#8f98a2','#c52828','#c8963e','#6b7378','#8a8680','#9a9690','#7a7670','#8e8a84','#85817b','#7e7a74','#6e6a64','#75716b','#6a6660']",
        "sw",
    )
    s = sub(
        s,
        '<label>Mood<select id="mood"><option>Overcast</option><option>Noon</option><option>Golden hour</option><option>Blue hour</option></select></label>',
        '<label>Mood<select id="mood"><option>Overcast</option><option>Noon</option><option>Golden hour</option><option>Blue hour</option><option>Concrete overcast</option></select></label>',
        "mood select",
    )

    s = sub(
        s,
        """function bakeSteel(img){ktgRng=17;for(let i=0;i<128*128;i++){img[i*4]=0x58;img[i*4+1]=0x5d;img[i*4+2]=0x60;img[i*4+3]=255}
for(let i=0;i<16;i++)for(let y=0;y<128;y++)for(let x=i*8;x<i*8+4&&x<128;x++){if(i%2)ktgBlend(img,x,y,255,255,255,.12);else ktgBlend(img,x,y,0,0,0,.2)}
for(let i=0;i<300;i++)ktgBlend(img,ktgRand()*128|0,ktgRand()*128|0,120,70,40,.15)}
""",
        """function bakeSteel(img){ktgRng=17;for(let i=0;i<128*128;i++){img[i*4]=0x58;img[i*4+1]=0x5d;img[i*4+2]=0x60;img[i*4+3]=255}
for(let i=0;i<16;i++)for(let y=0;y<128;y++)for(let x=i*8;x<i*8+4&&x<128;x++){if(i%2)ktgBlend(img,x,y,255,255,255,.12);else ktgBlend(img,x,y,0,0,0,.2)}
for(let i=0;i<300;i++)ktgBlend(img,ktgRand()*128|0,ktgRand()*128|0,120,70,40,.15)}
function bakeBoardForm(img){ktgRng=29;for(let i=0;i<128*128;i++){img[i*4]=0x8e;img[i*4+1]=0x8a;img[i*4+2]=0x82;img[i*4+3]=255}
for(let i=0;i<22;i++){const x0=ktgRand()*128,w=3+ktgRand()*5;for(let y=0;y<128;y++){const a=.18+.12*(y/127);for(let dx=0;dx<(w|0)+1;dx++)ktgBlend(img,x0+dx|0,y,55,52,48,a)}}
for(let i=0;i<18;i++){const y0=ktgRand()*128,h=1+ktgRand()*2;for(let x=0;x<128;x++){const a=.12;for(let dy=0;dy<(h|0)+1;dy++)ktgBlend(img,x,y0+dy|0,70,68,62,a)}}
for(let i=0;i<700;i++){const v=120+ktgRand()*50|0;ktgBlend(img,ktgRand()*128|0,ktgRand()*128|0,v,v-2,v-6,.07)}
const ties=[];for(let gy=0;gy<4;gy++)for(let gx=0;gx<4;gx++)ties.push([16+gx*32,16+gy*32]);
ties.forEach(([hx,hy])=>{for(let y=-5;y<=5;y++)for(let x=-5;x<=5;x++){const d=x*x+y*y;if(d<=20)ktgPut(img,hx+x,hy+y,22,22,20);if(d<=6)ktgPut(img,hx+x,hy+y,12,12,11)}
for(let a=0;a<12;a++){const t=a/12*Math.PI*2;ktgBlend(img,hx+Math.cos(t)*7|0,hy+Math.sin(t)*7|0,90,88,82,.55)}});
for(let i=0;i<80;i++){const x=ktgRand()*128|0,y=ktgRand()*128|0;ktgBlend(img,x,y,30,30,28,.35)}
}
""",
        "bakeBoardForm",
    )

    s = sub(
        s,
        "const tex=ktgTex(bakeConcrete),LT=ktgTex(img=>bakeLeaf(img,false)),MT=ktgTex(ktgTintMoss),IT=ktgTex(img=>bakeBiomeLeaf(img,true)),BT=ktgTex(bakeBrick),WT=ktgTex(bakeWood),ST=ktgTex(bakeSteel);",
        "const tex=ktgTex(bakeConcrete),LT=ktgTex(img=>bakeLeaf(img,false)),MT=ktgTex(ktgTintMoss),IT=ktgTex(img=>bakeBiomeLeaf(img,true)),BT=ktgTex(bakeBrick),WT=ktgTex(bakeWood),ST=ktgTex(bakeSteel),BF=ktgTex(bakeBoardForm);",
        "BF tex",
    )
    s = sub(
        s,
        "steelM=new THREE.MeshStandardMaterial({map:ST,bumpMap:ST,bumpScale:1.5,roughness:.45,metalness:.5}),waterM=new THREE.MeshStandardMaterial({color:0x2b4a52,roughness:.04,metalness:.4,transparent:true,opacity:.85});",
        "steelM=new THREE.MeshStandardMaterial({map:ST,bumpMap:ST,bumpScale:1.5,roughness:.45,metalness:.5}),boardM=new THREE.MeshStandardMaterial({map:BF,bumpMap:BF,bumpScale:1.35,roughness:.92}),waterM=new THREE.MeshStandardMaterial({color:0x2b4a52,roughness:.04,metalness:.4,transparent:true,opacity:.85});",
        "boardM",
    )

    s = sub(
        s,
        "[[cyl(.18,1,0),woodM]],[[cyl(.16,1,0),woodM]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]]];",
        """[[cyl(.18,1,0),woodM]],[[cyl(.16,1,0),woodM]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]],
[[geos[0],boardM]],
[[bx(1,.28,1.45,0,.36,.225),conc]],
[[mg([bx(1,.28,.95,0,-.36,.025),bx(1,.28,.8,0,-.08,.1),bx(1,.28,.65,0,.2,.175),bx(1,.16,.5,0,.42,.25)]),conc]],
[[mg([bx(1,.14,1,0,.43),bx(1,.08,.08,0,.34,0),bx(.08,.08,1,0,.34,0)].concat([-.33,0,.33].flatMap(x=>[-.33,0,.33].map(z=>bx(.28,.1,.28,x,.36,z))))),conc]],
[[mg([bx(.22,1,.55,-.39,0,-.225),bx(.22,1,.55,.39,0,-.225),bx(.78,.18,.55,0,.41,-.225)]),conc]],
[[mg([bx(1,.55,.35,0,-.225),bx(.08,.3,.35,-.46,.35),bx(.08,.3,.35,.46,.35),bx(1,.08,.35,0,.49)]),conc],[bx(.84,.22,.04,0,.35,0),glass]],
[[bx(1,.22,.9,0,-.39),conc]],
[[mg([bx(.18,1,.45,-.41,0,-.275),bx(.18,1,.45,.41,0,-.275),bx(1,1,.18,0,0,-.41)]),conc]],
[[mg([bx(.4,.14,.55,0,.38,-.1),bx(.16,.1,.4,0,.3,-.42),bx(.22,.08,.18,0,.26,-.55)]),conc]]];""",
        "defs",
    )

    s = sub(
        s,
        "const CATS=[['Structure',[0,1,34,2,7,9,12,10,6]],['Shapes',[4,36,28,29,30,38]],['Openings',[3,8,23,22,21,32,33,31]],['Detail',[11,25,13,27,26,37,16,39,35]],['Nature',[5,14,15,17,24,40,41,42]],['Material',[18,19,20,0]],['Pipes',[43,44,45,46]]];",
        "const CATS=[['Structure',[0,1,34,48,49,50,2,9,10,6]],['Shapes',[4,36,28,29,30,38,12,7,35,37]],['Openings',[3,51,52,8,23,22,21,32,33,31]],['Detail',[53,54,55,11,25,13,27,26,39,16]],['Nature',[5,14,15,17,24,40,41,42]],['Material',[47,18,19,20,0]],['Pipes',[43,44,45,46]]];",
        "CATS",
    )

    s = sub(
        s,
        "case 39:return fy<.35&&Math.abs(lz-.5)<.2&&Math.abs(lx-.5)<.45;default:return true}}",
        "case 39:return fy<.35&&Math.abs(lz-.5)<.2&&Math.abs(lx-.5)<.45;case 43:case 44:case 45:case 46:return false;"
        "case 48:return fy>.72;case 49:return lz<.55-.28*fy;case 50:return fy>.86;"
        "case 51:return Math.abs(lz-.5)<.275&&!(lx>.22&&lx<.78&&fy<.82);"
        "case 52:return Math.abs(lz-.5)<.175&&!(lx>.1&&lx<.9&&fy>.55&&fy<.85);"
        "case 53:return fy<.22;case 54:return lz<.18||Math.abs(lx-.5)>.32;case 55:return fy>.65&&Math.abs(lz-.5)<.35;default:return true}}",
        "solid",
    )

    s = sub(
        s,
        "const MOODS={Overcast:{el:40,az:130,fog:38,top:0x8e9498,hor:0xb4b8b8,sun:0xfff1dc,si:2.4,hi:.45,sat:0},Noon:{el:78,az:120,fog:10,top:0x4f86c6,hor:0xbfd4e4,sun:0xfff6e8,si:3.2,hi:.7,sat:.75},'Golden hour':{el:11,az:60,fog:28,top:0x5f7aa6,hor:0xf0b080,sun:0xffa860,si:3.4,hi:.5,sat:.8},'Blue hour':{el:4,az:250,fog:34,top:0x1c2a50,hor:0x7f8fb0,sun:0x8fa5e0,si:.9,hi:.55,sat:.55}};",
        "const MOODS={Overcast:{el:40,az:130,fog:38,top:0x8e9498,hor:0xb4b8b8,sun:0xfff1dc,si:2.4,hi:.45,sat:0},Noon:{el:78,az:120,fog:10,top:0x4f86c6,hor:0xbfd4e4,sun:0xfff6e8,si:3.2,hi:.7,sat:.75},'Golden hour':{el:11,az:60,fog:28,top:0x5f7aa6,hor:0xf0b080,sun:0xffa860,si:3.4,hi:.5,sat:.8},'Blue hour':{el:4,az:250,fog:34,top:0x1c2a50,hor:0x7f8fb0,sun:0x8fa5e0,si:.9,hi:.55,sat:.55},'Concrete overcast':{el:26,az:150,fog:52,top:0x6a7074,hor:0x9aa0a2,sun:0xd8dde0,si:1.55,hi:.38,sat:0}};",
        "MOODS",
    )

    a = s.find("function starter(){")
    b = s.find("// boot")
    if a < 0 or b < 0:
        raise SystemExit("starter/boot markers missing")
    starter = """function starter(){cells.clear();while(cables.length)delCab(cables[0]);pipeClearAll();N=10;$('plot').value=N;
// Brutalist L-corridor: board-form walls, chase, cantilever soffit, plants, pipes
for(let z=1;z<=6;z++){for(let y=0;y<4;y++){put(1,y,z,47,0);put(4,y,z,47,0)}put(2,0,z,47,0);put(3,0,z,47,0);put(2,3,z,48,2);put(3,3,z,48,2)}
for(let x=1;x<=7;x++){for(let y=0;y<4;y++){put(x,y,7,47,1);if(x>=5)put(x,y,4,47,3)}put(x,0,5,47,0);put(x,0,6,47,0);if(x>=2&&x<=6)put(x,3,6,50,0)}
for(let y=0;y<4;y++)put(1,y,4,54,0);
put(1,0,2,55,0);put(4,3,3,52,0);put(2,0,1,53,0);put(3,0,1,53,0);
for(let y=1;y<3;y++)put(7,y,5,47,1);put(7,0,5,51,1);put(7,3,5,48,1);
put(2,0,3,5,0);put(3,0,5,5,0);put(5,0,6,5,0);
put(6,1,6,14,0);put(0,0,3,17,0);
pipeLoad([{cx:1,cy:1,cz:3,face:0,u:1,v:1,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:1,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:0,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:2,kind:'valve',rot:1,auto:true},{cx:1,cy:0,cz:3,face:0,u:1,v:0,kind:'outlet',rot:0}]);
$('mood').value='Concrete overcast';const M=MOODS['Concrete overcast'];$('sun').value=M.el;$('az').value=M.az;$('fog').value=M.fog;sun();ambSync();
}
"""
    s = s[:a] + starter + s[b:]
    print("ok starter")

    HTML.write_text(s, encoding="utf-8")
    print(f"patched {HTML} size={HTML.stat().st_size}")


if __name__ == "__main__":
    main()
