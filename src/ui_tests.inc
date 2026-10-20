void uiSelfTest(){
 bool oldAtmosphere=atmosphereExpanded,oldWorld=worldExpanded;int oldModal=uiModal;bool oldAudio=audioExpanded;
 auto has=[](int id){for(auto a:actions)if(a.action==id)return true;return false;};
 atmosphereExpanded=true;worldExpanded=false;uiModal=0;rebuildActions();
 check(has(24)&&has(30)&&has(26)&&has(27),"Controls and collapsible section headers are available");
 check(has(300)&&!has(7),"Atmosphere starts expanded and World starts collapsed");
 for(int c=0;c<CategoryCount;c++)check(has(100+c),"all six HTML category tabs are clickable");
 worldExpanded=true;rebuildActions();check(has(7)&&has(19)&&has(20)&&has(9),"World expands to plot, Share, Hide UI and Clear");
 atmosphereExpanded=false;rebuildActions();check(!has(300)&&has(27),"collapsed Atmosphere hides its sliders");
 audioExpanded=true;rebuildActions();check(has(304)&&has(305)&&has(306)&&has(29),"Audio exposes independent music, ambience, effects and mute controls");
 uiModal=3;rebuildActions();check(has(31)&&has(32)&&has(33)&&has(34),"Saves area exposes save, load and paging controls");
 uiModal=1;rebuildActions();check(has(22),"Controls dialog provides Close");
 uiModal=2;rebuildActions();check(has(22)&&has(23),"Share dialog provides Load and Close");
 audioExpanded=oldAudio;atmosphereExpanded=oldAtmosphere;worldExpanded=oldWorld;uiModal=oldModal;rebuildActions();
}
