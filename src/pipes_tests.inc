void pipesSelfTest(){
 World original=world;auto oldHistory=history,oldRedo=redoStack;int oldTurn=turn;V oldOrigin=origin;bool oldGhost=ghostMode;ghostMode=false;origin={};turn=0;
 world=World{};world.n=10;for(int x=2;x<7;x++)for(int z=2;z<7;z++)for(int y=1;y<6;y++)put(x,y,z,0);
 const int du[4]={0,0,1,-1},dv[4]={1,-1,0,0};bool steps=true,reciprocal=true;
 for(int face=0;face<6;face++)for(int u=0;u<3;u++)for(int v=0;v<3;v++)for(int d=0;d<4;d++){
  PipeNode n{4,3,4,(int8_t)face,(int8_t)u,(int8_t)v,0,0,0},next,back;
  steps=steps&&pipeStep(n,du[d],dv[d],next)&&fabsf(len(pipeCenter(next)-pipeCenter(n))-PipePitch)<.00001f;
  reciprocal=reciprocal&&pipeStep(next,du[d^1],dv[d^1],back)&&pipeSameCell(n,back);
 }
 check(steps&&reciprocal,"pipes cross host-cell seams continuously and reversibly on every face and slot");
 for(int face=0;face<6;face++)for(int kind=1;kind<=2;kind++){
  PipeNode n{4,3,4,(int8_t)face,1,1,(int8_t)kind,0,1},neighbor=n;neighbor.kind=0;neighbor.u=2;world.pipes={n,neighbor};settlePipes();
  check(world.pipes[0].rot==0,"automatic valve/gauge follows horizontal neighbors");
  world.pipes[1].u=1;world.pipes[1].v=2;settlePipes();check(world.pipes[0].rot==1,"automatic valve/gauge adapts when its neighbor changes axis");
  world.pipes[0].autoAlign=0;world.pipes[0].rot=2;settlePipes();check(world.pipes[0].rot==2,"manual pipe orientation survives automatic settling");
 }
 world.pipes={{4,3,4,4,1,1,0,0,0}};remember();world.pipes.push_back({4,3,4,4,2,1,2,1,1});undoEdit();check(world.pipes.size()==1,"undo restores pipe overlays");redoEdit();check(world.pipes.size()==2&&world.pipes[1].kind==2,"redo restores pipe kind and auto state");
 bool valid=true;for(int kind=0;kind<4;kind++)for(int r=0;r<4;r++)for(int d=0;d<4;d++){
  PipeNode n{4,3,4,4,1,1,(int8_t)kind,(int8_t)r,0};bool expected=kind==0||(kind==3?d==0:(r%2==0?d>=2:d<2));valid=valid&&pipeAllowed(n,d)==expected;
 }check(valid,"all pipe kinds and rotations expose the intended connection ports");
 world=original;history=oldHistory;redoStack=oldRedo;turn=oldTurn;origin=oldOrigin;ghostMode=oldGhost;vertices.clear();transparent.clear();dirty=true;
}
