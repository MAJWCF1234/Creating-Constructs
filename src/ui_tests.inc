void uiSelfTest(){

 for(int type=0;type<PieceCount;type++){int count=0;auto f=pieceFamily(type,count);if(f){bool found=false;for(int i=0;i<count;i++)if(f[i]==type)found=true;check(found,"Every adaptive family includes its selected variant");}}
 {int n=0;auto f=pieceFamily(95,n);check(f&&n==7,"Angle participates in the seven-light cycle");}
 bool oldAtmosphere=atmosphereExpanded,oldWorld=worldExpanded;int oldModal=uiModal;bool oldAudio=audioExpanded,oldSettings=settingsExpanded;int oldScheme=camScheme;
 auto has=[](int id){for(auto a:actions)if(a.action==id)return true;return false;};
 atmosphereExpanded=true;worldExpanded=false;settingsExpanded=false;uiModal=0;rebuildActions();
 check(has(24)&&has(30)&&has(26)&&has(27)&&has(35),"Controls and collapsible section headers are available");
 check(has(300)&&has(309)&&has(42)&&!has(7),"Atmosphere starts expanded with clouds/wind and World starts collapsed");
 for(int c=0;c<CategoryCount;c++)check(has(100+c),"all six HTML category tabs are clickable");
 worldExpanded=true;rebuildActions();check(has(7)&&has(19)&&has(20)&&has(9)&&has(40)&&has(43),"World expands to plot, Share, Hide UI, Export OBJ/FBX and Clear");
 check(has(37),"Redo control is available beside Undo");
 atmosphereExpanded=false;rebuildActions();check(!has(300)&&has(27),"collapsed Atmosphere hides its sliders");
 audioExpanded=true;rebuildActions();check(has(304)&&has(305)&&has(306)&&has(29),"Audio exposes independent music, ambience, effects and mute controls");
 settingsExpanded=true;rebuildActions();check(has(36)&&has(307)&&has(308)&&has(41)&&has(38)&&has(39),"Settings exposes camera, sens, FOV, resolution, fullscreen and photos folder");
 camScheme=1;rebuildActions();check(has(36),"Modifiers camera scheme stays selectable");
 uiModal=3;rebuildActions();check(has(31)&&has(32)&&has(33)&&has(34),"Saves area exposes save, load and paging controls");
 int oldCategory=category;category=5;uiModal=0;dockScrollX=0;rebuildActions();check(has(600)&&has(55)&&has(56),"Material tab exposes surface swatches, texture rotation and scale");category=oldCategory;
 uiModal=5;rebuildActions();check(has(52)&&has(53)&&has(54)&&has(22),"Viewpoints exposes save, paging and close controls");
 uiModal=1;rebuildActions();check(has(22),"Controls dialog provides Close");
 uiModal=2;rebuildActions();check(has(22)&&has(23),"Share dialog provides Load and Close");
 audioExpanded=oldAudio;atmosphereExpanded=oldAtmosphere;worldExpanded=oldWorld;settingsExpanded=oldSettings;camScheme=oldScheme;uiModal=oldModal;rebuildActions();
}
