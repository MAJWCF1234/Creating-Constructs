void audioSelfTest(){
 FootstepCadence steps;
 bool silent=true;for(int i=0;i<120;i++)silent=silent&&!steps.advance(0,true,false,false);check(silent,"standing still produces no footsteps");
 bool emitted=false;for(int i=0;i<60;i++)emitted=steps.advance(.07f,true,false,false)||emitted;
 check(emitted&&steps.serial>=2,"grounded displacement produces spaced walking steps");
 unsigned count=steps.serial;for(int i=0;i<60;i++)steps.advance(.1f,false,false,false);
 check(steps.serial==count,"airborne travel never produces footsteps");
 MovementVariations variants;bool varied=true;
 for(int surface=0;surface<5;surface++){int last=-1;std::vector<int> heard;for(int i=0;i<128;i++){int clip=variants.choose(surface);varied=varied&&clip!=last;last=clip;if(std::find(heard.begin(),heard.end(),clip)==heard.end())heard.push_back(clip);}varied=varied&&heard.size()==(surface==4?6:4);}
 check(varied,"movement banks use every variation without immediate repeats");
 WaterCueGate gate;check(gate.admit(1000)&&!gate.admit(1001)&&!gate.admit(1419)&&gate.admit(1420),"all water variations share a 420 ms anti-overlap gate");
 World originalWorld=world;world=World{};put(1,0,1,24);put(2,0,1,5);put(3,0,1,40);
 check(movementSurface({1.5f,.1f,1.5f})==4&&movementSurface({1.05f,.5f,1.5f})==0,"splash cues detect the pool interior and exclude its concrete rim");
 check(movementSurface({2.5f,1,1.5f})==3&&movementSurface({3.5f,.2f,1.5f})==4&&movementSurface({3.5f,.9f,1.5f})!=4,"movement distinguishes soft ground and actual water height");world=originalWorld;

 for(int i=0;i<A_Count;i++){
  auto& c=audioClips[i];check(c.pcm.size()>100&&c.rate>=12000,"embedded ADPCM clip decodes");
  bool finite=true;float pk=0;for(float v:c.pcm){finite=finite&&std::isfinite(v);pk=std::max(pk,fabsf(v));}
  check(finite&&pk<.32f,"curated source stays beneath safe digital peak ceiling");
  if(i>=A_stepStone1)check(pk<(isWaterStep(i)?.035f:.09f)&&c.pcm.size()/(float)c.rate<.42f,"movement source has a short tail and a restrained peak ceiling");
 }
 for(int i=0;i<3;i++)check(fabsf(audioClips[i].pcm.front()-audioClips[i].pcm.back())<.08f,"music and ambience loops join without a discontinuity");

 for(auto& c:moodMusic){
  check(c.pcm.size()>20*c.rate,"each new mood has its own complete looping score");
  bool valid=true;double energy=0;for(float v:c.pcm){valid=valid&&std::isfinite(v)&&fabsf(v)<.32f;energy+=v*v;}
  check(valid&&sqrt(energy/c.pcm.size())>.01,"mood scores retain gentle peaks and audible musical content");
  check(fabsf(c.pcm.front()-c.pcm.back())<.08f,"generated mood scores wrap without a discontinuity");
 }
 wchar_t temp[MAX_PATH];GetTempPathW(MAX_PATH,temp);
 std::wstring preview=std::wstring(temp)+L"creating-constructs-audio-preview.wav";
 FILE* f=nullptr;_wfopen_s(&f,preview.c_str(),L"wb");check(f!=nullptr,"audio review mix can be written");
 const uint32_t frames=20*AudioMixer::Rate;uint32_t bytes=frames*4;
 fwrite("RIFF",1,4,f);uint32_t n=bytes+36;fwrite(&n,4,1,f);fwrite("WAVEfmt ",1,8,f);n=16;fwrite(&n,4,1,f);
 uint16_t v=1;fwrite(&v,2,1,f);v=2;fwrite(&v,2,1,f);n=AudioMixer::Rate;fwrite(&n,4,1,f);n*=4;fwrite(&n,4,1,f);v=4;fwrite(&v,2,1,f);v=16;fwrite(&v,2,1,f);fwrite("data",1,4,f);fwrite(&bytes,4,1,f);
 AudioMixer test;test.nightTarget.store(1);test.rainTarget.store(0);test.focus.store(.8f);
 int16_t buffer[AudioMixer::Block*2];
 for(int pos=0;pos<75*AudioMixer::Rate;pos+=AudioMixer::Block){
  int frame=pos/AudioMixer::Block;
  // Build, destroy and UI feedback overlap, then run past every loop boundary.
  if(pos<18*AudioMixer::Rate&&frame%75==0){unsigned w=test.write.load();int id=A_placeStone+(frame/75)%(A_slider-A_placeStone+1);test.queue[w%128]={id,1,1,0};test.write.store(w+1);}
  if(pos>12*AudioMixer::Rate)test.rainTarget.store(1);
  test.mix(buffer,AudioMixer::Block);
  if(pos<(int)frames)fwrite(buffer,4,std::min(AudioMixer::Block,(int)frames-pos),f);
 }
 fclose(f);check(test.mixedEvents>=20,"all interaction cues mix over looping music and ambience");
 // Movement-only listening sequence, rendered through the actual runtime mixer at defaults.
 std::wstring movementReview=std::wstring(temp)+L"creating-constructs-movement-review.wav";FILE* movementFile=nullptr;_wfopen_s(&movementFile,movementReview.c_str(),L"wb");check(movementFile!=nullptr,"first-person listening review opens");
 uint32_t movementBytes=16*AudioMixer::Rate*4,movementRiff=movementBytes+36;unsigned char movementHeader[44];
 FILE* headerFile=nullptr;_wfopen_s(&headerFile,preview.c_str(),L"rb");fread(movementHeader,1,44,headerFile);fclose(headerFile);memcpy(movementHeader+4,&movementRiff,4);memcpy(movementHeader+40,&movementBytes,4);fwrite(movementHeader,1,44,movementFile);
 AudioMixer movement;movement.music.store(0);movement.ambience.store(0);movement.focus.store(.8f);MovementVariations reviewVariants;int lastBeat=-1;
 for(int pos=0;pos<16*AudioMixer::Rate;pos+=AudioMixer::Block){
  int beat=pos/(AudioMixer::Rate/2),surface=std::min(4,pos/(3*AudioMixer::Rate));
  if(beat!=lastBeat){lastBeat=beat;unsigned w=movement.write.load();movement.queue[w%128]={reviewVariants.choose(surface),.54f,1.f,beat%2?.045f:-.045f};movement.write.store(w+1);}
  movement.mix(buffer,AudioMixer::Block);fwrite(buffer,4,std::min(AudioMixer::Block,16*AudioMixer::Rate-pos),movementFile);
 }
 fclose(movementFile);check(movement.peak<.04f&&movement.limited==0,"first-person review retains restrained peaks without limiter pumping");
 check(test.peak>.01f&&test.peak<.85f&&test.limited==0,"75-second default mix retains headroom without clipping or limiter pumping");
 test.focus.store(0);for(int i=0;i<100;i++)test.mix(buffer,AudioMixer::Block);
 int peak=0;for(int16_t value:buffer)peak=std::max(peak,abs((int)value));check(peak<=1,"focus loss and mute fade output to silence");
 test.music.store(0);test.ambience.store(0);test.effects.store(0);test.focus.store(.8f);
 for(int i=0;i<1600;i++)test.mix(buffer,AudioMixer::Block);
 peak=0;for(int16_t value:buffer)peak=std::max(peak,abs((int)value));check(peak<=1,"all three volume controls reach silence");

 // Exercise all four soundtracks, including mid-transition reversals.
 AudioMixer moods;moods.focus.store(.8f);bool unity=true;double transitionJump=0;int16_t last=0;
 std::wstring moodReview=std::wstring(temp)+L"creating-constructs-mood-review.wav";FILE* review=nullptr;_wfopen_s(&review,moodReview.c_str(),L"wb");check(review!=nullptr,"mood listening preview opens");
 // Reuse the valid stereo header, then patch its length for 48 seconds.
 FILE* original=nullptr;_wfopen_s(&original,preview.c_str(),L"rb");unsigned char header[44];fread(header,1,44,original);fclose(original);uint32_t dataBytes=48*AudioMixer::Rate*4,riffBytes=dataBytes+36;memcpy(header+4,&riffBytes,4);memcpy(header+40,&dataBytes,4);fwrite(header,1,44,review);
 for(int pos=0;pos<48*AudioMixer::Rate;pos+=AudioMixer::Block){
  int selected=std::min(3,pos/(12*AudioMixer::Rate));moods.moodTarget.store(selected);
  moods.mix(buffer,AudioMixer::Block);
  transitionJump=std::max(transitionJump,abs((int)buffer[0]-last)/32768.);last=buffer[AudioMixer::Block*2-2];
  fwrite(buffer,4,std::min(AudioMixer::Block,48*AudioMixer::Rate-pos),review);
  float sum=0;for(float w:moods.moodWeights)sum+=w;unity=unity&&fabsf(sum-1)<.001f;
 }
 fclose(review);check(unity,"mood crossfade retains unity total gain");check(moods.peak<.85f&&moods.limited==0,"four-track crossfades preserve mix headroom");
 check(transitionJump<.1,"mood changes introduce no abrupt sample jump");
 moods.moodTarget.store(1);moods.mix(buffer,AudioMixer::Block);moods.moodTarget.store(2);moods.mix(buffer,AudioMixer::Block);moods.moodTarget.store(0);
 for(int i=0;i<800;i++)moods.mix(buffer,AudioMixer::Block);
 check(moods.moodWeights[0]>.999f,"rapid mood switches settle on the latest requested soundtrack");
 // Dump decoded PCM so the build check can compare it against FFmpeg's decoder.
 std::wstring decoded=std::wstring(temp)+L"creating-constructs-audio-decoded.bin";_wfopen_s(&f,decoded.c_str(),L"wb");check(f!=nullptr,"ADPCM decoder parity output opens");
 for(auto& c:audioClips){uint32_t count=(uint32_t)c.pcm.size();fwrite(&count,4,1,f);for(float sample:c.pcm){int16_t value=(int16_t)lroundf(sample*32768);fwrite(&value,2,1,f);}}fclose(f);
 char b[160];sprintf_s(b,"Audio offline mix peak %.2f dBFS; %u events; 75 seconds plus fade tests",20*log10(test.peak),test.mixedEvents);log(b);
}
