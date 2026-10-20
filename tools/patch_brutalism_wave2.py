"""Inject brutalism wave-2 pieces (Pilotis…Corbel) into Creating Constructs.html."""
from __future__ import annotations
from pathlib import Path

HTML = Path(__file__).resolve().parents[1] / "Creating Constructs.html"


def sub(s: str, old: str, new: str, label: str) -> str:
    if old not in s:
        raise SystemExit(f"MISSING: {label}\n{old[:160]!r}")
    print("ok", label)
    return s.replace(old, new, 1)


def main() -> None:
    s = HTML.read_text(encoding="utf-8")

    s = sub(
        s,
        "TN=['Block','Slab','Pillar','Glass','Slope','Moss','Stairs','Column','Window','Panel','Arch','Railing','Beam','Light','Tree','Ivy','Cable','Planter','Brick','Timber','Steel','Fins','Cobogó','Doorway','Pool','Ladder','Bollard','Sconce','Dome','Silo','Vault','Fence','Slit','Ribbon','Floor','Pergola','Fine stairs','Pendant','Obelisk','Bench','Water','Birch','Pine','Pipe','Valve','Gauge','Outlet','Boardform','Cantilever','Batter','Waffle','Portal','Clerestory','Plinth','Chase','Spout']",
        "TN=['Block','Slab','Pillar','Glass','Slope','Moss','Stairs','Column','Window','Panel','Arch','Railing','Beam','Light','Tree','Ivy','Cable','Planter','Brick','Timber','Steel','Fins','Cobogó','Doorway','Pool','Ladder','Bollard','Sconce','Dome','Silo','Vault','Fence','Slit','Ribbon','Floor','Pergola','Fine stairs','Pendant','Obelisk','Bench','Water','Birch','Pine','Pipe','Valve','Gauge','Outlet','Boardform','Cantilever','Batter','Waffle','Portal','Clerestory','Plinth','Chase','Spout','Pilotis','Megastep','Lightwell','Bridge','Core','Aggregate','Corbel']",
        "TN",
    )

    s = sub(
        s,
        "'#8f98a2','#c52828','#c8963e','#6b7378','#8a8680','#9a9690','#7a7670','#8e8a84','#85817b','#7e7a74','#6e6a64','#75716b','#6a6660']",
        "'#8f98a2','#c52828','#c8963e','#6b7378','#8a8680','#9a9690','#7a7670','#8e8a84','#85817b','#7e7a74','#6e6a64','#75716b','#6a6660','#908c86','#86827c','#7c7872','#827e78','#78746e','#9a9488','#8a867e']",
        "sw",
    )

    s = sub(
        s,
        "steelM=new THREE.MeshStandardMaterial({map:ST,bumpMap:ST,bumpScale:1.5,roughness:.45,metalness:.5}),boardM=new THREE.MeshStandardMaterial({map:BF,bumpMap:BF,bumpScale:1.35,roughness:.92}),waterM=new THREE.MeshStandardMaterial({color:0x2b4a52,roughness:.04,metalness:.4,transparent:true,opacity:.85});",
        "steelM=new THREE.MeshStandardMaterial({map:ST,bumpMap:ST,bumpScale:1.5,roughness:.45,metalness:.5}),boardM=new THREE.MeshStandardMaterial({map:BF,bumpMap:BF,bumpScale:1.35,roughness:.92}),aggM=new THREE.MeshStandardMaterial({map:tex,bumpMap:tex,bumpScale:1.4,color:0xb8b0a4,roughness:.98}),waterM=new THREE.MeshStandardMaterial({color:0x2b4a52,roughness:.04,metalness:.4,transparent:true,opacity:.85});",
        "aggM",
    )

    s = sub(
        s,
        "[[mg([bx(.4,.14,.55,0,.38,-.1),bx(.16,.1,.4,0,.3,-.42),bx(.22,.08,.18,0,.26,-.55)]),conc]]];",
        """[[mg([bx(.4,.14,.55,0,.38,-.1),bx(.16,.1,.4,0,.3,-.42),bx(.22,.08,.18,0,.26,-.55)]),conc]],
[[mg([cyl(.26,.34,1.83),bx(.85,.12,.85,0,1.94,0)]),conc]],
[[bx(2,.5,1,.5,-.25,0),conc]],
[[mg([bx(.18,2,2,-.41,.5,.5),bx(.18,2,2,1.41,.5,.5),bx(2,2,.18,.5,.5,-.41),bx(2,2,.18,.5,.5,1.41)]),conc]],
[[mg([bx(2,.12,1,.5,-.44,0),bx(2,.88,.14,.5,.06,-.43),bx(2,.88,.14,.5,.06,.43),bx(2,.1,1,.5,.45,0)]),conc]],
[[mg([bx(.18,2,1,-.41,.5,0),bx(.18,2,1,.41,.5,0),bx(1,2,.18,0,.5,.41),bx(1,.18,1,0,1.41,0)]),conc]],
[[geos[0],aggM]],
[[mg([bx(.55,.12,.5,0,.38,-.05),bx(.4,.12,.4,0,.24,0),bx(.26,.12,.3,0,.1,.05)]),conc]]];""",
        "defs wave2",
    )

    s = sub(
        s,
        "function footprintW(t){return (t===23||t===51)?2:1}\nfunction footprintH(t){return (t===23||t===51)?2:1}\nfunction eachFootprintCell(x,y,z,t,r,fn){const w=footprintW(t),h=footprintH(t);for(let ly=0;ly<h;ly++)for(let lx=0;lx<w;lx++){let dx=lx,dz=0;for(let i=0;i<r;i++){const nx=-dz;dz=dx;dx=nx}fn(x+dx,y+ly,z+dz)}}\nfunction footprintCovers(ax,ay,az,t,r,x,y,z){let hit=false;eachFootprintCell(ax,ay,az,t,r,(cx,cy,cz)=>{if(cx===x&&cy===y&&cz===z)hit=true});return hit}\nfunction footprintRoot(x,y,z){const v=cells.get(key(x,y,z));if(v)return [x,y,z,v];for(let ay=Math.max(0,y-1);ay<=y;ay++)for(let ax=x-1;ax<=x+1;ax++)for(let az=z-1;az<=z+1;az++){if(!inb(ax,ay,az))continue;const b=cells.get(key(ax,ay,az));if(!b||(footprintW(b.t)<=1&&footprintH(b.t)<=1))continue;if(footprintCovers(ax,ay,az,b.t,b.r,x,y,z))return [ax,ay,az,b]}return null}",
        "function footprintW(t){return (t===23||t===51||t===57||t===58||t===59)?2:1}\nfunction footprintD(t){return t===58?2:1}\nfunction footprintH(t){return (t===23||t===51||t===56||t===58||t===60)?2:1}\nfunction eachFootprintCell(x,y,z,t,r,fn){const w=footprintW(t),d=footprintD(t),h=footprintH(t);for(let ly=0;ly<h;ly++)for(let lz=0;lz<d;lz++)for(let lx=0;lx<w;lx++){let dx=lx,dz=lz;for(let i=0;i<r;i++){const nx=-dz;dz=dx;dx=nx}fn(x+dx,y+ly,z+dz)}}\nfunction footprintCovers(ax,ay,az,t,r,x,y,z){let hit=false;eachFootprintCell(ax,ay,az,t,r,(cx,cy,cz)=>{if(cx===x&&cy===y&&cz===z)hit=true});return hit}\nfunction footprintRoot(x,y,z){const v=cells.get(key(x,y,z));if(v)return [x,y,z,v];for(let ay=Math.max(0,y-1);ay<=y;ay++)for(let ax=x-1;ax<=x+1;ax++)for(let az=z-1;az<=z+1;az++){if(!inb(ax,ay,az))continue;const b=cells.get(key(ax,ay,az));if(!b||(footprintW(b.t)<=1&&footprintH(b.t)<=1&&footprintD(b.t)<=1))continue;if(footprintCovers(ax,ay,az,b.t,b.r,x,y,z))return [ax,ay,az,b]}return null}",
        "footprint helpers",
    )

    s = sub(
        s,
        "const CATS=[['Structure',[0,1,34,48,49,50,2,9,10,6]],['Shapes',[4,36,28,29,30,38,12,7,35,37]],['Openings',[3,51,52,8,23,22,21,32,33,31]],['Detail',[53,54,55,11,25,13,27,26,39,16]],['Nature',[5,14,15,17,24,40,41,42]],['Material',[47,18,19,20,0]],['Pipes',[43,44,45,46]]];",
        "const CATS=[['Structure',[0,1,34,48,49,50,56,57,58,59,60,2]],['Shapes',[4,36,28,29,30,38,12,7,35,37,6,9]],['Openings',[3,51,52,8,23,22,21,32,33,31,10]],['Detail',[53,54,55,62,11,25,13,27,26,39,16]],['Nature',[5,14,15,17,24,40,41,42]],['Material',[47,61,18,19,20,0]],['Pipes',[43,44,45,46]]];",
        "CATS",
    )

    s = sub(
        s,
        "case 53:return fy<.22;case 54:return lz<.18||Math.abs(lx-.5)>.32;case 55:return fy>.65&&Math.abs(lz-.5)<.35;default:return true}}",
        "case 53:return fy<.22;case 54:return lz<.18||Math.abs(lx-.5)>.32;case 55:return fy>.65&&Math.abs(lz-.5)<.35;"
        "case 56:return fy>1.66&&Math.abs(lx-.5)<.28&&Math.abs(lz-.5)<.28;"
        "case 57:return fy<.5;"
        "case 58:return lx<.18||lx>1.82||lz<.18||lz>1.82;"
        "case 59:return fy<.12||Math.abs(lz-.5)>.36||fy>.9;"
        "case 60:return Math.abs(lx-.5)>.32||lz>.41||fy>1.82;"
        "case 62:return fy>.05&&lz<.55-.35*fy&&Math.abs(lx-.5)<.3;"
        "default:return true}}",
        "solid wave2",
    )

    s = sub(
        s,
        """function starter(){cells.clear();while(cables.length)delCab(cables[0]);pipeClearAll();N=10;$('plot').value=N;
// Brutalist L-corridor: board-form walls, chase, cantilever soffit, plants, pipes
for(let z=1;z<=6;z++){for(let y=0;y<4;y++){put(1,y,z,47,0);put(4,y,z,47,0)}put(2,0,z,47,0);put(3,0,z,47,0);put(2,3,z,48,2);put(3,3,z,48,2)}
for(let x=1;x<=7;x++){for(let y=0;y<4;y++){put(x,y,7,47,1);if(x>=5)put(x,y,4,47,3)}put(x,0,5,47,0);put(x,0,6,47,0);if(x>=2&&x<=6)put(x,3,6,50,0)}
for(let y=0;y<4;y++)put(1,y,4,54,0);
put(1,0,2,55,0);put(4,3,3,52,0);put(2,0,1,53,0);put(3,0,1,53,0);
put(7,0,5,51,1);put(7,0,6,-1);put(7,1,5,-1);put(7,1,6,-1);put(7,2,5,47,1);put(7,3,5,48,1);
put(2,0,3,5,0);put(3,0,5,5,0);put(5,0,6,5,0);
put(6,1,6,14,0);put(0,0,3,17,0);
pipeLoad([{cx:1,cy:1,cz:3,face:0,u:1,v:1,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:1,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:0,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:2,kind:'valve',rot:1,auto:true},{cx:1,cy:0,cz:3,face:0,u:1,v:0,kind:'outlet',rot:0}]);
$('mood').value='Concrete overcast';const M=MOODS['Concrete overcast'];$('sun').value=M.el;$('az').value=M.az;$('fog').value=M.fog;sun();ambSync();
}""",
        """function starter(){cells.clear();while(cables.length)delCab(cables[0]);pipeClearAll();N=10;$('plot').value=N;
// Brutalist L-corridor + wave-2 megastructure accents
for(let z=1;z<=6;z++){for(let y=0;y<4;y++){put(1,y,z,47,0);put(4,y,z,47,0)}put(2,0,z,47,0);put(3,0,z,47,0);put(2,3,z,48,2);put(3,3,z,48,2)}
for(let x=1;x<=7;x++){for(let y=0;y<4;y++){put(x,y,7,47,1);if(x>=5)put(x,y,4,47,3)}put(x,0,5,47,0);put(x,0,6,47,0);if(x>=2&&x<=6)put(x,3,6,50,0)}
for(let y=0;y<4;y++)put(1,y,4,54,0);
put(1,0,2,55,0);put(4,3,3,52,0);put(2,0,1,53,0);put(3,0,1,53,0);
put(7,0,5,51,1);put(7,0,6,-1);put(7,1,5,-1);put(7,1,6,-1);put(7,2,5,47,1);put(7,3,5,48,1);
put(2,0,3,5,0);put(3,0,5,5,0);put(5,0,6,5,0);
put(6,1,6,14,0);put(0,0,3,17,0);
put(8,0,4,56,0);put(8,1,4,-1);
put(0,0,6,57,0);put(1,0,6,-1);
put(2,1,2,-1);put(3,1,2,-1);put(2,1,3,-1);put(3,1,3,-1);put(2,2,2,-1);put(3,2,2,-1);put(2,2,3,-1);put(3,2,3,-1);
put(2,1,2,58,0);
put(5,2,5,59,0);put(6,2,5,-1);
put(8,0,7,60,0);put(8,1,7,-1);
put(6,0,4,61,0);
put(2,2,4,62,0);put(3,2,4,62,0);
pipeLoad([{cx:1,cy:1,cz:3,face:0,u:1,v:1,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:1,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:0,kind:'pipe',rot:0},{cx:1,cy:2,cz:3,face:0,u:1,v:2,kind:'valve',rot:1,auto:true},{cx:1,cy:0,cz:3,face:0,u:1,v:0,kind:'outlet',rot:0}]);
$('mood').value='Concrete overcast';const M=MOODS['Concrete overcast'];$('sun').value=M.el;$('az').value=M.az;$('fog').value=M.fog;sun();ambSync();
}""",
        "starter",
    )

    HTML.write_text(s, encoding="utf-8")
    print("wrote", HTML)


if __name__ == "__main__":
    main()
