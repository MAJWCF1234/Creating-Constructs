void addAction(RECT r,int id){actions.push_back({r,id});}
void rebuildActions(){actions.clear();if(hideUI)return;
int x=W-210+12,y=21;addAction({x,y,x+176,y+28},1);
if(!walk){y+=36;addAction({x,y,x+84,y+28},2);addAction({x+92,y,x+176,y+28},3);y+=36;
 auto slider=[&](int id){y+=20;addAction({x,y-7,x+176,y+14},id);y+=22;};
 slider(300);slider(301);y+=18;addAction({x,y,x+176,y+28},4);y+=34;slider(302);slider(303);
 addAction({x,y,x+84,y+28},5);addAction({x+92,y,x+176,y+28},6);y+=36;
 addAction({x,y,x+176,y+28},7);y+=36;addAction({x,y,x+84,y+28},19);addAction({x+92,y,x+176,y+28},21);y+=36;
 addAction({x,y,x+176,y+28},20);y+=36;addAction({x,y,x+176,y+28},8);y+=36;addAction({x,y,x+176,y+28},9);
 int dx=(W-670)/2,dy=H-116;for(int i=0;i<CategoryCount;i++)addAction({dx+10+i*(650/CategoryCount),dy+10,dx+10+i*(650/CategoryCount)+(650/CategoryCount)-8,dy+37},100+i);
 int count=0;while(count<CategorySlots&&catTypes[category][count]>=0)count++;int width=std::min(96,648/std::max(1,count));
 for(int i=0;i<count;i++){int t=catTypes[category][i],xx=dx+10+i*width;addAction({xx,dy+45,xx+width-5,dy+96},200+t);}
}}
void paintUI(HDC dc){char b[256];
auto panel=[&](RECT r){fill(dc,r,paper);HBRUSH br=CreateSolidBrush(ink);FrameRect(dc,&r,br);InflateRect(&r,-1,-1);FrameRect(dc,&r,br);DeleteObject(br);};
auto btn=[&](int x,int y,int w,int h,const char* label,bool on=false){RECT r{x,y,x+w,y+h};fill(dc,r,on?accent:paper);HBRUSH br=CreateSolidBrush(on?ink:line);FrameRect(dc,&r,br);InflateRect(&r,-1,-1);FrameRect(dc,&r,br);DeleteObject(br);InflateRect(&r,1,1);SelectObject(dc,font);SetTextColor(dc,on?RGB(255,255,255):ink);SetBkMode(dc,TRANSPARENT);DrawTextA(dc,label,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);};
if(hideUI)return;
panel({10,10,294,168});text(dc,22,16,"Creating Constructs",fontTitle,ink);
RECT hint{22,48,282,138};SelectObject(dc,font);SetTextColor(dc,muted);SetBkMode(dc,TRANSPARENT);
const char* h=walk?"Click to capture the mouse. WASD move, Shift run, Space jump, C crouch, Q/E turn, wheel for field of view. Tab returns to building.":"Click builds, hold and drag to fill, Shift+click removes. Keys 1-9 pick a piece, comma and period switch tabs. Lights, ivy and ladders auto-face walls, and Cable needs two clicks on two faces. R rotates. Right-drag orbits, middle-drag pans, wheel zooms, Ctrl+Z undoes.";
DrawTextA(dc,h,-1,&hint,DT_WORDBREAK);sprintf_s(b,"%d pieces",pieces);text(dc,22,146,b,font,muted);
int x=W-210;int sideH=walk?52:std::min(600,std::max(200,H-24));panel({x,10,x+200,10+sideH});x+=12;int y=21;
btn(x,y,176,28,walk?"Back to building (Tab)":"Walk inside (Tab)",walk);if(!walk){y+=36;
btn(x,y,84,28,"Undo");btn(x+92,y,84,28,"Night",night);y+=36;
auto slider=[&](const char* label,float value){text(dc,x,y,label,font,ink);y+=20;RECT track{x+3,y,x+173,y+5};fill(dc,track,RGB(60,59,64));RECT progress=track;progress.right=x+3+(int)(170*value);fill(dc,progress,accent);HBRUSH br=CreateSolidBrush(accent);HGDIOBJ old=SelectObject(dc,br);HPEN pen=CreatePen(PS_SOLID,1,accent);HGDIOBJ op=SelectObject(dc,pen);int cx=x+3+(int)(170*value);Ellipse(dc,cx-6,y-4,cx+7,y+9);SelectObject(dc,old);SelectObject(dc,op);DeleteObject(br);DeleteObject(pen);y+=22;};
slider("Sun height",(sunHeight-4)/86);slider("Sun angle",sunAngle/360);
text(dc,x,y,"Mood",font,ink);y+=18;const char* moods[]={"Overcast","Noon","Golden hour","Blue hour","Concrete overcast"};btn(x,y,176,28,moods[mood]);y+=34;
slider("Exposure",(exposure-.5f)/1.3f);slider("Fog",fog);
btn(x,y,84,28,"Rain",rain);btn(x+92,y,84,28,"AO",ao);y+=36;
sprintf_s(b,"Plot size: %d",world.n);btn(x,y,176,28,b);y+=36;
btn(x,y,84,28,"Share");btn(x+92,y,84,28,"Load");y+=36;
btn(x,y,176,28,"Hide UI (H)",hideUI);y+=36;
btn(x,y,176,28,erasing?"Erase (hold X): on":"Erase (hold X): off",erasing);y+=36;
btn(x,y,176,28,"Clear plot");
}
if(!walk){int dx=(W-670)/2,dy=H-116;panel({dx,dy,dx+670,dy+106});for(int i=0;i<CategoryCount;i++)btn(dx+10+i*(650/CategoryCount),dy+10,(650/CategoryCount)-8,27,cats[i],category==i);
int count=0;while(count<CategorySlots&&catTypes[category][count]>=0)count++;int width=std::min(96,648/std::max(1,count));for(int i=0;i<count;i++){int t=catTypes[category][i],xx=dx+10+i*width;RECT r{xx,dy+45,xx+width-5,dy+96};bool on=t==selected;fill(dc,r,on?accent:RGB(45,45,48));HBRUSH br=CreateSolidBrush(on?ink:line);FrameRect(dc,&r,br);DeleteObject(br);int iw=t==2?10:25,ih=t==1?7:17;RECT sw{xx+(width-iw-5)/2,dy+51,xx+(width+iw-5)/2,dy+51+ih};COLORREF col=t==3?RGB(156,196,207):t==5||t==14||t==15||t==17||t==41||t==42||t==65||t==66||t==67?RGB(127,160,90):t==63||t==64?RGB(118,114,108):t==13||t==26||t==27||t==37?RGB(255,224,160):t==18?RGB(138,74,58):t==19||t==35||t==39?RGB(138,90,50):t==20||t==43||t==46?RGB(143,152,162):t==44?RGB(197,40,40):t==45?RGB(200,150,62):t==16?RGB(34,34,34):t==24||t==40?RGB(43,106,120):RGB(180,178,170);fill(dc,sw,col);br=CreateSolidBrush(ink);FrameRect(dc,&sw,br);DeleteObject(br);sprintf_s(b,"%d %s",(i+1)%10,names[t]);RECT label{xx,dy+74,xx+width-5,dy+94};SelectObject(dc,font);SetTextColor(dc,on?RGB(255,255,255):ink);DrawTextA(dc,b,-1,&label,DT_CENTER|DT_SINGLELINE);}}
}
