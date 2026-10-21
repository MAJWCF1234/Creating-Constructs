// Optional QA images use the same Vulkan image and native UI painter as the window.
void menuReviewImage(const wchar_t* name){
 if(!menuReviewImages||!uiBackgroundReady)return;
 BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=W;info.bmiHeader.biHeight=-H;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
 void* pixels=nullptr;HDC dc=CreateCompatibleDC(nullptr);HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,nullptr,0);auto previous=SelectObject(dc,bitmap);
 BITMAPINFO source=info;source.bmiHeader.biWidth=uiBackgroundWidth;source.bmiHeader.biHeight=-(int)uiBackgroundHeight;
 StretchDIBits(dc,0,0,W,H,0,0,uiBackgroundWidth,uiBackgroundHeight,uiBackgroundMap,&source,DIB_RGB_COLORS,SRCCOPY);composeUi(dc);GdiFlush();
 wchar_t temp[MAX_PATH];GetTempPathW(MAX_PATH,temp);std::wstring directory=std::wstring(temp)+L"CreatingConstructsMenuReview";CreateDirectoryW(directory.c_str(),nullptr);
 BITMAPFILEHEADER file{};file.bfType=0x4d42;file.bfOffBits=sizeof(file)+sizeof(BITMAPINFOHEADER);file.bfSize=file.bfOffBits+W*H*4;FILE* f=nullptr;_wfopen_s(&f,(directory+L"\\"+name).c_str(),L"wb");if(f){fwrite(&file,sizeof(file),1,f);fwrite(&info.bmiHeader,sizeof(BITMAPINFOHEADER),1,f);fwrite(pixels,4,(size_t)W*H,f);fclose(f);}
 SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);
}
void clickMenuAction(int id){
 rebuildActions();for(auto a:actions)if(a.action==id){RECT panel;GetWindowRect(modalPanel,&panel);POINT p{panel.left,panel.top};ScreenToClient(mainWindow,&p);int x=(a.rect.left+a.rect.right)/2-p.x,y=(a.rect.top+a.rect.bottom)/2-p.y;SendMessageW(modalPanel,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));SendMessageW(modalPanel,WM_LBUTTONUP,0,MAKELPARAM(x,y));return;}
 check(false,"menu action has a clickable rectangle");
}
void menuSmokeStage(){
if(frames==2){keys['W']=true;showUiModal(6);check(menuActive&&uiModal==6&&!keys['W']&&!IsWindowVisible(dockPanel),"main menu clears input and hides the editor");check(actions.size()==5,"main menu exposes five isolated actions");bool priorWalk=walk;V priorPlayer=player;float oldVelocity=velY;walk=true;keys['W']=true;camera(.05f);check(len(player-priorPlayer)<.00001f&&velY==oldVelocity,"main menu pauses first-person movement and gravity");keys['W']=false;walk=priorWalk;}
if(frames==3){clickMenuAction(62);check(uiModal==7&&menuActive,"menu settings opens without entering the world");float prior=musicVolume;setUiSlider(304,.31f);check(fabsf(musicVolume-.31f)<.001f,"menu audio slider changes the real mixer");musicVolume=prior;action(22);check(uiModal==6,"settings Back returns to main menu");}
if(frames==4){clickMenuAction(61);check(uiModal==3&&menuActive&&IsWindowVisible(saveNameEdit),"main menu opens the real saved world browser");SetWindowTextW(saveNameEdit,L"Menu load test");action(31);action(32);check(!menuActive&&uiModal==0,"loading a saved world from the main menu enters the game");showUiModal(6);action(63);check(uiModal==1&&menuActive,"main menu controls opens");keyDown(VK_ESCAPE,false);check(uiModal==6,"Escape returns from controls to main menu");}
if(frames==5){keyDown(VK_DOWN,false);check(menuSelection==1,"menu keyboard navigation selects saved worlds");keyDown(VK_UP,false);keyDown(VK_RETURN,false);check(!menuActive&&uiModal==0&&IsWindowVisible(dockPanel),"Play restores editor and resumes gameplay");keyDown(VK_ESCAPE,false);check(menuActive,"Escape opens pause menu in game");keyDown(VK_ESCAPE,false);check(!menuActive,"Escape resumes from pause menu");}
if(menuReviewImages&&frames==6){showUiModal(6);menuReviewImage(L"main-menu.bmp");}
if(menuReviewImages&&frames==7){action(62);menuReviewImage(L"settings.bmp");}
if(menuReviewImages&&frames==8){action(22);action(61);menuReviewImage(L"saved-worlds.bmp");}
if(menuReviewImages&&frames==9){action(22);action(60);}
}
