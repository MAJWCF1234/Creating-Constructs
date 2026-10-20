// Match HTML dark theme (--bg #17171a, --panel #232326, --ink #e9e8e3, --acc #e8501c)
COLORREF ink=RGB(233,232,227),paper=RGB(35,35,38),accent=RGB(232,80,28),line=RGB(85,84,90),muted=RGB(180,179,174),bgCol=RGB(23,23,26);
void fill(HDC dc,RECT r,COLORREF c){HBRUSH b=CreateSolidBrush(c);FillRect(dc,&r,b);DeleteObject(b);}
void text(HDC dc,int x,int y,const char* s,HFONT ft,COLORREF c){SelectObject(dc,ft);SetTextColor(dc,c);SetBkMode(dc,TRANSPARENT);TextOutA(dc,x,y,s,(int)strlen(s));}
void uiButton(HDC dc,int x,int y,int w,int h,const char* label,int action,bool on=false){RECT r{x,y,x+w,y+h};fill(dc,r,on?accent:paper);HBRUSH br=CreateSolidBrush(on?ink:line);FrameRect(dc,&r,br);InflateRect(&r,-1,-1);FrameRect(dc,&r,br);DeleteObject(br);InflateRect(&r,1,1);SelectObject(dc,font);SetTextColor(dc,on?RGB(255,255,255):ink);SetBkMode(dc,TRANSPARENT);DrawTextA(dc,label,-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);actions.push_back({r,action});}
#include "glass_panels.inc"
void invalidate(){rebuildActions();if(topPanel)InvalidateRect(topPanel,nullptr,FALSE);if(sidePanel)InvalidateRect(sidePanel,nullptr,FALSE);if(dockPanel)InvalidateRect(dockPanel,nullptr,FALSE);if(modalPanel)InvalidateRect(modalPanel,nullptr,FALSE);if(uiModal==3&&saveNameEdit)SetFocus(saveNameEdit);else if(uiModal==2&&shareEdit)SetFocus(shareEdit);else if(viewWindow)SetFocus(viewWindow);}
int typeCount(){int n=0;while(n<CategorySlots&&catTypes[category][n]>=0)n++;return n;}
void selectType(int t){if(selected!=t)audioEvent(A_click);selected=t;for(int i=0;i<typeCount();i++)if(catTypes[category][i]==t){invalidate();return;}for(int c=0;c<CategoryCount;c++)for(int i=0;i<CategorySlots&&catTypes[c][i]>=0;i++)if(catTypes[c][i]==t){category=c;invalidate();return;}}
void undo(){if(history.empty())return;world=history.back();history.pop_back();audioEvent(A_back);dirty=true;save();}
void raisePanels(){if(viewWindow)SetWindowPos(viewWindow,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);for(HWND h:{topPanel,sidePanel,dockPanel,modalPanel})if(h)SetWindowPos(h,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);}
void roundWindow(HWND h,int w,int height){if(h)SetWindowRgn(h,CreateRoundRectRgn(0,0,w+1,height+1,24,24),TRUE);}
void layout(){int oldW=vw,oldH=vh;vw=std::max(1,W);vh=std::max(1,H);if(viewWindow)MoveWindow(viewWindow,0,0,vw,vh,TRUE);
 int topH=walk?80:202,sideH=std::min(H-24,uiSideHeight()),dockW=uiDockWidth();
 if(topPanel){MoveWindow(topPanel,10,10,284,topH,TRUE);roundWindow(topPanel,284,topH);ShowWindow(topPanel,hideUI?SW_HIDE:SW_SHOW);}
 if(sidePanel){MoveWindow(sidePanel,W-222,10,212,sideH,TRUE);roundWindow(sidePanel,212,sideH);ShowWindow(sidePanel,hideUI?SW_HIDE:SW_SHOW);}
 if(dockPanel){MoveWindow(dockPanel,(W-dockW)/2,H-uiDockHeight()-12,dockW,uiDockHeight(),TRUE);roundWindow(dockPanel,dockW,uiDockHeight());ShowWindow(dockPanel,hideUI||walk?SW_HIDE:SW_SHOW);}
 if(modalPanel){int width=uiModal==2?560:620,height=uiModal==1?510:uiModal==3?560:330;MoveWindow(modalPanel,(W-width)/2,(H-height)/2,width,height,TRUE);roundWindow(modalPanel,width,height);ShowWindow(modalPanel,uiModal&&!hideUI?SW_SHOW:SW_HIDE);if(shareEdit){MoveWindow(shareEdit,18,82,width-36,183,TRUE);ShowWindow(shareEdit,uiModal==2&&!hideUI?SW_SHOW:SW_HIDE);}if(saveNameEdit){MoveWindow(saveNameEdit,18,73,width-36,26,TRUE);ShowWindow(saveNameEdit,uiModal==3&&!hideUI?SW_SHOW:SW_HIDE);}}
 raisePanels();rebuildActions();rebuildSwap=rebuildSwap||oldW!=vw||oldH!=vh;
}
std::string exportWorldJson(){std::string j="{\"N\":"+std::to_string(world.n)+",\"c\":[";bool first=true;for(int x=0;x<MaxPlot;x++)for(int z=0;z<MaxPlot;z++)for(int y=0;y<20;y++){auto c=get(x,y,z);if(c.t<0)continue;int k=x*1024+z*32+y;if(!first)j+=',';first=false;j+="["+std::to_string(k)+",{\"t\":"+std::to_string(c.t)+",\"r\":"+std::to_string(c.r)+"}]";}
 j+="],\"cab\":[";first=true;for(auto& c:world.cables){if(!first)j+=',';first=false;char b[128];sprintf_s(b,"[%.4f,%.4f,%.4f,%.4f,%.4f,%.4f]",c.a.x,c.a.y,c.a.z,c.b.x,c.b.y,c.b.z);j+=b;}j+="]}";return j;}
void showUiModal(int mode){uiModal=mode;layout();if(modalPanel)InvalidateRect(modalPanel,nullptr,FALSE);if(mode==2&&shareEdit)SetFocus(shareEdit);else if(mode==3&&saveNameEdit)SetFocus(saveNameEdit);else if(viewWindow)SetFocus(viewWindow);}
void openShare(){showUiModal(2);std::string json=exportWorldJson();SetWindowTextA(shareEdit,json.c_str());SendMessage(shareEdit,EM_SETSEL,0,-1);SetFocus(shareEdit);}
bool loadWorldJson(const char* text){const char* p=text;auto skip=[&]{while(*p==' '||*p=='\n'||*p=='\r'||*p=='\t')p++;};auto num=[&]()->double{skip();char* e;double v=strtod(p,&e);if(e==p)return NAN;p=e;return v;};
 skip();if(*p!='{')return false;World w;w.c.fill(Cell{});w.n=10;
 while(*p&&*p!='}'){if(*p==','||*p=='{'){p++;continue;}skip();if(*p=='"'){p++;const char* k=p;while(*p&&*p!='"')p++;std::string key(k,p-k);if(*p=='"')p++;skip();if(*p==':')p++;skip();
  if(key=="N"){double v=num();if(!std::isfinite(v)||v<4||v>MaxPlot)return false;w.n=(int)v;}
  else if(key=="c"){skip();if(*p!='[')return false;p++;while(*p&&*p!=']'){skip();if(*p==','){p++;continue;}if(*p!='[')return false;p++;double keyv=num();skip();if(*p==',')p++;skip();int t=-1,r=0;
   if(*p=='{'){p++;while(*p&&*p!='}'){skip();if(*p==','){p++;continue;}if(*p=='"'){p++;const char* fk=p;while(*p&&*p!='"')p++;std::string f(fk,p-fk);if(*p=='"')p++;skip();if(*p==':')p++;double v=num();if(f=="t")t=(int)v;if(f=="r")r=(int)v;}else p++;}if(*p=='}')p++;}
   skip();if(*p==']')p++;int ik=(int)keyv,x=ik>>10,z=(ik>>5)&31,y=ik&31;if(x>=0&&x<MaxPlot&&z>=0&&z<MaxPlot&&y>=0&&y<20&&t>=0&&t<PieceCount&&r>=0&&r<4)w.c[idx(x,y,z)]={(int8_t)t,(int8_t)r};}
   skip();if(*p==']')p++;}
  else if(key=="cab"){skip();if(*p!='[')return false;p++;while(*p&&*p!=']'){skip();if(*p==','){p++;continue;}if(*p!='[')return false;p++;float v[6];for(int i=0;i<6;i++){v[i]=(float)num();skip();if(*p==','&&i<5)p++;}skip();if(*p==']')p++;if(std::isfinite(v[0])&&std::isfinite(v[5]))w.cables.push_back({{v[0],v[1],v[2]},{v[3],v[4],v[5]}});}skip();if(*p==']')p++;}
  else{while(*p&&*p!=','&&*p!='}'){if(*p=='['||*p=='{'){int d=0;do{if(*p=='['||*p=='{')d++;else if(*p==']'||*p=='}')d--;p++;}while(*p&&d>0);}else p++;}}
 }else p++;}
 remember();world=w;target={world.n/2.f,2,world.n/2.f};dirty=true;save();return true;}
void loadFromClipboard(){if(!OpenClipboard(mainWindow)){MessageBoxA(mainWindow,"Could not read clipboard.","Load world",MB_ICONWARNING);return;}HANDLE h=GetClipboardData(CF_TEXT);bool ok=false;if(h){const char* t=(const char*)GlobalLock(h);if(t)ok=loadWorldJson(t);if(t)GlobalUnlock(h);}CloseClipboard();
 MessageBoxA(mainWindow,ok?"World loaded from clipboard.":"Could not read that world. Copy the full exported JSON first.","Load world",ok?MB_OK:MB_ICONWARNING);if(ok)invalidate();}
void action(int a){if(a>=400&&a<405){saveSelected=savePage*5+a-400;invalidate();return;}if(a!=2&&a<200)audioEvent(a==22?A_back:A_click);if(a>=200){selectType(a-200);return;}if(a>=100){category=a-100;selected=catTypes[category][0];dockScrollX=0;layout();invalidate();return;}switch(a){case 1:toggleWalk();layout();break;case 2:undo();break;case 3:night=!night;break;case 4:{mood=(mood+1)%5;const float el[]={40,78,11,4,26},az[]={130,120,60,250,150},fg[]={.38f,.10f,.28f,.34f,.52f},sat[]={0,.75f,.8f,.55f,0};sunHeight=el[mood];sunAngle=az[mood];fog=fg[mood];moodSat=sat[mood];break;}case 5:rain=!rain;break;case 6:ao=!ao;break;
case 7:{const int sizes[]={4,6,10,12,16,24,32};int next=0;for(int i=0;i<7;i++)if(world.n==sizes[i])next=(i+1)%7;remember();world.n=sizes[next];for(int x=0;x<MaxPlot;x++)for(int z=0;z<MaxPlot;z++)if(x>=world.n||z>=world.n)for(int y=0;y<20;y++)world.c[idx(x,y,z)]=Cell{};target={world.n/2.f,2,world.n/2.f};dirty=true;save();break;}
case 8:erasing=!erasing;break;case 9:remember();world.c.fill(Cell{});world.cables.clear();world.pipes.clear();dirty=true;save();break;case 18:remember();starter();target={5,2,5};dirty=true;save();break;
case 19:openShare();break;case 20:hideUI=!hideUI;layout();break;case 21:openShare();break;
case 22:showUiModal(0);break;
case 23:{int n=GetWindowTextLengthA(shareEdit);std::string data((size_t)n+1,'\0');GetWindowTextA(shareEdit,data.data(),n+1);if(loadWorldJson(data.c_str()))showUiModal(0);else MessageBoxA(mainWindow,"Could not read that world. Paste the full exported text.","Load world",MB_ICONWARNING);break;}
case 30:refreshNamedSaves();savesStatus="Saved copies stay here between sessions.";showUiModal(3);break;
case 31:{wchar_t title[64]{};GetWindowTextW(saveNameEdit,title,64);if(saveCurrentNamed(title))audioEvent(A_click);SetFocus(saveNameEdit);break;}
case 32:if(loadSelectedNamed()){audioEvent(A_back);showUiModal(0);layout();}break;
case 33:savePage=std::max(0,savePage-1);if(!saveEntries.empty())saveSelected=savePage*5;break;
case 34:savePage=std::min(std::max(0,((int)saveEntries.size()-1)/5),savePage+1);if(!saveEntries.empty())saveSelected=savePage*5;break;
case 24:showUiModal(1);break;case 26:atmosphereExpanded=!atmosphereExpanded;layout();break;case 27:worldExpanded=!worldExpanded;layout();break;case 28:audioExpanded=!audioExpanded;if(audioExpanded){atmosphereExpanded=false;worldExpanded=false;}layout();break;case 29:audioMuted=!audioMuted;audioSaveSettings();break;}invalidate();}
void keyDown(int k,bool repeat){if(benchmarkRun)return;keys[k]=true;if(repeat)return;if(k==VK_TAB){action(1);fieldOfView=walk?75.f:60.f;}if(k=='S'&&keys[VK_CONTROL]){action(30);return;}if(k=='M'){action(29);return;}if(k==VK_OEM_2){showUiModal(uiModal==1?0:1);return;}if(uiModal&&k!=VK_ESCAPE&&k!='H')return;if(k==VK_ESCAPE){if(uiModal){showUiModal(0);return;}pendingCable=false;ReleaseCapture();button=-1;}if(k=='Z'&&keys[VK_CONTROL]){undo();invalidate();}if(k=='H'){hideUI=!hideUI;layout();invalidate();}if(k==VK_F5)save();if(k==VK_F9){remember();load();target={world.n/2.f,2,world.n/2.f};dirty=true;}
if(!walk){if(k=='R'){audioEvent(A_click,.65f);rotation=(rotation+1)%4;if(selected==44||selected==45)pipeManual=true;invalidate();}if(k=='A'&&(selected==44||selected==45)){pipeManual=false;audioEvent(A_click,.5f);invalidate();}if(k==VK_OEM_COMMA||k==VK_OEM_PERIOD){audioEvent(A_click);category=(category+(k==VK_OEM_COMMA?CategoryCount-1:1))%CategoryCount;selected=catTypes[category][0];dockScrollX=0;invalidate();}if(k==VK_OEM_4)selectType((selected+PieceCount-1)%PieceCount);if(k==VK_OEM_6)selectType((selected+1)%PieceCount);if(k>='0'&&k<='9'){int i=(k-'0'+9)%10;if(i<typeCount())selectType(catTypes[category][i]);}if(k=='W')target=target+V{-sinf(theta),0,-cosf(theta)};if(k=='S')target=target+V{sinf(theta),0,cosf(theta)};if(k=='Q')theta+=.15f;if(k=='E')theta-=.15f;}}
void setUiSlider(int id,float t){t=std::clamp(t,0.f,1.f);if(id==300)sunHeight=4+t*86;if(id==301)sunAngle=t*360;if(id==302)exposure=.5f+t*1.3f;if(id==303)fog=t;if(id==304)musicVolume=t;if(id==305)ambienceVolume=t;if(id==306)effectsVolume=t;audioEvent(A_slider,.65f);invalidate();}
void chooseUiValue(int id,HWND hwnd,POINT point){HMENU menu=CreatePopupMenu();const char* moods[]={"Overcast","Noon","Golden hour","Blue hour","Concrete overcast"};const int sizes[]={4,6,10,12,16,24,32};char label[16];int count=id==4?5:7;for(int i=0;i<count;i++){if(id==4)AppendMenuA(menu,MF_STRING|(mood==i?MF_CHECKED:0),i+1,moods[i]);else{sprintf_s(label,"%d",sizes[i]);AppendMenuA(menu,MF_STRING|(world.n==sizes[i]?MF_CHECKED:0),i+1,label);}}ClientToScreen(hwnd,&point);int choice=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_LEFTALIGN,point.x,point.y,0,hwnd,nullptr);DestroyMenu(menu);if(!choice)return;audioEvent(A_click);if(id==4){int desired=choice-1;while(mood!=desired)action(4);}else{remember();world.n=sizes[choice-1];for(int x=0;x<MaxPlot;x++)for(int z=0;z<MaxPlot;z++)if(x>=world.n||z>=world.n)for(int y=0;y<20;y++)world.c[idx(x,y,z)]=Cell{};target={world.n/2.f,2,world.n/2.f};dirty=true;save();}invalidate();}
LRESULT CALLBACK wndProc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){bool child=(GetWindowLongPtr(hwnd,GWL_STYLE)&WS_CHILD)!=0,view=GetDlgCtrlID(hwnd)==1;switch(msg){case WM_ERASEBKGND:return 1;
case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(hwnd,&ps);if(child&&!view){POINT pt{0,0};MapWindowPoints(hwnd,mainWindow,&pt,1);SetViewportOrgEx(dc,-pt.x,-pt.y,nullptr);double paintStart=benchmarkRun?perfClock():0;paintUI(dc);if(benchmarkRun)perfPaintMs+=perfClock()-paintStart;}EndPaint(hwnd,&ps);return 0;}
case WM_CTLCOLOREDIT:{HDC dc=(HDC)wp;SetTextColor(dc,ink);SetBkColor(dc,RGB(28,28,31));static HBRUSH editBrush=CreateSolidBrush(RGB(28,28,31));return (LRESULT)editBrush;}
case WM_SIZE:if(!child){W=LOWORD(lp);H=HIWORD(lp);layout();}return 0;
case WM_GETMINMAXINFO:((MINMAXINFO*)lp)->ptMinTrackSize={1000,740};return 0;
case WM_KEYDOWN:case WM_SYSKEYDOWN:if(wp==VK_F4&&(GetKeyState(VK_MENU)&0x8000))return DefWindowProc(hwnd,msg,wp,lp);if(wp<256)keyDown((int)wp,(lp&(1u<<30))!=0);return 0;
case WM_KEYUP:case WM_SYSKEYUP:if(wp<256)keys[wp]=false;return 0;
case WM_ACTIVATEAPP:audio.focus.store(wp&&!audioMuted?.8f:0.f);return 0;
case WM_KILLFOCUS:std::fill(std::begin(keys),std::end(keys),false);button=-1;drag=false;pipeEndStroke();ReleaseCapture();return 0;
case WM_LBUTTONDOWN:case WM_RBUTTONDOWN:case WM_MBUTTONDOWN:{if(benchmarkRun)return 0;int x=(short)LOWORD(lp),y=(short)HIWORD(lp);if(!view){POINT pt{x,y};MapWindowPoints(hwnd,mainWindow,&pt,1);if(msg==WM_LBUTTONDOWN){if(actions.empty())rebuildActions();for(auto a:actions)if(PtInRect(&a.rect,pt)){if(a.action>=300&&a.action<=306){sliderActive=a.action;sliderBounds=a.rect;SetCapture(hwnd);setUiSlider(sliderActive,(pt.x-a.rect.left)/(float)(a.rect.right-a.rect.left));}else if(a.action==4||a.action==7)chooseUiValue(a.action,hwnd,{x,y});else action(a.action);return 0;}}return 0;}if(uiModal)return 0;SetFocus(hwnd);SetCapture(hwnd);button=msg==WM_LBUTTONDOWN?0:msg==WM_RBUTTONDOWN?2:1;mouseX=lastX=x;mouseY=lastY=y;if(!walk&&button==0){remember();std::fill(std::begin(placed),std::end(placed),false);drag=true;pick();edit();}return 0;}
case WM_LBUTTONUP:case WM_RBUTTONUP:case WM_MBUTTONUP:if(sliderActive>=304)audioSaveSettings();sliderActive=-1;button=-1;drag=false;pipeEndStroke();ReleaseCapture();return 0;
case WM_CAPTURECHANGED:sliderActive=-1;button=-1;drag=false;pipeEndStroke();return 0;
case WM_MOUSELEAVE:if(view)mouseInView=false;return 0;
case WM_MOUSEMOVE:if(benchmarkRun)return 0;if(!view){POINT p{(short)LOWORD(lp),(short)HIWORD(lp)};MapWindowPoints(hwnd,mainWindow,&p,1);if(sliderActive>=300)setUiSlider(sliderActive,(p.x-sliderBounds.left)/(float)(sliderBounds.right-sliderBounds.left));else{int hovered=-1;for(auto a:actions)if(PtInRect(&a.rect,p)){hovered=a.action;break;}if(hovered!=hoveredAction){if(hovered>=0&&hovered<300)audioEvent(A_hover,.5f);hoveredAction=hovered;InvalidateRect(hwnd,nullptr,FALSE);}}return 0;}if(uiModal)return 0;if(view){mouseInView=true;TRACKMOUSEEVENT track{sizeof(TRACKMOUSEEVENT),TME_LEAVE,hwnd,0};TrackMouseEvent(&track);int x=(short)LOWORD(lp),y=(short)HIWORD(lp),dx=x-lastX,dy=y-lastY;mouseX=x;mouseY=y;lastX=x;lastY=y;if(walk&&button==2){yaw-=dx*.003f;pitch=std::clamp(pitch-dy*.003f,-1.5f,1.5f);}else if(!walk){if(button==2){theta-=dx*.006f;phi=std::clamp(phi-dy*.006f,.15f,1.5f);}if(button==1){target=target+rightv*(-dx*.025f)+V{0,dy*.025f,0};target.y=std::max(0.f,target.y);}if(drag&&selected!=16){pick();edit();}}}return 0;
case WM_MOUSEWHEEL:{int delta=GET_WHEEL_DELTA_WPARAM(wp)>0?1:-1;POINT cursor;GetCursorPos(&cursor);auto overPanel=[&](HWND panel){if(!panel||!IsWindowVisible(panel))return false;RECT r;GetWindowRect(panel,&r);return PtInRect(&r,cursor)!=0;};
 if(walk)fieldOfView=std::clamp(fieldOfView-delta*3.f,45.f,110.f);
 else if(overPanel(dockPanel)&&uiDockMaxScroll()>0){dockScrollX=std::clamp(dockScrollX-delta*48,0,uiDockMaxScroll());invalidate();}
 else if(button==0||drag||overPanel(dockPanel)){int n=typeCount();if(n>0){int idx=0;for(int i=0;i<n;i++)if(catTypes[category][i]==selected){idx=i;break;}selectType(catTypes[category][(idx+(delta>0?1:n-1)+n)%n]);}}
 else if(overPanel(sidePanel)||overPanel(topPanel)||overPanel(modalPanel))return 0;
 else distanceCam=std::clamp(distanceCam*(delta>0?.92f:1.08f),4.f,70.f);return 0;}
case WM_CLOSE:DestroyWindow(mainWindow);return 0;case WM_DESTROY:if(!child)PostQuitMessage(0);return 0;}return DefWindowProc(hwnd,msg,wp,lp);}
void destroyPostPass(){if(postPipeline)vkDestroyPipeline(device,postPipeline,nullptr);if(postPipeLayout)vkDestroyPipelineLayout(device,postPipeLayout,nullptr);if(postPool)vkDestroyDescriptorPool(device,postPool,nullptr);if(postDescLayout)vkDestroyDescriptorSetLayout(device,postDescLayout,nullptr);if(sceneSampler)vkDestroySampler(device,sceneSampler,nullptr);if(postMap){vkUnmapMemory(device,postMem);postMap=0;}if(postBuf)vkDestroyBuffer(device,postBuf,nullptr);if(postMem)vkFreeMemory(device,postMem,nullptr);if(postPass)vkDestroyRenderPass(device,postPass,nullptr);if(scenePass)vkDestroyRenderPass(device,scenePass,nullptr);postPipeline=0;postPass=0;scenePass=0;renderPass=0;}
void cleanup(){vkDeviceWaitIdle(device);destroyUiBackground();if(perfQueries)vkDestroyQueryPool(device,perfQueries,nullptr);for(auto bitmap:pieceThumbs)if(bitmap)DeleteObject(bitmap);destroySwap();destroyPostPass();vkDestroyFence(device,fence,nullptr);vkDestroySemaphore(device,acquired,nullptr);vkDestroySemaphore(device,rendered,nullptr);vkDestroyCommandPool(device,commandPool,nullptr);vkDestroyDescriptorPool(device,descriptorPool,nullptr);vkUnmapMemory(device,foliageMemory);vkDestroyBuffer(device,foliageBuffer,nullptr);vkFreeMemory(device,foliageMemory,nullptr);vkUnmapMemory(device,vertexMemory);vkUnmapMemory(device,stateMemory);vkDestroyBuffer(device,vertexBuffer,nullptr);vkDestroyBuffer(device,stateBuffer,nullptr);vkFreeMemory(device,vertexMemory,nullptr);vkFreeMemory(device,stateMemory,nullptr);if(texSampler)vkDestroySampler(device,texSampler,nullptr);if(texView)vkDestroyImageView(device,texView,nullptr);if(texImage)vkDestroyImage(device,texImage,nullptr);if(texMemory)vkFreeMemory(device,texMemory,nullptr);vkDestroyPipeline(device,pipeline,nullptr);vkDestroyPipeline(device,waterPipeline,nullptr);vkDestroyPipeline(device,skyPipeline,nullptr);vkDestroyPipelineLayout(device,pipelineLayout,nullptr);vkDestroyDescriptorSetLayout(device,descriptorLayout,nullptr);vkDestroyDevice(device,nullptr);vkDestroySurfaceKHR(instance,surface,nullptr);if(debugMessenger){auto fn=(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");if(fn)fn(instance,debugMessenger,nullptr);}vkDestroyInstance(instance,nullptr);}
LRESULT CALLBACK saveNameProc(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_KEYDOWN&&w==VK_RETURN){action(31);return 0;}if(m==WM_KEYDOWN&&w==VK_ESCAPE){showUiModal(0);return 0;}return CallWindowProcW(saveNameOldProc,h,m,w,l);}
void check(bool value,const char* name){log((std::string(value?"PASS: ":"FAIL: ")+name).c_str());if(!value)fail(name);}
#include "audio_tests.inc"
#include "saves_tests.inc"
#include "nature_tests.inc"
#include "world_size_tests.inc"
#include "water_tests.inc"
void selfTest(){starter();World initial=world;int count=0;for(auto c:world.c)if(c.t>=0)count++;check(count==129&&world.cables.empty()&&world.pipes.size()==5,"brutalist starter: 129 pieces, five pipes");save();world=World{};check(load(),"binary save reload");check(world.n==initial.n&&memcmp(world.c.data(),initial.c.data(),sizeof(Cell)*WorldCells)==0&&world.cables.size()==initial.cables.size()&&world.pipes.size()==initial.pipes.size(),"save roundtrip equality");remember();put(0,12,0,23,3);undo();check(memcmp(world.c.data(),initial.c.data(),sizeof(Cell)*WorldCells)==0,"undo restores cells");check(solidAt({2.5,.5,3.5}),"board-form floor collision");check(!solidAt({2.5,1.5,3.5}),"corridor airspace is open");put(1,0,2,-1);put(0,1,2,-1);put(1,1,2,-1);put(0,0,2,23);
 check(!solidAt({1.f,.4f,2.45f})&&solidAt({.12f,.4f,2.45f}),"doorway opening and jamb collision");
 check(!solidAt({1.f,.4f,2.45f})&&solidAt({1.88f,.4f,2.45f}),"doorway 2-wide opening and far jamb");
 check(solidAt({.12f,1.4f,2.45f})&&!solidAt({1.f,1.4f,2.45f}),"doorway upper jamb and opening");
 check(!solidAt({1.f,1.7f,2.45f})&&solidAt({1.f,1.9f,2.45f}),"doorway head clearance under lintel");
 check(freeAt({1.f,0,2.45f},1.6f),"doorway passable at standing height");
 check(solidAt({.12f,.4f,2.15f})&&solidAt({.12f,.4f,2.55f}),"doorway jamb has depth toward interior");
 check(!canPlace(0,1,2,0)&&!canPlace(1,0,2,0),"footprint blocks placing into doorway span");put(0,0,2,4);check(solidAt({.5,.2,2.2})&&!solidAt({.5,.9,2.8}),"slope collision");
 world=World{};put(0,0,0,56);put(0,1,0,-1);check(!solidAt({.5f,.5f,.5f})&&solidAt({.5f,1.8f,.5f}),"pilotis walk-under and capital");
 put(2,0,0,57);put(3,0,0,-1);check(solidAt({2.5f,.2f,.5f})&&solidAt({3.5f,.2f,.5f})&&!solidAt({2.5f,.7f,.5f}),"megastep 2-wide tread");
 put(2,0,2,58);for(int dx=0;dx<2;dx++)for(int dz=0;dz<2;dz++)for(int dy=0;dy<2;dy++)if(dx||dz||dy)put(2+dx,dy,2+dz,-1);
 check(solidAt({2.05f,.5f,2.5f})&&!solidAt({2.5f,.5f,2.5f}),"lightwell rim and open center");
 put(5,0,0,59);put(6,0,0,-1);put(5,1,0,-1);put(6,1,0,-1);
 check(solidAt({5.5f,.05f,.5f})&&!solidAt({5.5f,.8f,.5f})&&freeAt({5.5f,.12f,.5f},1.6f),"bridge tunnel fits standing player");
 put(8,0,0,60);put(8,1,0,-1);
 check(solidAt({8.1f,.5f,.5f})&&!solidAt({8.5f,.5f,.3f})&&freeAt({8.5f,0,.3f},1.6f),"tower core fits standing player");
 world=initial;for(int t=0;t<PieceCount;t++){vertices.clear();transparent.clear();origin={};turn=0;ghostMode=t==15;piece(t);ghostMode=false;check(vertices.size()+transparent.size()>0,(std::string("geometry: ")+names[t]).c_str());}dirty=true;}
void ivySelfTest(){
 World preserved=world;V oldOrigin=origin;int oldTurn=turn;
 for(int rot=0;rot<4;rot++){
  world=World{};turn=rot;V backing=transformNormal({0,0,1});for(int y=0;y<5;y++){put(3,y,3,15,rot);put(3-(int)backing.x,y,3-(int)backing.z,0);}
  vertices.clear();transparent.clear();origin={3.5f,4.5f,3.5f};turn=rot;ivyBeginRefresh();ivyGrowth();
  check(!vertices.empty(),"connected ivy emits geometry from upper edge");
  size_t count=vertices.size();float low=100,high=-100;
  uint64_t signature=1469598103934665603ull;
  bool finite=true;for(auto v:vertices){finite=finite&&std::isfinite(v.p.x)&&std::isfinite(v.p.y)&&std::isfinite(v.p.z);low=std::min(low,v.p.y);high=std::max(high,v.p.y);uint32_t bits;memcpy(&bits,&v.p.x,4);signature=(signature^bits)*1099511628211ull;}
  check(finite,"finite ivy vertices");check(high-low>3.4f,"vine stems continue across multiple cells");
  vertices.clear();ivyBeginRefresh();ivyGrowth();uint64_t again=1469598103934665603ull;for(auto v:vertices){uint32_t bits;memcpy(&bits,&v.p.x,4);again=(again^bits)*1099511628211ull;}
  check(vertices.size()==count&&signature==again,"ivy geometry is stable across refreshes");
  vertices.clear();transparent.clear();origin={3.5f,2.5f,3.5f};ivyGrowth();check(vertices.empty()&&transparent.empty(),"middle cells do not restart horizontal ivy bands");
 }
 // Adjacent columns must be emitted as one component, with shared leaf spacing.
 world=World{};for(int x=3;x<=5;x++)for(int y=0;y<5;y++){put(x,y,3,15,0);put(x,y,2,0);}
 origin={3.5f,4.5f,3.5f};turn=0;vertices.clear();transparent.clear();ivyBeginRefresh();ivyGrowth();
 check(ivyGenerated[idx(5,0,3)],"ivy flood joins horizontal and vertical neighbors");
 check(ivyLeavesAccepted>0&&ivyLeavesSkipped>0,"neighbor ivy shares a leaf spacing budget");
 size_t before=vertices.size();origin={4.5f,3.5f,3.5f};ivyGrowth();check(vertices.size()==before,"neighbor cells do not duplicate patch geometry");
 bool sitesFinite=true;for(const auto& bin:ivyLeafBins)for(V site:bin)sitesFinite=sitesFinite&&std::isfinite(site.x)&&std::isfinite(site.y)&&std::isfinite(site.z);check(sitesFinite,"finite adaptive leaf sites");
 world=World{};put(3,2,3,0);
 for(int rot=0;rot<4;rot++){
  turn=rot;V n=transformNormal({0,0,1});origin=V{3.5f,2.5f,3.5f}+n*2.f;
  V attached=transform(ivyAttached({0,0,-.4f}));float outward=dot(attached-V{3.5f,2.5f,3.5f},n);
  check(outward>.5f&&outward<.67f,"nearby ivy attaches to supporting wall in all rotations");
 }
 ivyBeginRefresh();check(ivyReserveLeaf({3,2,3},.1f),"first nearby leaf occupies its spacing site");check(!ivyReserveLeaf({3.025f,2,3},.1f),"nearby overlapping vine leaf is rejected");
 world=preserved;origin=oldOrigin;turn=oldTurn;vertices.clear();transparent.clear();ivyBeginRefresh();dirty=true;
}
#include "foliage_trace_tests.inc"
#include "parity_tests.inc"
#include "ui_tests.inc"
int WINAPI wWinMain(HINSTANCE hi,HINSTANCE,PWSTR cmd,int){SetProcessDPIAware();std::wstring args=cmd;audioTest=args.find(L"--audio-test")!=std::wstring::npos;verifyGlassBlur=std::wstring(cmd).find(L"--verify-blur")!=std::wstring::npos;benchmarkRun=args.find(L"--benchmark")!=std::wstring::npos;fixedFrame=args.find(L"--fixed-frame")!=std::wstring::npos;benchmarkStorm=args.find(L"--bench-storm")!=std::wstring::npos;benchmarkWalk=args.find(L"--bench-walk")!=std::wstring::npos;inspectVines=args.find(L"--inspect-vines")!=std::wstring::npos;capturePending=args.find(L"--capture")!=std::wstring::npos;if(capturePending){wchar_t t[MAX_PATH];GetTempPathW(MAX_PATH,t);captureFile=std::wstring(t)+L"creating-constructs-vines.bmp";}smoke=args.find(L"--smoke-test")!=std::wstring::npos;validate=args.find(L"--validate")!=std::wstring::npos;wchar_t local[MAX_PATH];GetEnvironmentVariableW(L"LOCALAPPDATA",local,MAX_PATH);std::wstring dir=std::wstring(local)+L"\\CreatingConstructs";CreateDirectoryW(dir.c_str(),nullptr);{std::wstring legacy=std::wstring(local)+L"\\MonolithVulkan";auto copyIfMissing=[&](const wchar_t* name){std::wstring dst=dir+L"\\"+name,src=legacy+L"\\"+name;if(GetFileAttributesW(dst.c_str())==INVALID_FILE_ATTRIBUTES&&GetFileAttributesW(src.c_str())!=INVALID_FILE_ATTRIBUTES)CopyFileW(src.c_str(),dst.c_str(),TRUE);};if(!smoke){copyIfMissing(L"world.bin");copyIfMissing(L"audio.cfg");}}dataPath=dir+(smoke?L"\\test-world.bin":L"\\world.bin");logPath=dir+(smoke?L"\\smoke.log":L"\\runtime.log");_wfopen_s(&logFile,logPath.c_str(),L"wb");log("Creating Constructs · native C++ / Vulkan");audioLoadSettings(dir);if(audioTest||(!smoke&&!benchmarkRun&&captureFile.empty())){if(!audioLoadBank())log("Audio bank could not be decoded");}if(args.find(L"--export-soundtracks")!=std::wstring::npos){exportMoodSoundtracks();if(logFile)fclose(logFile);return 0;}if(audioTest)audioSelfTest();if(!load())starter();savesDirectory=dir+(smoke?L"\\TestSaves":L"\\Saves");CreateDirectoryW(savesDirectory.c_str(),nullptr);if(!smoke){std::wstring legacySaves=std::wstring(local)+L"\\MonolithVulkan\\Saves";WIN32_FIND_DATAW fd;HANDLE h=FindFirstFileW((savesDirectory+L"\\*").c_str(),&fd);int own=0;if(h!=INVALID_HANDLE_VALUE){do{if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))own++;}while(FindNextFileW(h,&fd));FindClose(h);}if(!own&&GetFileAttributesW(legacySaves.c_str())!=INVALID_FILE_ATTRIBUTES){h=FindFirstFileW((legacySaves+L"\\*").c_str(),&fd);if(h!=INVALID_HANDLE_VALUE){do{if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;CopyFileW((legacySaves+L"\\"+fd.cFileName).c_str(),(savesDirectory+L"\\"+fd.cFileName).c_str(),TRUE);}while(FindNextFileW(h,&fd));FindClose(h);}} }refreshNamedSaves();if(!smoke&&!audioTest&&!benchmarkRun&&captureFile.empty()&&saveEntries.empty())saveCurrentNamed(L"My first world");if(smoke){selfTest();ivySelfTest();catalogSelfTest();uiSelfTest();savesSelfTest();natureSelfTest();worldSizeSelfTest();waterSelfTest();}target={world.n/2.f,2,world.n/2.f};
if(inspectVines){target={7.6f,2.7f,1.1f};theta=.65f;phi=1.25f;distanceCam=6.3f;}
if(args.find(L"--inspect-tower")!=std::wstring::npos){target={3.5f,6.0f,3.9f};theta=.78f;phi=1.12f;distanceCam=6.2f;}
if(!captureFile.empty()&&args.find(L"--catalog-review")!=std::wstring::npos){world=World{};world.n=16;for(int t=28;t<PieceCount;t++){int i=t-28;put(2+(i%5)*2,0,2+(i/5)*3,t,0);}target={6.5f,.5f,5};theta=.65f;phi=.85f;distanceCam=17;dirty=true;}
if(!captureFile.empty()&&args.find(L"--light-review")!=std::wstring::npos){
 world=World{};world.n=16;
 for(int x=1;x<=10;x++)for(int z=2;z<=7;z++)put(x,0,z,0);
 int types[4]={13,26,27,37};
 for(int i=0;i<4;i++){int x=2+i*2;for(int y=1;y<=3;y++)put(x,y,3,0);put(x,1,4,types[i],0);}
 target={5.5f,1.6f,4.2f};theta=.3f;phi=1.02f;distanceCam=11;dirty=true;
}
if(!captureFile.empty()&&args.find(L"--nature-review")!=std::wstring::npos){
 world=World{};world.n=12;
 for(int x=0;x<10;x++)for(int z=0;z<9;z++)put(x,0,z,(x<4||x>7)?5:0);
 put(1,1,2,14);put(8,1,6,14);put(2,1,7,14);
 for(int x=5;x<9;x++)for(int y=1;y<7;y++){put(x,y,2,0);if(y<5&&x%2)put(x,y,3,15);}
 target={4.5f,2.5f,4.3f};theta=.65f;phi=1.05f;distanceCam=15;dirty=true;
 if(args.find(L"--nature-sun")!=std::wstring::npos){mood=1;sunHeight=50;sunAngle=135;}
 if(args.find(L"--nature-close")!=std::wstring::npos){target={2.4f,1.8f,3.7f};theta=.8f;phi=1.37f;distanceCam=6;}
}
if(args.find(L"--night-review")!=std::wstring::npos)night=true;
if(benchmarkStorm){night=true;rain=true;}if(args.find(L"--view-close")!=std::wstring::npos){target={3.5f,6,3.9f};distanceCam=6.2f;theta=.78f;phi=1.12f;}if(args.find(L"--view-side")!=std::wstring::npos){theta=2.1f;phi=1.3f;}if(args.find(L"--view-wide")!=std::wstring::npos){distanceCam=33;theta=4.1f;phi=.8f;}
if(!captureFile.empty()){
 size_t argument=args.find(L"--capture-save=");if(argument!=std::wstring::npos){
  size_t start=argument+15,end;if(start<args.size()&&args[start]=='"'){start++;end=args.find(L'"',start);}else end=args.find(L' ',start);
  std::wstring path=args.substr(start,end==std::wstring::npos?end:end-start),title;World snapshot;SavedView view{};
  if(!readNamedSave(path,snapshot,view,title))fail("Could not read the requested capture save");
  world=std::move(snapshot);target=view.target;theta=view.theta;phi=view.phi;distanceCam=view.distance;player=view.player;namedWalkSpawn=view.player;hasNamedWalkSpawn=true;yaw=view.yaw;pitch=view.pitch;fieldOfView=view.fov;sunHeight=view.sunHeight;sunAngle=view.sunAngle;exposure=view.exposure;fog=view.fog;moodSat=view.sat;mood=view.mood;night=view.flags&1;rain=view.flags&2;ao=view.flags&4;dirty=true;
  if(args.find(L"--verify-construct")!=std::wstring::npos){
   auto route=[&](V start,float facing,int count){player=start;yaw=facing;pitch=0;velY=hx=hz=0;grounded=true;spaceHeld=false;keys['W']=true;for(int i=0;i<count;i++)playerStep(1.f/120);keys['W']=false;char routeLog[160];sprintf_s(routeLog,"Route end %.3f %.3f %.3f / velocity %.3f",player.x,player.y,player.z,velY);log(routeLog);};
   check(freeAt({15.4f,4,17.4f},1.6f),"showcase walking spawn has full standing clearance");
   route({17.5f,4,12.5f},0,360);check(player.y>7.8f&&player.z<4.5f,"broad terrace stairs climb from the courtyard to the roof");
   route({16.5f,4,14.3f},0,330);check(player.y<1.2f&&player.z<8,"basement stairs descend into a standing-height tunnel");
   route({16.5f,1,7.5f},PI,330);check(player.y>3.8f&&player.z>13.5f,"basement stairs return to the courtyard without jumping");
   route({10.5f,4,6.8f},0,80);check(player.z<4.5f&&player.y>3.8f,"test block doorway is traversable at full height");
   check(freeAt({20.5f,1,3.5f},1.6f)&&freeAt({16.5f,1,5.5f},1.6f),"basement chamber and tunnel have standing clearance");
   player=view.player;yaw=view.yaw;pitch=view.pitch;velY=hx=hz=0;grounded=true;
  }
  if(args.find(L"--capture-walk")!=std::wstring::npos)walk=true;
 }
}
bgBrush=CreateSolidBrush(bgCol);font=CreateFontA(-13,0,0,0,400,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Arial");fontBold=CreateFontA(-13,0,0,0,700,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Arial");fontTitle=CreateFontA(-18,0,0,0,800,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,"Arial Narrow");
WNDCLASSW wc{};wc.lpfnWndProc=wndProc;wc.hInstance=hi;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.lpszClassName=L"CreatingConstructsMain";wc.hbrBackground=bgBrush;RegisterClassW(&wc);wc.lpszClassName=L"CreatingConstructsView";wc.hbrBackground=nullptr;RegisterClassW(&wc);RECT r{0,0,1360,900};AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,FALSE);mainWindow=CreateWindowW(L"CreatingConstructsMain",L"Creating Constructs",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,20,20,r.right-r.left,r.bottom-r.top,nullptr,nullptr,hi,nullptr);
viewWindow=CreateWindowW(L"CreatingConstructsView",L"",WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,0,0,vw,vh,mainWindow,(HMENU)1,hi,nullptr);
topPanel=CreateWindowW(L"CreatingConstructsMain",L"",WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,10,10,284,160,mainWindow,(HMENU)2,hi,nullptr);sidePanel=CreateWindowW(L"CreatingConstructsMain",L"",WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,W-210,10,200,520,mainWindow,(HMENU)3,hi,nullptr);dockPanel=CreateWindowW(L"CreatingConstructsMain",L"",WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,(W-670)/2,H-116,670,106,mainWindow,(HMENU)4,hi,nullptr);modalPanel=CreateWindowW(L"CreatingConstructsMain",L"",WS_CHILD|WS_CLIPSIBLINGS,0,0,620,510,mainWindow,(HMENU)5,hi,nullptr);
shareEdit=CreateWindowExW(0,L"EDIT",L"",WS_CHILD|WS_BORDER|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN,18,82,524,183,modalPanel,(HMENU)10,hi,nullptr);SendMessage(shareEdit,WM_SETFONT,(WPARAM)font,TRUE);
saveNameEdit=CreateWindowExW(0,L"EDIT",L"My world",WS_CHILD|WS_BORDER|ES_AUTOHSCROLL,18,73,584,26,modalPanel,(HMENU)11,hi,nullptr);SendMessageW(saveNameEdit,WM_SETFONT,(WPARAM)font,TRUE);SendMessageW(saveNameEdit,EM_SETLIMITTEXT,63,0);saveNameOldProc=(WNDPROC)SetWindowLongPtrW(saveNameEdit,GWLP_WNDPROC,(LONG_PTR)saveNameProc);
buildThumbnails();if(smoke)for(int t=0;t<PieceCount;t++)check(pieceThumbs[t]!=nullptr,"3D piece thumbnail generated");ShowWindow(mainWindow,SW_SHOW);layout();UpdateWindow(mainWindow);initVulkan();initPerformance();if(!benchmarkRun&&captureFile.empty()&&(!smoke||audioTest))audioStart();if(benchmarkWalk){toggleWalk();layout();}raisePanels();rebuildActions();SetFocus(viewWindow);
LARGE_INTEGER frequency,start,last,now;QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&start);last=start;MSG msg{};bool running=true;while(running){while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){if(msg.message==WM_QUIT){running=false;break;}TranslateMessage(&msg);DispatchMessage(&msg);}if(!running)break;QueryPerformanceCounter(&now);float dt=std::min(.05f,(float)(now.QuadPart-last.QuadPart)/frequency.QuadPart),time=(float)(now.QuadPart-start.QuadPart)/frequency.QuadPart;last=now;if(benchmarkRun||fixedFrame){dt=1.f/60;time=fixedFrame?3.25f:frames/60.f;}audioUpdate(GetForegroundWindow()==mainWindow&&!IsIconic(mainWindow));render(dt,time);
if(audioTest){
 if(frames==20){audioEvent(A_click);Sleep(180);}
 if(frames==35){
 if(smoke){hit=true;hasAdd=true;hasRem=false;hitAdd={0,18,0};selected=0;unsigned count=audioQueued[A_placeStone];edit();check(get(0,18,0).t==0&&audioQueued[A_placeStone]>count,"actual placement queues its material sound");}
 else audioEdit(false,0,target);
}
 if(frames==50){
 if(smoke){hit=true;hasRem=true;hitRem={0,18,0};erasing=true;unsigned count=audioQueued[A_breakStone];edit();erasing=false;check(get(0,18,0).t<0&&audioQueued[A_breakStone]>count,"actual destruction queues its material sound");}
 else audioEdit(true,19,target);
}
 if(frames==65)audioEvent(A_back);
 if(smoke&&frames==84){
  World oldWorld=world;V oldPlayer=player;float oldVelocity=velY,oldHx=hx,oldHz=hz;bool oldGround=grounded,oldSpace=spaceHeld;
  world=World{};player={1,0,1};velY=hx=hz=0;grounded=true;spaceHeld=false;
  unsigned jumps=audioQueued[A_jump],lands=audioQueued[A_land];keys[VK_SPACE]=true;playerStep(1.f/60);keys[VK_SPACE]=false;
  for(int i=0;i<110;i++)playerStep(1.f/60);
  check(audioQueued[A_jump]>jumps&&audioQueued[A_land]>lands,"first-person jump and landing queue movement cues");
  world=oldWorld;player=oldPlayer;velY=oldVelocity;hx=oldHx;hz=oldHz;grounded=oldGround;spaceHeld=oldSpace;
 }

 if(frames>=130)running=false;
}
if(frames==1){char b[200];sprintf_s(b,"Camera %.2f %.2f %.2f / target %.2f %.2f %.2f / viewport %u %u",eye.x,eye.y,eye.z,target.x,target.y,target.z,extent.width,extent.height);log(b);}
if(smoke){if(frames==10){check(staticCount>10000,"Vulkan geometry upload");foliageTraceSelfTest();action(3);action(5);action(24);}if(frames==14)action(22);if(frames==18)action(27);if(frames==20){action(19);check(GetWindowTextLengthA(shareEdit)>50,"Share dialog contains the exported world JSON");}if(frames==23)action(22);if(frames==26){action(30);check(uiModal==3&&IsWindowVisible(saveNameEdit),"Saves area opens with its name input");}if(frames==29){SetWindowTextW(saveNameEdit,L"Smoke test world");action(31);check(!saveEntries.empty(),"Save new copy action persists a named world");}if(frames==31){action(32);check(uiModal==0,"Load selected action restores a world and closes Saves");}if(frames==25){action(3);action(5);action(4);}if(frames==40)action(4);if(frames==55){action(1);keys['W']=true;}if(frames==80){keys['W']=false;if(audioTest)check(audioQueued[A_stepStone1]+audioQueued[A_stepStone2]+audioQueued[A_stepWood1]+audioQueued[A_stepWood2]+audioQueued[A_stepMetal1]+audioQueued[A_stepMetal2]+audioQueued[A_stepWater]+audioQueued[A_stepStone3]+audioQueued[A_stepStone4]+audioQueued[A_stepWood3]+audioQueued[A_stepWood4]+audioQueued[A_stepMetal3]+audioQueued[A_stepMetal4]+audioQueued[A_stepSoft1]+audioQueued[A_stepSoft2]+audioQueued[A_stepSoft3]+audioQueued[A_stepSoft4]+audioQueued[A_stepWater2]+audioQueued[A_stepWater3]+audioQueued[A_stepWater4]+audioQueued[A_stepWater5]+audioQueued[A_stepWater6]>0,"first-person walking emits footstep cues");check(std::isfinite(player.x)&&player.y>=0,"walk integration");action(1);SetWindowPos(mainWindow,nullptr,0,0,1280,880,SWP_NOMOVE|SWP_NOZORDER);}if(frames==100){world.n=32;put(31,12,31,0);for(int t=0;t<PieceCount;t++)put(t%10,12+t/10,8,t,t%4);dirty=true;}if(frames==102)check(gpu.cells[idx(31,12,31)]==1&&solidAt({31.5f,12.2f,31.5f}),"Vulkan grid upload and collision reach tile 31 in both axes");if(frames==103){world=World{};for(int i=0;i<8;i++)put(i,0,0,37);night=true;dirty=true;}
if(frames==106){check(gpu.lamps[7].w>0,"eighth pendant light illuminates at night");night=false;}
if(frames==109){check(gpu.lamps[7].w==0,"pendant lights switch off during day");night=true;}
if(frames==112)check(gpu.lamps[7].w>0,"pendant lights relight after day-night toggles");
if(frames==115){night=false;starter();dirty=true;}if(frames>=130){check(validationErrors==0,"zero Vulkan validation errors");char b[256];sprintf_s(b,"PASS: %d frames / %.2f seconds / %u vertices / %s",frames,time,vertexCount,gpuName.c_str());log(b);running=false;}}if(benchmarkRun&&frames>=360)running=false;if(!captureFile.empty()&&frames>=6)running=false;if(IsIconic(mainWindow))Sleep(30);}
audioStop();if(audioTest)check(audio.bufferWrites>6&&audio.underruns==0,"live audio survives 180 ms render stall without underruns");if(!smoke&&!benchmarkRun&&captureFile.empty()&&!audioTest){save();audioSaveSettings();}writePerformance();if(verifyGlassBlur){char b[128];sprintf_s(b,"PASS: %d glass panel regions match original blur pixel-for-pixel",verifiedBlurRegions);log(b);}cleanup();DeleteObject(font);DeleteObject(fontBold);DeleteObject(fontTitle);DeleteObject(bgBrush);log("Clean shutdown");if(logFile)fclose(logFile);return validationErrors?2:0;}

