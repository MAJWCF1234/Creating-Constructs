bool atmosphereExpanded=true,worldExpanded=false,audioExpanded=false,settingsExpanded=false;int uiModal=0,hoveredAction=-1,sliderActive=-1,dockScrollX=0;
int menuSelection=0;
RECT sliderBounds{};void showUiModal(int mode);void openShare();
int uiDockWidth(){return std::min(840,W-32);}
int uiDockPieceCount(){if(category==5)return SurfaceCount;int n=0;while(n<CategorySlots&&catTypes[category][n]>=0)n++;return n;}
int uiDockHeight(){return category==5?190:148;}
int uiDockPieceStride(){return 96;}
int uiDockMaxScroll(){int inner=uiDockWidth()-20;return std::max(0,uiDockPieceCount()*uiDockPieceStride()-inner);}
void uiDockClampScroll(){dockScrollX=std::clamp(dockScrollX,0,uiDockMaxScroll());}
// Collapsed base must clear Walk + Undo row + 4 section headers + bottom pad (SETTINGS was clipping at 216).
int uiSideHeight(){return walk?64:258+(audioExpanded?169:0)+(atmosphereExpanded?348:0)+(worldExpanded?163:0)+(settingsExpanded?300:0);}
void roundedBox(HDC dc,RECT r,COLORREF bg,COLORREF border,int radius=8){if(!RectVisible(dc,&r))return;HBRUSH b=CreateSolidBrush(bg);HPEN p=CreatePen(PS_SOLID,1,border);auto ob=SelectObject(dc,b),op=SelectObject(dc,p);RoundRect(dc,r.left,r.top,r.right,r.bottom,radius,radius);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(p);}
void glassPanel(HDC dc,RECT r){
 if(!dc||!RectVisible(dc,&r))return;int width=r.right-r.left,height=r.bottom-r.top;
 if(uiBackgroundReady&&uiBackgroundMap){ensureUiGlassRect(r);
  BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=uiBackgroundWidth;info.bmiHeader.biHeight=-(LONG)uiBackgroundHeight;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
  SetStretchBltMode(dc,HALFTONE);StretchDIBits(dc,r.left,r.top,width,height,r.left,r.top,width,height,uiBlurred.data(),&info,DIB_RGB_COLORS,SRCCOPY);
  HDC tint=CreateCompatibleDC(dc);HBITMAP pixel=CreateCompatibleBitmap(dc,1,1);auto previous=SelectObject(tint,pixel);RECT one{0,0,1,1};fill(tint,one,RGB(24,24,27));
  BLENDFUNCTION blend{AC_SRC_OVER,0,146,0};AlphaBlend(dc,r.left,r.top,width,height,tint,0,0,1,1,blend);SelectObject(tint,previous);DeleteObject(pixel);DeleteDC(tint);

 }else fill(dc,r,RGB(35,35,38));
 HGDIOBJ ob=SelectObject(dc,GetStockObject(NULL_BRUSH));HPEN p=CreatePen(PS_SOLID,1,RGB(102,102,105));auto op=SelectObject(dc,p);RoundRect(dc,r.left,r.top,r.right,r.bottom,24,24);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(p);
}
// Warm, quiet cast-concrete surface. Stable pores do not shimmer between frames.
void menuSlab(HDC dc,RECT r){
 if(!dc||!RectVisible(dc,&r))return;
 fill(dc,r,RGB(54,55,49));
 uint32_t seed=173;
 for(int i=0;i<430;i++){seed=seed*1664525u+1013904223u;int x=r.left+3+seed%(r.right-r.left-6);seed=seed*1664525u+1013904223u;int y=r.top+3+seed%(r.bottom-r.top-6);int shade=48+(seed>>24)%13;SetPixelV(dc,x,y,RGB(shade,shade+1,shade-4));}
 fill(dc,{r.left,r.top,r.left+4,r.bottom},RGB(127,133,110));
 fill(dc,{r.left+4,r.top,r.right,r.top+1},RGB(103,105,91));
 fill(dc,{r.right-1,r.top,r.right,r.bottom},RGB(38,40,35));
 fill(dc,{r.left,r.bottom-2,r.right,r.bottom},RGB(32,34,29));
}
void composeUi(HDC dc){
 actions.clear();if(hideUI)return;char b[256];
 auto label=[&](int x,int y,const char* s,HFONT f,COLORREF c){if(dc)text(dc,x,y,s,f,menuActive?(c==ink?RGB(235,229,212):c==muted?RGB(184,188,166):c):c);};
 auto button=[&](RECT r,const char* s,int id,bool on=false,bool big=false){actions.push_back({r,id});if(!dc||!RectVisible(dc,&r))return;COLORREF bg=on||big?accent:(hoveredAction==id?RGB(72,72,75):RGB(43,43,46));if(menuActive){bool focus=on||hoveredAction==id;bg=big?(focus?RGB(183,119,83):RGB(163,102,73)):(focus?RGB(78,83,68):RGB(46,48,42));roundedBox(dc,r,bg,big?RGB(192,139,101):focus?RGB(151,160,128):RGB(88,93,78),3);if(on)fill(dc,{r.left+1,r.top+6,r.left+4,r.bottom-6},RGB(224,213,183));}else roundedBox(dc,r,bg,on||big?accent:RGB(99,99,102));SelectObject(dc,big?fontBold:font);SetTextColor(dc,menuActive?RGB(235,229,212):on||big?RGB(255,255,255):ink);SetBkMode(dc,TRANSPARENT);uiDrawText(dc,s,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);};
 auto section=[&](int x,int& y,const char* name,bool expanded,int id){RECT lineR{x,y,x+190,y+1};if(dc)fill(dc,lineR,RGB(81,81,84));y+=8;std::string s=(expanded?"v  ":">  ")+std::string(name);RECT click{x,y-2,x+190,y+22};actions.push_back({click,id});label(x,y,s.c_str(),fontBold,muted);y+=28;};
 // Menu pages share the live world backdrop, with no editor controls behind them.
 if(menuActive&&(uiModal==6||uiModal==7)){
  int mx=(W-620)/2,my=(H-510)/2;menuSlab(dc,{mx,my,mx+620,my+510});
  if(uiModal==6){
   if(dc){HFONT title=CreateFontW(-46,0,0,0,800,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Arial Narrow");text(dc,mx+36,my+29,"CREATING",title,RGB(235,229,212));text(dc,mx+36,my+76,"CONSTRUCTS",title,RGB(235,229,212));SelectObject(dc,font);DeleteObject(title);fill(dc,{mx+38,my+143,mx+582,my+144},RGB(92,98,81));fill(dc,{mx+38,my+143,mx+102,my+146},RGB(155,166,128));}
   label(mx+38,my+162,"A quiet place to build. Stay a while.",font,muted);
   const char* labels[]={"Play / continue","Saved worlds","Settings","Controls","Save and quit"};
   for(int i=0;i<5;i++)button({mx+38,my+207+i*45,mx+582,my+245+i*45},labels[i],60+i,menuSelection==i,i==0);
   label(mx+38,my+466,"Your world is kept safe.   Esc returns to play.",font,muted);
  }else{
   label(mx+30,my+25,"Settings",fontTitle,ink);label(mx+30,my+58,"Sound and camera",font,muted);
   auto menuSlider=[&](int yy,const char* name,float value,int id){label(mx+30,my+yy,name,font,ink);RECT r{mx+250,my+yy-2,mx+580,my+yy+22};actions.push_back({r,id});if(dc){int cy=my+yy+10,cx=r.left+(int)(330*std::clamp(value,0.f,1.f));fill(dc,{r.left,cy,r.right,cy+3},line);fill(dc,{r.left,cy,cx,cy+3},RGB(155,166,128));fill(dc,{cx-3,cy-5,cx+4,cy+8},ink);}};
   menuSlider(94,"Music",musicVolume,304);menuSlider(137,"Ambience",ambienceVolume,305);menuSlider(180,"Effects",effectsVolume,306);menuSlider(223,"Mouse sensitivity",(mouseSens-.35f)/1.85f,307);menuSlider(266,"Build field of view",(buildFov-40.f)/60.f,308);
   button({mx+30,my+315,mx+300,my+350},audioMuted?"Sound: muted":"Sound: on",29,audioMuted);
   button({mx+310,my+315,mx+590,my+350},camSchemeLabel(),36);
   sprintf_s(b,"Resolution: %d x %d",W,H);button({mx+30,my+365,mx+300,my+400},b,41);
   button({mx+310,my+365,mx+590,my+400},fullscreen?"Windowed (F11)":"Fullscreen (F11)",38);
   button({mx+30,my+445,mx+590,my+485},"Back",22);
  }return;
 }
 if(!menuActive){
 glassPanel(dc,{10,10,294,walk?126:248});label(22,18,"Creating Constructs",fontTitle,ink);
 if(!walk){RECT hint{22,48,282,146};if(dc){SelectObject(dc,font);SetTextColor(dc,muted);SetBkMode(dc,TRANSPARENT);char tip[360];sprintf_s(tip,"Click builds; drag to fill. Doorway/Portal/Bridge need empty footprint cells — walk through the opening. %s removes. 1-9 pieces, ,/. tabs. R rotates. Q/E raise/lower. %s.",camScheme==1?"X or Erase":"Shift+click",camHelpText());if(!placementStatus.empty())sprintf_s(tip,"%s",placementStatus.c_str());uiDrawText(dc,tip,-1,&hint,DT_WORDBREAK);}sprintf_s(b,"%d pieces",pieces);label(22,150,b,font,muted);}
 button({22,walk?53:175,140,walk?79:201},"Controls (?)",24);button({150,walk?53:175,282,walk?79:201},"Saves",30);
 if(!walk){button({22,208,282,236},"Viewpoints (V)",51);}
 if(walk&&!doorHint.empty())label(22,96,doorHint.c_str(),fontBold,ink);
 int x=W-222,y=22;glassPanel(dc,{x,10,x+212,10+uiSideHeight()});x+=11;
 button({x,y,x+190,y+36},walk?"Back to building (Tab)":"Walk inside (Tab)",1,false,true);y+=44;
 auto slider=[&](const char* name,float value,int id){label(x,y,name,font,ink);y+=21;RECT area{x,y-7,x+190,y+14};actions.push_back({area,id});if(dc){RECT track{x+3,y,x+187,y+4};fill(dc,track,RGB(92,92,95));RECT progress=track;progress.right=x+3+(int)(184*std::clamp(value,0.f,1.f));fill(dc,progress,accent);int cx=progress.right;HBRUSH brush=CreateSolidBrush(accent);auto old=SelectObject(dc,brush),pen=SelectObject(dc,GetStockObject(NULL_PEN));Ellipse(dc,cx-6,y-4,cx+7,y+9);SelectObject(dc,old);SelectObject(dc,pen);DeleteObject(brush);}y+=22;};
 if(!walk){
  button({x,y,x+60,y+28},"Undo",2);button({x+66,y,x+126,y+28},"Redo",37);button({x+132,y,x+190,y+28},"Erase",8,erasing);y+=40;
  section(x,y,"ATMOSPHERE",atmosphereExpanded,26);
  if(atmosphereExpanded){
   label(x,y,"Mood",font,ink);y+=18;const char* moods[]={"Overcast","Noon","Golden hour","Blue hour","Concrete overcast"};sprintf_s(b,"%s   v",moods[mood]);button({x,y,x+190,y+28},b,4);y+=38;

   slider("Sun height",(sunHeight-4)/86,300);slider("Sun angle",sunAngle/360,301);slider("Exposure",(exposure-.5f)/1.3f,302);slider("Fog",fog,303);slider("Clouds",clouds,309);
   button({x,y,x+59,y+28},"Night",3,night);button({x+65,y,x+125,y+28},"Rain",5,rain);button({x+131,y,x+190,y+28},"AO",6,ao);y+=36;
   button({x,y,x+190,y+28},"Wind",42,wind);y+=36;
  }
  section(x,y,"WORLD",worldExpanded,27);
  if(worldExpanded){label(x,y,"Plot size",font,ink);y+=19;sprintf_s(b,"%d    v",world.n);button({x,y,x+190,y+28},b,7);y+=36;
   button({x,y,x+80,y+28},"Share",19);button({x+86,y,x+190,y+28},"Hide UI (H)",20);y+=36;
   button({x,y,x+95,y+28},"Export OBJ",40);button({x+100,y,x+190,y+28},"Export FBX",43);y+=36;button({x,y,x+190,y+28},"Clear plot",9);y+=36;
  }
  section(x,y,"AUDIO",audioExpanded,28);
  if(audioExpanded){char musicLabel[80];sprintf_s(musicLabel,"Music - %s",moodMusicNames[std::clamp(mood,0,3)]);slider(musicLabel,musicVolume,304);slider("Ambience",ambienceVolume,305);slider("Effects",effectsVolume,306);button({x,y,x+190,y+28},audioMuted?"Unmute (M)":"Mute (M)",29,audioMuted);y+=40;}
  section(x,y,"SETTINGS",settingsExpanded,35);
  if(settingsExpanded){label(x,y,"Camera",font,ink);y+=18;button({x,y,x+190,y+28},camSchemeLabel(),36,camScheme==1);y+=36;
   slider("Mouse sens",(mouseSens-.35f)/1.85f,307);slider("Build FOV",(buildFov-40.f)/60.f,308);
   sprintf_s(b,"%dx%d   v",W,H);button({x,y,x+190,y+28},b,41);y+=36;
   button({x,y,x+190,y+28},fullscreen?"Windowed (F11)":"Fullscreen (F11)",38,fullscreen);y+=36;
   button({x,y,x+190,y+28},"Open photos folder",39);y+=36;
   RECT tip{x,y,x+190,y+40};if(dc){SelectObject(dc,font);SetTextColor(dc,muted);SetBkMode(dc,TRANSPARENT);uiDrawText(dc,"F12 photo. L last piece, F family. Gamepad: sticks, A place, B erase, X/Y undo/redo.",-1,&tip,DT_WORDBREAK);}y+=44;}
 }
 if(!walk){int width=uiDockWidth(),dockH=uiDockHeight(),dx=(W-width)/2,dy=H-dockH-12,inner=width-20;glassPanel(dc,{dx,dy,dx+width,dy+dockH});
  int cw=inner/CategoryCount;for(int i=0;i<CategoryCount;i++)button({dx+10+i*cw,dy+10,dx+10+(i+1)*cw-6,dy+38},cats[i],100+i,category==i);
  int count=uiDockPieceCount(),bw=uiDockPieceStride();uiDockClampScroll();
  RECT vis{dx+10,dy+46,dx+width-10,dy+128};int saved=dc?SaveDC(dc):0;if(dc)IntersectClipRect(dc,vis.left,vis.top,vis.right,vis.bottom);
  for(int i=0;i<count;i++){int t=category==5?i:catTypes[category][i],x0=dx+10-dockScrollX+i*bw;RECT rr{x0,dy+46,x0+bw-6,dy+126},hitRect;if(!IntersectRect(&hitRect,&rr,&vis))continue;int id=category==5?600+i:200+t;actions.push_back({hitRect,id});
   if(dc&&RectVisible(dc,&rr)){bool on=category==5?i==chosenSurface:t==selected;roundedBox(dc,rr,on?RGB(75,46,34):hoveredAction==id?RGB(68,68,71):RGB(43,43,46),on?accent:RGB(93,93,96));
    if(category==5)drawSurfaceThumbnail(dc,rr.left+23,rr.top+5,i);else drawThumbnail(dc,(rr.left+rr.right-48)/2,rr.top+4,t);
    sprintf_s(b,"%s",category==5?surfaceNames[i]:names[t]);RECT tx{rr.left+2,rr.bottom-24,rr.right-2,rr.bottom-2};SelectObject(dc,font);SetTextColor(dc,ink);SetBkMode(dc,TRANSPARENT);uiDrawText(dc,b,-1,&tx,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
   }
  }
  if(dc)RestoreDC(dc,saved);
  if(category==5){sprintf_s(b,"Rotate texture: %d degrees",surfaceRotation*90);button({dx+10,dy+140,dx+230,dy+175},b,55);const char* scales[]={"Pattern: large","Pattern: natural","Pattern: fine"};button({dx+240,dy+140,dx+430,dy+175},scales[surfaceScale],56);sprintf_s(b,"Building %s with %s",names[selected],surfaceNames[chosenSurface]);label(dx+443,dy+150,b,font,ink);}
 }
 }
 if(uiModal){actions.clear();int width=uiModal==2?560:620,height=uiModal==1?510:uiModal==3?560:uiModal==5?560:330,mx=(W-width)/2,my=(H-height)/2;glassPanel(dc,{mx,my,mx+width,my+height});if(menuActive)menuSlab(dc,{mx,my,mx+width,my+height});label(mx+18,my+16,uiModal==1?"Controls":uiModal==3?"Saved worlds":uiModal==5?"Named viewpoints":"Share world",fontTitle,ink);
  if(uiModal==1){const char* keysText[]={"Place","Erase","Rotate piece","Pick / family","Switch tab","Cable","Camera","Undo","Walk mode","Parkour","Photo"};const char* eraseHelp=camScheme==1?"Hold X, or the Erase toggle (Shift is orbit)":"Shift+click, hold X, or the Erase toggle";
   const char* help[]={"Click, or hold and drag to fill. Multi-cell pieces (Doorway, Portal, Bridge) need cleared neighbors — openings are walk-through",eraseHelp,"R; openings, screens, lights, ivy and ladders auto-face flush on a wall click","1-9 within a tab; [ / ]; hold+scroll cycles family variants; L last placed; F next family","Comma and period","Click two faces; Esc cancels",camHelpText(),"Ctrl+Z undo, Ctrl+Y redo","Tab; WASD, Shift run, Space jump, C crouch; F / click uses doors","Hold W against a wall; Space wall-jumps; W climbs ledges","H hides UI; M mute; F11 fullscreen; F12 photo; Export OBJ/FBX in World"};
   for(int i=0;i<11;i++){int yy=my+61+i*35;label(mx+20,yy,keysText[i],fontBold,ink);if(dc){RECT tx{mx+146,yy,mx+width-18,yy+33};SelectObject(dc,font);SetTextColor(dc,muted);uiDrawText(dc,help[i],-1,&tx,DT_WORDBREAK);}}
   button({mx+18,my+height-47,mx+width-18,my+height-17},"Close",22);
  }else if(uiModal==2){label(mx+18,my+51,"Copy this text to export. Paste a world here and press Load.",font,muted);button({mx+18,my+height-47,mx+width/2-4,my+height-17},"Load",23);button({mx+width/2+4,my+height-47,mx+width-18,my+height-17},"Close",22);}
  else if(uiModal==5){
   label(mx+18,my+51,"Name the current camera and mood, then save a viewpoint.",font,muted);
   for(int row=0;row<8;row++){int index=viewpointPage*8+row;if(index>=(int)world.viewpoints.size())break;auto title=world.viewpoints[index].name;int yy=my+120+row*36;RECT rr{mx+18,yy,mx+width-18,yy+30};button(rr,"",700+index);if(dc){SelectObject(dc,font);SetTextColor(dc,ink);DrawTextW(dc,title,-1,&rr,DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);}}
   label(mx+18,my+432,"Viewpoints travel with your saved world. Maximum 16.",font,muted);
   button({mx+18,my+460,mx+150,my+488},"Previous",53);button({mx+158,my+460,mx+290,my+488},"Next",54);
   button({mx+18,my+height-47,mx+300,my+height-17},"Save current viewpoint",52);button({mx+310,my+height-47,mx+width-18,my+height-17},"Close",22);
  }
  else if(uiModal==3){
   label(mx+18,my+51,"Name this world, then save a new copy.",font,muted);
   label(mx+18,my+106,"Your saved worlds",fontBold,ink);
   if(saveEntries.empty())label(mx+18,my+145,"No saves yet. Your current world is ready to save.",font,muted);
   for(int row=0;row<5;row++){int index=savePage*5+row;if(index>=(int)saveEntries.size())break;
    auto& entry=saveEntries[index];RECT rr{mx+18,my+135+row*61,mx+width-18,my+190+row*61};button(rr,"",400+row,index==saveSelected);
    if(dc){drawSaveThumbnail(dc,entry,{rr.left+5,rr.top+4,rr.left+91,rr.bottom-4});RECT title{rr.left+102,rr.top+7,rr.right-12,rr.top+27};SelectObject(dc,fontBold);SetTextColor(dc,ink);DrawTextW(dc,entry.name.c_str(),-1,&title,DT_SINGLELINE|DT_END_ELLIPSIS);sprintf_s(b,"%d pieces",entry.pieces);text(dc,rr.left+102,rr.top+30,b,font,muted);}
   }
   int pages=std::max(1,((int)saveEntries.size()+4)/5);sprintf_s(b,"Page %d / %d",savePage+1,pages);label(mx+18,my+446,b,font,muted);
   button({mx+width-170,my+440,mx+width-98,my+466},"Previous",33);button({mx+width-90,my+440,mx+width-18,my+466},"Next",34);
   label(mx+18,my+478,savesStatus.c_str(),font,muted);
   button({mx+18,my+height-47,mx+210,my+height-17},"Save new copy",31);button({mx+218,my+height-47,mx+width-108,my+height-17},"Load selected",32);button({mx+width-100,my+height-47,mx+width-18,my+height-17},"Close",22);
  }
 }
}
void rebuildActions(){composeUi(nullptr);}
void paintUI(HDC dc){composeUi(dc);}

