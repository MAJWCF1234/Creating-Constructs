bool atmosphereExpanded=true,worldExpanded=false,audioExpanded=false;int uiModal=0,hoveredAction=-1,sliderActive=-1;
RECT sliderBounds{};void showUiModal(int mode);void openShare();
int uiDockWidth(){return std::min(760,W-32);}int uiDockHeight(){return 138;}
int uiSideHeight(){return walk?64:216+(audioExpanded?169:0)+(atmosphereExpanded?268:0)+(worldExpanded?127:0);}
void roundedBox(HDC dc,RECT r,COLORREF bg,COLORREF border,int radius=8){if(!RectVisible(dc,&r))return;HBRUSH b=CreateSolidBrush(bg);HPEN p=CreatePen(PS_SOLID,1,border);auto ob=SelectObject(dc,b),op=SelectObject(dc,p);RoundRect(dc,r.left,r.top,r.right,r.bottom,radius,radius);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(p);}
void glassPanel(HDC dc,RECT r){
 if(!dc||!RectVisible(dc,&r))return;int width=r.right-r.left,height=r.bottom-r.top;
 if(uiBackgroundReady&&uiBackgroundMap){ensureUiGlassRect(r);
  HDC blurDC=CreateCompatibleDC(dc),large=CreateCompatibleDC(dc),tint=CreateCompatibleDC(dc);
  int bw=width,bh=height;HBITMAP sb=CreateCompatibleBitmap(dc,bw,bh),lb=CreateCompatibleBitmap(dc,width,height),tb=CreateCompatibleBitmap(dc,1,1);
  auto so=SelectObject(blurDC,sb),lo=SelectObject(large,lb),to=SelectObject(tint,tb);
  BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=uiBackgroundWidth;info.bmiHeader.biHeight=-(LONG)uiBackgroundHeight;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
  SetStretchBltMode(blurDC,HALFTONE);StretchDIBits(blurDC,0,0,bw,bh,r.left,r.top,width,height,uiBlurred.data(),&info,DIB_RGB_COLORS,SRCCOPY);
  SetStretchBltMode(large,HALFTONE);StretchBlt(large,0,0,width,height,blurDC,0,0,bw,bh,SRCCOPY);RECT one{0,0,1,1};fill(tint,one,RGB(24,24,27));
  BLENDFUNCTION blend{AC_SRC_OVER,0,146,0};AlphaBlend(large,0,0,width,height,tint,0,0,1,1,blend);BitBlt(dc,r.left,r.top,width,height,large,0,0,SRCCOPY);
  SelectObject(blurDC,so);SelectObject(large,lo);SelectObject(tint,to);DeleteObject(sb);DeleteObject(lb);DeleteObject(tb);DeleteDC(blurDC);DeleteDC(large);DeleteDC(tint);
 }else fill(dc,r,RGB(35,35,38));
 HGDIOBJ ob=SelectObject(dc,GetStockObject(NULL_BRUSH));HPEN p=CreatePen(PS_SOLID,1,RGB(102,102,105));auto op=SelectObject(dc,p);RoundRect(dc,r.left,r.top,r.right,r.bottom,24,24);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(p);
}
void composeUi(HDC dc){
 actions.clear();if(hideUI)return;char b[256];
 auto label=[&](int x,int y,const char* s,HFONT f,COLORREF c){if(dc)text(dc,x,y,s,f,c);};
 auto button=[&](RECT r,const char* s,int id,bool on=false,bool big=false){actions.push_back({r,id});if(!dc||!RectVisible(dc,&r))return;COLORREF bg=on||big?accent:(hoveredAction==id?RGB(72,72,75):RGB(43,43,46));roundedBox(dc,r,bg,on||big?accent:RGB(99,99,102));SelectObject(dc,big?fontBold:font);SetTextColor(dc,on||big?RGB(255,255,255):ink);SetBkMode(dc,TRANSPARENT);DrawTextA(dc,s,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);};
 auto section=[&](int x,int& y,const char* name,bool expanded,int id){RECT lineR{x,y,x+190,y+1};if(dc)fill(dc,lineR,RGB(81,81,84));y+=8;std::string s=(expanded?"v  ":">  ")+std::string(name);RECT click{x,y-2,x+190,y+22};actions.push_back({click,id});label(x,y,s.c_str(),fontBold,muted);y+=28;};
 glassPanel(dc,{10,10,294,walk?90:212});label(22,18,"Creating Constructs",fontTitle,ink);
 if(!walk){RECT hint{22,48,282,146};if(dc){SelectObject(dc,font);SetTextColor(dc,muted);SetBkMode(dc,TRANSPARENT);DrawTextA(dc,"Click builds; drag to fill. Shift+click removes. 1-9 pick a piece, comma / period switch tabs. R rotates. Right-drag orbits, middle-drag pans, wheel zooms. Ctrl+Z undoes.",-1,&hint,DT_WORDBREAK);}sprintf_s(b,"%d pieces",pieces);label(22,150,b,font,muted);}
 button({22,walk?53:175,140,walk?79:201},"Controls (?)",24);button({150,walk?53:175,282,walk?79:201},"Saves",30);
 int x=W-222,y=22;glassPanel(dc,{x,10,x+212,10+uiSideHeight()});x+=11;
 button({x,y,x+190,y+36},walk?"Back to building (Tab)":"Walk inside (Tab)",1,false,true);y+=44;
 auto slider=[&](const char* name,float value,int id){label(x,y,name,font,ink);y+=21;RECT area{x,y-7,x+190,y+14};actions.push_back({area,id});if(dc){RECT track{x+3,y,x+187,y+4};fill(dc,track,RGB(92,92,95));RECT progress=track;progress.right=x+3+(int)(184*std::clamp(value,0.f,1.f));fill(dc,progress,accent);int cx=progress.right;HBRUSH brush=CreateSolidBrush(accent);auto old=SelectObject(dc,brush),pen=SelectObject(dc,GetStockObject(NULL_PEN));Ellipse(dc,cx-6,y-4,cx+7,y+9);SelectObject(dc,old);SelectObject(dc,pen);DeleteObject(brush);}y+=22;};
 if(!walk){
  button({x,y,x+92,y+28},"Undo",2);button({x+98,y,x+190,y+28},"Erase (X)",8,erasing);y+=40;
  section(x,y,"ATMOSPHERE",atmosphereExpanded,26);
  if(atmosphereExpanded){
   label(x,y,"Mood",font,ink);y+=18;const char* moods[]={"Overcast","Noon","Golden hour","Blue hour","Concrete overcast"};sprintf_s(b,"%s   v",moods[mood]);button({x,y,x+190,y+28},b,4);y+=38;

   slider("Sun height",(sunHeight-4)/86,300);slider("Sun angle",sunAngle/360,301);slider("Exposure",(exposure-.5f)/1.3f,302);slider("Fog",fog,303);
   button({x,y,x+59,y+28},"Night",3,night);button({x+65,y,x+125,y+28},"Rain",5,rain);button({x+131,y,x+190,y+28},"AO",6,ao);y+=40;
  }
  section(x,y,"WORLD",worldExpanded,27);
  if(worldExpanded){label(x,y,"Plot size",font,ink);y+=19;sprintf_s(b,"%d    v",world.n);button({x,y,x+190,y+28},b,7);y+=36;
   button({x,y,x+80,y+28},"Share",19);button({x+86,y,x+190,y+28},"Hide UI (H)",20);y+=36;button({x,y,x+190,y+28},"Clear plot",9);y+=36;
  }
  section(x,y,"AUDIO",audioExpanded,28);
  if(audioExpanded){char musicLabel[80];sprintf_s(musicLabel,"Music - %s",moodMusicNames[std::clamp(mood,0,3)]);slider(musicLabel,musicVolume,304);slider("Ambience",ambienceVolume,305);slider("Effects",effectsVolume,306);button({x,y,x+190,y+28},audioMuted?"Unmute (M)":"Mute (M)",29,audioMuted);y+=40;}
 }
 if(!walk){int width=uiDockWidth(),dx=(W-width)/2,dy=H-uiDockHeight()-12,inner=width-20;glassPanel(dc,{dx,dy,dx+width,dy+uiDockHeight()});
  int cw=inner/CategoryCount;for(int i=0;i<CategoryCount;i++)button({dx+10+i*cw,dy+10,dx+10+(i+1)*cw-6,dy+38},cats[i],100+i,category==i);
  int count=0;while(count<10&&catTypes[category][count]>=0)count++;int bw=std::min(102,inner/std::max(1,count));int start=dx+(width-count*bw)/2;
  for(int i=0;i<count;i++){int t=catTypes[category][i];RECT r{start+i*bw,dy+46,start+(i+1)*bw-6,dy+126};actions.push_back({r,200+t});if(dc&&RectVisible(dc,&r)){bool on=t==selected;roundedBox(dc,r,on?RGB(75,46,34):hoveredAction==200+t?RGB(68,68,71):RGB(43,43,46),on?accent:RGB(93,93,96));drawThumbnail(dc,(r.left+r.right-48)/2,r.top+4,t);sprintf_s(b,"%d %s",(i+1)%10,names[t]);RECT tx{r.left,r.bottom-22,r.right,r.bottom-4};SelectObject(dc,font);SetTextColor(dc,ink);SetBkMode(dc,TRANSPARENT);DrawTextA(dc,b,-1,&tx,DT_CENTER|DT_VCENTER|DT_SINGLELINE);}}
 }
 if(uiModal){int width=uiModal==2?560:620,height=uiModal==1?510:uiModal==3?560:330,mx=(W-width)/2,my=(H-height)/2;glassPanel(dc,{mx,my,mx+width,my+height});label(mx+18,my+16,uiModal==1?"Controls":uiModal==3?"Saved worlds":"Share world",fontTitle,ink);
  if(uiModal==1){const char* keysText[]={"Place","Erase","Rotate piece","Pick a piece","Switch tab","Cable","Camera","Undo","Walk mode","Parkour","Photo"};const char* help[]={"Click, or hold and drag to fill","Shift+click, hold X, or the Erase toggle","R; lights, ivy and ladders auto-face walls","1-9 within a tab; [ / ]; hold mouse and scroll","Comma and period","Click two faces; Esc cancels","Right-drag orbit, middle-drag pan, wheel zoom","Ctrl+Z","Tab; WASD, Shift run, Space jump, C crouch","Hold W against a wall; Space wall-jumps; W climbs ledges","H hides the interface; M mutes audio"};
   for(int i=0;i<11;i++){int yy=my+61+i*35;label(mx+20,yy,keysText[i],fontBold,ink);if(dc){RECT tx{mx+146,yy,mx+width-18,yy+33};SelectObject(dc,font);SetTextColor(dc,muted);DrawTextA(dc,help[i],-1,&tx,DT_WORDBREAK);}}
   button({mx+18,my+height-47,mx+width-18,my+height-17},"Close",22);
  }else if(uiModal==2){label(mx+18,my+51,"Copy this text to export. Paste a world here and press Load.",font,muted);button({mx+18,my+height-47,mx+width/2-4,my+height-17},"Load",23);button({mx+width/2+4,my+height-47,mx+width-18,my+height-17},"Close",22);}
  else if(uiModal==3){
   label(mx+18,my+51,"Name this world, then save a new copy.",font,muted);
   label(mx+18,my+106,"Your saved worlds",fontBold,ink);
   if(saveEntries.empty())label(mx+18,my+145,"No saves yet. Your current world is ready to save.",font,muted);
   for(int row=0;row<5;row++){int index=savePage*5+row;if(index>=(int)saveEntries.size())break;
    auto& entry=saveEntries[index];RECT rr{mx+18,my+135+row*61,mx+width-18,my+190+row*61};button(rr,"",400+row,index==saveSelected);
    if(dc){RECT title{rr.left+12,rr.top+7,rr.right-12,rr.top+27};SelectObject(dc,fontBold);SetTextColor(dc,ink);DrawTextW(dc,entry.name.c_str(),-1,&title,DT_SINGLELINE|DT_END_ELLIPSIS);sprintf_s(b,"%d pieces",entry.pieces);text(dc,rr.left+12,rr.top+30,b,font,muted);}
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

