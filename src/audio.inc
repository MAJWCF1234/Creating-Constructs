// Embedded IMA ADPCM clips; independent event-driven WinMM mixer (no frame-thread I/O).
#include <mmsystem.h>
#include <atomic>
#include "audio_assets.h"
struct AudioClip {std::vector<float> pcm;int rate=16000,channels=1;};
AudioClip audioClips[A_Count];
#include "mood_music.inc"
bool audioTest=false;unsigned audioQueued[A_Count]{};
float musicVolume=.65f,ambienceVolume=.65f,effectsVolume=.75f;bool audioMuted=false;
std::wstring audioSettingsPath;
struct AudioEvent {int clip;float gain,pitch,pan;};
struct AudioVoice {int clip=-1;double cursor=0;float gain=0,pitch=1,pan=0;};
struct AudioMixer {
 static constexpr int Rate=48000,Block=512,Buffers=6;
 HWAVEOUT device=nullptr;HANDLE event=nullptr,thread=nullptr;
 std::atomic<bool> running{false};std::atomic<unsigned> write{0},read{0};AudioEvent queue[128]{};
 std::atomic<float> music{.65f},ambience{.65f},effects{.75f},focus{1},nightTarget{0},rainTarget{0};
 AudioVoice voices[8];double loops[3]{},moodCursors[4]{};
 std::atomic<int> moodTarget{0};double moodWeights[4]={1,0,0,0};
 float master=0,musicGain=0,ambienceGain=0,effectsGain=0,nightGain=0,rainGain=0,limiter=1,rainLow=0;
 uint32_t random=13925;unsigned mixedEvents=0,underruns=0,bufferWrites=0,limited=0;float peak=0;double squares=0;uint64_t samples=0;
 WAVEHDR headers[Buffers]{};int16_t output[Buffers][Block*2]{};
 bool decode(const unsigned char* p,size_t bytes,AudioClip& clip){
  if(bytes<44||memcmp(p,"RIFF",4)||memcmp(p+8,"WAVE",4))return false;
  int format=0,channels=0,block=0;const unsigned char* data=nullptr;size_t length=0,fact=0;
  auto u16=[](const unsigned char* x){return int(x[0])+(int(x[1])<<8);};
  auto u32=[](const unsigned char* x){return uint32_t(x[0])|(uint32_t(x[1])<<8)|(uint32_t(x[2])<<16)|(uint32_t(x[3])<<24);};
  for(size_t off=12;off+8<=bytes;){size_t n=u32(p+off+4);if(n>bytes-off-8)return false;
   const auto* q=p+off+8;
   if(!memcmp(p+off,"fmt ",4)&&n>=16){format=u16(q);channels=u16(q+2);clip.channels=channels;clip.rate=(int)u32(q+4);block=u16(q+12);}
   if(!memcmp(p+off,"fact",4)&&n>=4)fact=u32(q);
   if(!memcmp(p+off,"data",4)){data=q;length=n;}off+=8+n+(n&1);
  }
  if(format!=17||(channels!=1&&channels!=2)||block<4*channels||!data||clip.rate<8000||clip.rate>48000)return false;
  static const int steps[89]={7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};
  static const int indices[8]={-1,-1,-1,-1,2,4,6,8};
  auto nibble=[&](int nib,int& value,int& index){int step=steps[index],delta=step>>3;if(nib&1)delta+=step>>2;if(nib&2)delta+=step>>1;if(nib&4)delta+=step;value=std::clamp(value+((nib&8)?-delta:delta),-32768,32767);index=std::clamp(index+indices[nib&7],0,88);return value/32768.f;};
  for(size_t off=0;off+4*channels<=length;off+=block){
   const auto* q=data+off;int value[2]{},index[2]{};
   for(int ch=0;ch<channels;ch++){value[ch]=(int16_t)u16(q+ch*4);index[ch]=q[ch*4+2];if(index[ch]>88)return false;clip.pcm.push_back(value[ch]/32768.f);}
   size_t end=std::min((size_t)block,length-off);
   if(channels==1){for(size_t j=4;j<end;j++)for(int k=0;k<2;k++)clip.pcm.push_back(nibble((q[j]>>(k*4))&15,value[0],index[0]));}
   else for(size_t group=8;group+8<=end;group+=8){
    float samples[2][8];
    for(int ch=0;ch<2;ch++)for(int j=0;j<4;j++)for(int k=0;k<2;k++)samples[ch][j*2+k]=nibble((q[group+ch*4+j]>>(k*4))&15,value[ch],index[ch]);
    for(int frame=0;frame<8;frame++)for(int ch=0;ch<2;ch++)clip.pcm.push_back(samples[ch][frame]);
   }
  }
  if(fact&&fact*channels<clip.pcm.size())clip.pcm.resize(fact*channels);return !clip.pcm.empty();
 }
 float sample(int id,double& cursor,float pitch,bool loop){
  const auto& clip=musicClip(id);size_t n=clip.pcm.size();if(n<2)return 0;
  if(cursor>=n){if(loop)cursor=fmod(cursor,(double)n);else return 0;}
  size_t i=(size_t)cursor,j=i+1<n?i+1:(loop?0:i);float t=(float)(cursor-i);
  float v=clip.pcm[i]+(clip.pcm[j]-clip.pcm[i])*t;cursor+=clip.rate*pitch/Rate;return v;
 }
 void stereoMusic(int id,double& cursor,float& left,float& right){
  const auto& clip=musicClip(id);size_t count=clip.pcm.size()/clip.channels;if(count<2){left=right=0;return;}
  if(cursor>=count)cursor=fmod(cursor,(double)count);size_t a=(size_t)cursor,b=(a+1)%count;float t=(float)(cursor-a);
  auto at=[&](int channel){int ch=std::min(channel,clip.channels-1);float x=clip.pcm[a*clip.channels+ch],y=clip.pcm[b*clip.channels+ch];return x+(y-x)*t;};
  left=at(0);right=at(1);cursor+=clip.rate/(double)Rate;
 }
 void mix(int16_t* dest,int count){
  unsigned r=read.load(std::memory_order_relaxed),w=write.load(std::memory_order_acquire);
  while(r!=w){auto e=queue[r%128];AudioVoice* voice=nullptr;
   for(auto& v:voices)if(v.clip<0){voice=&v;break;}
   // Keep interaction feedback bounded during large drag strokes; hover never steals a voice.
   if(!voice&&e.clip!=A_hover){voice=&voices[0];for(auto& v:voices)if(v.gain<voice->gain)voice=&v;}
   if(voice)*voice={e.clip,0,e.gain,e.pitch,e.pan};r++;mixedEvents++;
  }read.store(r,std::memory_order_release);
  int selectedMood=std::clamp(moodTarget.load(),0,3);
  float tm=music.load(),ta=ambience.load(),te=effects.load(),tf=focus.load(),tn=nightTarget.load(),tr=rainTarget.load();
  for(int frame=0;frame<count;frame++){
   master+=(tf-master)*.0007f;musicGain+=(tm-musicGain)*.000015f;ambienceGain+=(ta-ambienceGain)*.00004f;effectsGain+=(te-effectsGain)*.0007f;
   nightGain+=(tn-nightGain)*.000025f;rainGain+=(tr-rainGain)*.00004f;
   // Convex crossfades stay at unity total gain, including rapid mood changes.
   float musicLeft=0,musicRight=0;
   for(int m=0;m<4;m++){
    moodWeights[m]+=((m==selectedMood?1.f:0.f)-moodWeights[m])*.00002f;
    if(moodWeights[m]>.00001f){float l,r;stereoMusic(moodMusicIds[m],moodCursors[m],l,r);musicLeft+=l*(float)moodWeights[m];musicRight+=r*(float)moodWeights[m];}
   }
   musicLeft*=musicGain;musicRight*=musicGain;
   float wind=sample(A_wind,loops[1],1,true),crickets=sample(A_crickets,loops[2],1,true);
   random^=random<<13;random^=random>>17;random^=random<<5;
   float noise=(int32_t)random/2147483648.f;rainLow+=(noise-rainLow)*.12f;
   float ambient=(wind*.7f+crickets*nightGain*.65f+rainLow*rainGain*.045f)*ambienceGain;
   float left=musicLeft+ambient,right=musicRight+ambient;
   for(auto& v:voices)if(v.clip>=0){float s=sample(v.clip,v.cursor,v.pitch,false)*v.gain*effectsGain;
    left+=s*sqrtf((1-v.pan)*.5f);right+=s*sqrtf((1+v.pan)*.5f);
    if(v.cursor>=audioClips[v.clip].pcm.size())v.clip=-1;
   }
   left*=master;right*=master;float pk=std::max(fabsf(left),fabsf(right));
   float wanted=pk>.85f?.85f/pk:1.f;if(wanted<limiter){limiter=wanted;limited++;}else limiter+=(1-limiter)*.0001f;
   left*=limiter;right*=limiter;peak=std::max(peak,std::max(fabsf(left),fabsf(right)));squares+=left*left+right*right;samples+=2;
   dest[frame*2]=(int16_t)(std::clamp(left,-.999f,.999f)*32767);dest[frame*2+1]=(int16_t)(std::clamp(right,-.999f,.999f)*32767);
  }
 }
 static DWORD WINAPI worker(void* context){auto& a=*(AudioMixer*)context;
  while(a.running.load()){
   int done=0;for(auto& h:a.headers)if(h.dwFlags&WHDR_DONE)done++;
   if(done==Buffers&&a.bufferWrites>Buffers)a.underruns++;
   for(int i=0;i<Buffers;i++)if(a.headers[i].dwFlags&WHDR_DONE){a.mix(a.output[i],Block);waveOutWrite(a.device,&a.headers[i],sizeof(WAVEHDR));a.bufferWrites++;}
   WaitForSingleObject(a.event,20);
  }return 0;
 }
} audio;
void audioSaveSettings(){if(audioSettingsPath.empty())return;FILE* f=nullptr;_wfopen_s(&f,audioSettingsPath.c_str(),L"wb");if(f){fprintf(f,"%.4f %.4f %.4f %d\n",musicVolume,ambienceVolume,effectsVolume,audioMuted?1:0);fclose(f);}}
void audioLoadSettings(const std::wstring& directory){audioSettingsPath=directory+L"\\audio.cfg";FILE* f=nullptr;_wfopen_s(&f,audioSettingsPath.c_str(),L"rb");if(f){int muted=0;float m,a,e;if(fscanf_s(f,"%f %f %f %d",&m,&a,&e,&muted)==4&&std::isfinite(m)&&std::isfinite(a)&&std::isfinite(e)){musicVolume=std::clamp(m,0.f,1.f);ambienceVolume=std::clamp(a,0.f,1.f);effectsVolume=std::clamp(e,0.f,1.f);audioMuted=muted!=0;}fclose(f);}}
std::wstring inputSettingsPath;
void inputSaveSettings(){if(inputSettingsPath.empty())return;FILE* f=nullptr;_wfopen_s(&f,inputSettingsPath.c_str(),L"wb");if(f){fprintf(f,"%d %.3f %.1f\n",camScheme,mouseSens,buildFov);fclose(f);}}
void inputLoadSettings(const std::wstring& directory){inputSettingsPath=directory+L"\\input.cfg";FILE* f=nullptr;_wfopen_s(&f,inputSettingsPath.c_str(),L"rb");if(f){int scheme=0;float sens=1.f,fov=60.f;int n=fscanf_s(f,"%d %f %f",&scheme,&sens,&fov);if(n>=1)camScheme=scheme==1?1:0;if(n>=2&&std::isfinite(sens))mouseSens=std::clamp(sens,.35f,2.2f);if(n>=3&&std::isfinite(fov))buildFov=std::clamp(fov,40.f,100.f);fclose(f);}}
bool audioLoadBank(){HRSRC r=FindResourceW(nullptr,MAKEINTRESOURCEW(201),RT_RCDATA);if(!r)return false;auto* p=(const unsigned char*)LockResource(LoadResource(nullptr,r));size_t size=SizeofResource(nullptr,r),off=8;uint32_t magic=0,count=0;if(size<8)return false;memcpy(&magic,p,4);memcpy(&count,p+4,4);if(magic!=0x3141554d||count!=A_Count)return false;
 for(int i=0;i<A_Count;i++){uint32_t n=0;if(off+4>size)return false;memcpy(&n,p+off,4);off+=4;if(n>size-off||!audio.decode(p+off,n,audioClips[i]))return false;off+=n;}if(off!=size)return false;buildMoodMusic();return true;
}
bool isWaterStep(int clip){return clip==A_stepWater||(clip>=A_stepWater2&&clip<=A_stepWater6);}
struct WaterCueGate{ULONGLONG last=0;bool ready(ULONGLONG now)const{return !last||now-last>=420;}bool admit(ULONGLONG now){if(!ready(now))return false;last=now;return true;}} waterCueGate;
void audioEvent(int clip,float gain=1,float pitch=1,float pan=0){
 if(!audio.running.load()||audioMuted)return;static ULONGLONG last[A_Count]{};auto now=GetTickCount64();
 if(isWaterStep(clip)&&!waterCueGate.admit(now))return;
 unsigned cooldown=clip==A_hover?90:clip==A_slider?100:clip>=A_placeStone&&clip<=A_breakMetal?65:35;
 if(now-last[clip]<cooldown)return;last[clip]=now;
 unsigned w=audio.write.load(std::memory_order_relaxed),r=audio.read.load(std::memory_order_acquire);if(w-r>=128)return;
 audioQueued[clip]++;audio.queue[w%128]={clip,gain,pitch,std::clamp(pan,-.85f,.85f)};audio.write.store(w+1,std::memory_order_release);
}
int audioMaterial(int type){if(type==14||type==19||type==35||type==39||type==41||type==42)return 1;if(type==11||type==20||type==13||type==16||type==21||type==22||type==25||type==26||type==27||type==31||type==37||type==43||type==44||type==45||type==46)return 2;return 0;}
void audioEdit(bool destroyed,int type,V position){
 int material=audioMaterial(type);V d=position-eye;float distance=len(d),pan=distance>.01f?dot(d*(1/distance),rightv)*.65f:0;
 static unsigned variation=0;variation++;float pitch=.94f+(variation%7)*.018f;
 audioEvent((destroyed?A_breakStone:A_placeStone)+material,1/(1+distance*.035f),pitch,pan);
}
struct FootstepCadence {
 float distance=.6f;unsigned serial=0;
 bool advance(float moved,bool onGround,bool crouched,bool sprinting){
  if(!onGround){distance=0;return false;}
  if(moved<.00001f)return false;
  distance+=std::min(moved,.2f);float stride=crouched?1.1f:sprinting?2.1f:1.7f;
  if(distance<stride)return false;distance=fmodf(distance,stride);serial++;return true;
 }
} footstepCadence;
float cameraWaterDepth(V camera);
int movementSurface(V feet){
 if(cameraWaterDepth(feet+V{0,.06f,0})>.015f)return 4;
 int type=get((int)floorf(feet.x),(int)floorf(feet.y-.08f),(int)floorf(feet.z)).t;
 if(type==5||type==17)return 3;
 return audioMaterial(type);
}
struct MovementVariations {
 uint32_t random=0x831c53u;int last[5]={-1,-1,-1,-1,-1};
 int choose(int surface){
  static const int clips[5][6]={{A_stepStone1,A_stepStone2,A_stepStone3,A_stepStone4},{A_stepWood1,A_stepWood2,A_stepWood3,A_stepWood4},{A_stepMetal1,A_stepMetal2,A_stepMetal3,A_stepMetal4},{A_stepSoft1,A_stepSoft2,A_stepSoft3,A_stepSoft4},{A_stepWater,A_stepWater2,A_stepWater3,A_stepWater4,A_stepWater5,A_stepWater6}};
  random^=random<<13;random^=random>>17;random^=random<<5;int count=surface==4?6:4;
  int index=last[surface]<0?random%count:(last[surface]+1+random%(count-1))%count;
  last[surface]=index;return clips[surface][index];
 }
} movementVariations;
void audioFootstep(V before,bool wasGrounded,float fallingSpeed){
 bool crouched=keys['C']||keys[VK_CONTROL];bool sprinting=keys[VK_SHIFT]&&!crouched;
 V delta=player-before;if(len(delta)>1.f){footstepCadence.distance=0;return;}
 static float climbDistance=0;auto body=get((int)floorf(player.x),(int)floorf(player.y+.9f),(int)floorf(player.z));if(body.t==25){climbDistance+=fabsf(delta.y);if(climbDistance>.9f){climbDistance=0;audioEvent(movementVariations.choose(2),.28f,.97f);}return;}climbDistance=0;float moved=sqrtf(delta.x*delta.x+delta.z*delta.z);
 int surface=movementSurface(player);
 if(grounded&&!wasGrounded&&fallingSpeed< -3.f){if(surface!=4||waterCueGate.ready(GetTickCount64()))audioEvent(surface==4?movementVariations.choose(4):A_land,std::clamp((-fallingSpeed-2)*.045f,.18f,.55f),.98f);footstepCadence.distance=0;return;}
 if(!footstepCadence.advance(moved,grounded,crouched,sprinting))return;
 if(surface==4&&!waterCueGate.ready(GetTickCount64()))return;
 int clip=movementVariations.choose(surface);float gain=crouched?.28f:sprinting?.68f:.54f;
 float pitch=.975f+(movementVariations.random%7)*.008f;
 audioEvent(clip,gain,pitch,footstepCadence.serial%2?.045f:-.045f);
}
void audioUpdate(bool focused){audio.moodTarget.store(mood);audio.music.store(musicVolume);audio.ambience.store(ambienceVolume);audio.effects.store(effectsVolume);audio.focus.store(focused&&!audioMuted?.8f:0.f);audio.nightTarget.store(night?1.f:0.f);audio.rainTarget.store(rain?1.f:0.f);}
void audioStart(){audio.moodTarget.store(mood);for(int m=0;m<4;m++)audio.moodWeights[m]=m==mood?1.f:0.f;
 audio.event=CreateEventW(nullptr,FALSE,FALSE,nullptr);WAVEFORMATEX f{};f.wFormatTag=WAVE_FORMAT_PCM;f.nChannels=2;f.nSamplesPerSec=AudioMixer::Rate;f.wBitsPerSample=16;f.nBlockAlign=4;f.nAvgBytesPerSec=f.nSamplesPerSec*4;
 if(!audio.event||waveOutOpen(&audio.device,WAVE_MAPPER,&f,(DWORD_PTR)audio.event,0,CALLBACK_EVENT)!=MMSYSERR_NOERROR){log("Audio output unavailable; sound disabled");if(audio.event)CloseHandle(audio.event);audio.event=nullptr;return;}
 for(int i=0;i<AudioMixer::Buffers;i++){auto& h=audio.headers[i];h.lpData=(LPSTR)audio.output[i];h.dwBufferLength=sizeof(audio.output[i]);waveOutPrepareHeader(audio.device,&h,sizeof(h));h.dwFlags|=WHDR_DONE;}
 audio.running.store(true);audio.thread=CreateThread(nullptr,0,AudioMixer::worker,&audio,0,nullptr);
 if(!audio.thread){audio.running.store(false);log("Audio worker could not start");}else log("Audio: embedded music / wind / crickets / material edits / UI; 48 kHz stereo, six buffers");
}
void audioStop(){
 audio.running.store(false);if(audio.event)SetEvent(audio.event);if(audio.thread){WaitForSingleObject(audio.thread,INFINITE);CloseHandle(audio.thread);audio.thread=nullptr;}
 if(audio.device){waveOutReset(audio.device);for(auto& h:audio.headers)waveOutUnprepareHeader(audio.device,&h,sizeof(h));waveOutClose(audio.device);audio.device=nullptr;}
 if(audio.event){CloseHandle(audio.event);audio.event=nullptr;}
 char b[256];sprintf_s(b,"Audio: %u events, %u buffers, %u underruns, peak %.2f dBFS, %u limiter attacks",audio.mixedEvents,audio.bufferWrites,audio.underruns,20*log10(std::max(.000001f,audio.peak)),audio.limited);log(b);
}

void exportMoodSoundtracks(){
 wchar_t exe[MAX_PATH];GetModuleFileNameW(nullptr,exe,MAX_PATH);std::wstring folder=exe;folder.resize(folder.find_last_of(L"\\"));folder.resize(folder.find_last_of(L"\\"));folder+=L"\\Saved soundtracks";CreateDirectoryW(folder.c_str(),nullptr);
 const int archivedIds[4]={A_music,A_Count,A_Count+1,A_Count+2};const char* archivedNames[4]={"Quiet Rooms","Sunlit Steps","Amber Windows","After Hours"};
 for(int moodIndex=0;moodIndex<4;moodIndex++){
  auto& clip=musicClip(archivedIds[moodIndex]);std::wstring title;for(const char* c=archivedNames[moodIndex];*c;c++)title+=(wchar_t)*c;
  std::wstring path=folder+L"\\"+title+L".wav";FILE* f=nullptr;_wfopen_s(&f,path.c_str(),L"wb");if(!f){log("Could not export soundtrack");continue;}
  uint32_t bytes=(uint32_t)clip.pcm.size()*2,n=bytes+36;uint16_t v=1;
  fwrite("RIFF",1,4,f);fwrite(&n,4,1,f);fwrite("WAVEfmt ",1,8,f);n=16;fwrite(&n,4,1,f);fwrite(&v,2,1,f);fwrite(&v,2,1,f);n=clip.rate;fwrite(&n,4,1,f);n*=2;fwrite(&n,4,1,f);v=2;fwrite(&v,2,1,f);v=16;fwrite(&v,2,1,f);fwrite("data",1,4,f);fwrite(&bytes,4,1,f);
  for(float value:clip.pcm){int16_t sample=(int16_t)lroundf(std::clamp(value,-.999f,.999f)*32767);fwrite(&sample,2,1,f);}fclose(f);log(archivedNames[moodIndex]);
 }
}
