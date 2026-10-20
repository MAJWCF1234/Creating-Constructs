"""Inject 3×3 face pipe system into Creating Constructs.html."""
from __future__ import annotations
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HTML = ROOT / "Creating Constructs.html"
MOD = (ROOT / "tools" / "pipes_module.js").read_text(encoding="utf-8")


def sub_once(s: str, old: str, new: str, label: str) -> str:
    if old not in s:
        raise SystemExit(f"missing hook: {label}\n---\n{old[:120]}...")
    return s.replace(old, new, 1)


def main() -> None:
    s = HTML.read_text(encoding="utf-8")

    if "'Pipe'" not in s:
        s = sub_once(
            s,
            "TN=['Block','Slab','Pillar','Glass','Slope','Moss','Stairs','Column','Window','Panel','Arch','Railing','Beam','Light','Tree','Ivy','Cable','Planter','Brick','Timber','Steel','Fins','Cobogó','Doorway','Pool','Ladder','Bollard','Sconce','Dome','Silo','Vault','Fence','Slit','Ribbon','Floor','Pergola','Fine stairs','Pendant','Obelisk','Bench','Water','Birch','Pine']",
            "TN=['Block','Slab','Pillar','Glass','Slope','Moss','Stairs','Column','Window','Panel','Arch','Railing','Beam','Light','Tree','Ivy','Cable','Planter','Brick','Timber','Steel','Fins','Cobogó','Doorway','Pool','Ladder','Bollard','Sconce','Dome','Silo','Vault','Fence','Slit','Ribbon','Floor','Pergola','Fine stairs','Pendant','Obelisk','Bench','Water','Birch','Pine','Pipe','Valve','Gauge','Outlet']",
            "TN",
        )
        s = sub_once(
            s,
            "'#2b6a78','#7a9a55','#3d6b3a']",
            "'#2b6a78','#7a9a55','#3d6b3a','#8f98a2','#c52828','#c8963e','#6b7378']",
            "sw",
        )
        s = sub_once(
            s,
            "['Material',[18,19,20,0]]];",
            "['Material',[18,19,20,0]],['Pipes',[43,44,45,46]]];",
            "CATS",
        )
        s = sub_once(
            s,
            "[[cyl(.18,1,0),woodM]],[[cyl(.16,1,0),woodM]]];",
            "[[cyl(.18,1,0),woodM]],[[cyl(.16,1,0),woodM]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]],[[cyl(.06,1,0),dark]]];",
            "defs",
        )

    if "/*PIPE_SYSTEM*/" in s:
        a = s.index("/*PIPE_SYSTEM*/")
        b = s.index("/*PIPE_SYSTEM_END*/") + len("/*PIPE_SYSTEM_END*/")
        s = s[:a] + MOD + s[b:]
    else:
        needle = "const natureRoot=new THREE.Group();S.add(natureRoot);"
        s = sub_once(s, needle, needle + "\n" + MOD, "natureRoot inject")

    if "v.t>=43&&v.t<=46" not in s:
        s = sub_once(
            s,
            "if(v.t!==15&&v.t!==14&&v.t!==41&&v.t!==42){meshes[v.t]",
            "if(v.t!==15&&v.t!==14&&v.t!==41&&v.t!==42&&!(v.t>=43&&v.t<=46)){meshes[v.t]",
            "refresh skip",
        )

    if "pipes:pipeSerialize" not in s:
        s = sub_once(
            s,
            "cab:cables.map(c=>[...c.a.toArray(),...c.b.toArray()])}))",
            "cab:cables.map(c=>[...c.a.toArray(),...c.b.toArray()]),pipes:pipeSerialize()}))",
            "autosave",
        )

    if "pipeLoad(d.pipes" not in s:
        s = sub_once(
            s,
            "(d.cab||[]).forEach(a=>addCab(a.slice(0,3),a.slice(3)));",
            "(d.cab||[]).forEach(a=>addCab(a.slice(0,3),a.slice(3)));pipeLoad(d.pipes||[]);",
            "load",
        )

    if "isPipeTool(type)" not in s or "function act(e){if(!hit)return;if(isPipeTool" not in s:
        old_act = "function act(e){if(!hit)return;if(e.shiftKey||erase||keys.KeyX){if(hit.rem){const prev=cells.get(key(...hit.rem));sfx('break',prev?prev.t:0);put(...hit.rem,-1,0,true);refresh()}}else if(hit.add){const k=key(...hit.add);if(!cells.has(k)){const q=resolve();sfx('place',q[0]);put(...hit.add,q[0],q[1],true);refresh()}}}"
        new_act = (
            "function act(e){if(!hit)return;"
            "if(isPipeTool(type)){const cell=pipeHitFromPick(hit);if(!cell)return;"
            "if(e.shiftKey||erase||keys.KeyX){if(pipeDel(cell)){sfx('break',43);save()}}"
            "else if(pipePaint(cell,type,true)){sfx('place',43);save()}return}"
            "if(e.shiftKey||erase||keys.KeyX){if(hit.rem){const prev=cells.get(key(...hit.rem));sfx('break',prev?prev.t:0);put(...hit.rem,-1,0,true);refresh()}}"
            "else if(hit.add){const k=key(...hit.add);if(!cells.has(k)){const q=resolve();sfx('place',q[0]);put(...hit.add,q[0],q[1],true);refresh()}}}"
        )
        if "function act(e){if(!hit)return;if(isPipeTool" not in s:
            s = sub_once(s, old_act, new_act, "act")

    if "pipeStroke" not in s:
        s = sub_once(
            s,
            "const undo=[];let stroke=null;",
            "const undo=[];let stroke=null,pipeStroke=null;",
            "decl",
        )
        s = sub_once(
            s,
            "if(btn==0){if(type==16){mx=e.clientX;my=e.clientY;hit=pick();cabClick(e);return}stroke=new Map();drawing=true;mx=e.clientX;my=e.clientY;hit=pick();act(e)}});",
            "if(btn==0){if(type==16){mx=e.clientX;my=e.clientY;hit=pick();cabClick(e);return}"
            "if(isPipeTool(type)){mx=e.clientX;my=e.clientY;hit=pick();pipeStroke=pipeHitFromPick(hit);drawing=true;act(e);return}"
            "stroke=new Map();drawing=true;mx=e.clientX;my=e.clientY;hit=pick();act(e)}});",
            "mousedown",
        )
        s = sub_once(
            s,
            "addEventListener('mouseup',()=>{if(stroke&&stroke.size){undo.push(stroke)}stroke=null;drawing=false;btn=-1});",
            "addEventListener('mouseup',()=>{if(stroke&&stroke.size){undo.push(stroke)}stroke=null;pipeStroke=null;drawing=false;btn=-1});",
            "mouseup",
        )
        s = sub_once(
            s,
            "else if(drawing){hit=pick();act(e)}});",
            "else if(drawing){hit=pick();"
            "if(isPipeTool(type)){const c=pipeHitFromPick(hit);if(c){"
            "if(type===43&&pipeStroke){pipeDrawLine(pipeStroke,c,43);pipeStroke=c;save()}"
            "else if(!pipeStroke||c.cx!==pipeStroke.cx||c.cy!==pipeStroke.cy||c.cz!==pipeStroke.cz||c.face!==pipeStroke.face||c.u!==pipeStroke.u||c.v!==pipeStroke.v)"
            "{pipePaint(c,type,false);pipeStroke=c;save()}}}"
            "else act(e)}});",
            "mousemove draw",
        )

    if "pipeManual=true" not in s:
        s = sub_once(
            s,
            "if(e.code=='KeyR'){rot=(rot+1)%4;ui()}",
            "if(e.code=='KeyR'){rot=(rot+1)%4;if(type===44||type===45)pipeManual=true;ui()}"
            "if(e.code=='KeyA'&&(type===44||type===45)){pipeManual=false;ui()}",
            "KeyR/A",
        )

    s = s.replace("setcat((cat+5)%6)", "setcat((cat+CATS.length-1)%CATS.length)")
    s = s.replace("setcat((cat+1)%6)", "setcat((cat+1)%CATS.length)")

    old_hint = "document.getElementById('hint').textContent=walk?'WASD move · Shift run · Space jump · C crouch · Tab to build':(type==16?'Cable: click two faces · Esc cancels':'Click place · drag fill · X or Shift erase · R rotate '+rot*90+'°')}"
    new_hint = "document.getElementById('hint').textContent=walk?'WASD move · Shift run · Space jump · C crouch · Tab to build':(type==16?'Cable: click two faces · Esc cancels':isPipeTool(type)?'Pipes: drag faces · 3×3/m · Valve/Gauge R axis · A auto · Outlet=ground wall bottom':'Click place · drag fill · X or Shift erase · R rotate '+rot*90+'°')}"
    if "isPipeTool(type)?'Pipes:" not in s:
        s = sub_once(s, old_hint, new_hint, "hint")

    old_ghost = "else if(!er&&hit.add){const q=resolve();gm.geometry=q[0]==15?ivyGhostGeo:defs[q[0]][0][0];gm.position.set(hit.add[0]+.5,hit.add[1]+.5,hit.add[2]+.5);gm.rotation.y=q[1]*Math.PI/2;gm.visible=true}}"
    new_ghost = (
        "else if(!er&&hit.add&&!isPipeTool(type)){const q=resolve();gm.geometry=q[0]==15?ivyGhostGeo:defs[q[0]][0][0];"
        "gm.position.set(hit.add[0]+.5,hit.add[1]+.5,hit.add[2]+.5);gm.rotation.y=q[1]*Math.PI/2;gm.visible=true}"
        "else if(isPipeTool(type))gm.visible=false}"
    )
    if "isPipeTool(type))gm.visible=false" not in s:
        s = sub_once(s, old_ghost, new_ghost, "ghost")

    # wdata() share export
    if "pipes:pipeSerialize()" not in s or s.count("pipes:pipeSerialize") < 2:
        # find wdata function
        if "function wdata()" in s and "pipes:" not in s[s.find("function wdata()") : s.find("function wdata()") + 400]:
            i = s.find("function wdata()")
            j = s.find("cab:", i)
            if j > 0 and "pipes:" not in s[j : j + 120]:
                k = s.find("})", j)
                # insert before closing of return object — look for cab line end
                line_end = s.find("\n", j)
                # typically cab:...}]
                pass
        # also share uses wdata
        marker = "function wdata(){return{N,c:[...cells].map(([k,v])=>[k,{t:v.t,r:v.r}]),cab:cables.map(c=>[...c.a.toArray(),...c.b.toArray()])}"
        if marker in s and "pipes:pipeSerialize()" not in marker:
            s = sub_once(
                s,
                marker,
                "function wdata(){return{N,c:[...cells].map(([k,v])=>[k,{t:v.t,r:v.r}]),cab:cables.map(c=>[...c.a.toArray(),...c.b.toArray()]),pipes:pipeSerialize()}",
                "wdata",
            )

    # clear world should clear pipes — when cables cleared
    if "pipeClearAll()" not in s:
        s = s.replace(
            "while(cables.length)delCab(cables[0]);",
            "while(cables.length)delCab(cables[0]);pipeClearAll();",
            1,
        )

    HTML.write_text(s, encoding="utf-8")
    print(f"patched {HTML} size={HTML.stat().st_size}")


if __name__ == "__main__":
    main()
