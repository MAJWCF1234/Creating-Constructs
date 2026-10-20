// Three compact original scores are retained for archival export. Noon keeps its
// approved arrangement; the other active moods use full stereo Complex recordings.
AudioClip moodMusic[3];
const char* moodMusicNames[4]={"Quiet Rooms","Sunlit Steps","The Poolroom","Eastside Story - Mall"};
const int moodMusicIds[4]={A_overcastComplex,A_Count,A_goldenComplex,A_blueComplex};
AudioClip& musicClip(int id){return id<A_Count?audioClips[id]:moodMusic[id-A_Count];}
float noteFrequency(int midi){return 440.f*powf(2.f,(midi-69)/12.f);}
void buildMoodMusic(){
 // C / Am / F / G; F / C / Dm / Am; Am9 / Fmaj7 / Dm9 / Em7.
 const int chords[3][8][4]={
  {{48,52,55,59},{45,48,52,55},{53,57,60,64},{55,59,62,64},{48,52,55,59},{45,48,52,55},{53,57,60,64},{55,59,62,65}},
  {{53,57,60,64},{48,52,55,59},{50,53,57,60},{45,48,52,55},{53,57,60,64},{48,52,55,59},{50,53,57,60},{55,59,62,65}},
  {{45,48,52,59},{53,57,60,64},{50,53,57,64},{52,55,59,62},{45,48,52,59},{53,57,60,64},{50,53,57,64},{52,55,59,62}}
 };
 const int melody[3][16]={
  {72,76,79,76,72,71,69,67,69,72,76,74,71,67,69,71},
  {76,72,69,67,64,67,71,72,74,69,65,64,64,67,69,71},
  {71,76,72,69,72,76,79,76,77,76,72,69,71,74,76,71}
 };
 const float bpm[3]={78,60,50};
 for(int track=0;track<3;track++){
  auto& clip=moodMusic[track];clip.rate=16000;float beat=60.f/bpm[track],bar=beat*4;
  clip.pcm.assign((size_t)lroundf(bar*8*clip.rate),0.f);size_t length=clip.pcm.size();
  auto add=[&](int midi,float when,float duration,float gain,bool pad){
   float hz=noteFrequency(midi),keyPitch=hz/61.7354f;size_t offset=(size_t)lroundf(when*clip.rate);
   const auto& key=audioClips[A_musicKey];int count=(int)(duration*clip.rate);
   for(int i=0;i<count;i++){
    float t=i/(float)clip.rate,phase=2*PI*hz*t,env,value;
    if(pad){env=sinf(PI*i/count);env*=env;value=sinf(phase)+.13f*sinf(phase*2)+.035f*sinf(phase*3);}
    else{
     env=std::min(1.f,t/.016f)*expf(-4*t/duration)*std::min(1.f,(duration-t)/.08f);
     double pos=t*key.rate*keyPitch;size_t k=(size_t)pos;float pluck=0;
     if(k+1<key.pcm.size())pluck=key.pcm[k]+(key.pcm[k+1]-key.pcm[k])*(float)(pos-k);
     value=.65f*(sinf(phase)+.2f*sinf(phase*2)*expf(-3*t)+.045f*sinf(phase*3))+pluck*1.1f;
    }
    clip.pcm[(offset+i)%length]+=value*env*gain;
   }
  };
  for(int b=0;b<8;b++){
   const auto& chord=chords[track][b];
   for(int n=0;n<4;n++)add(chord[n],b*bar,bar*1.5f,track==2?.014f:.009f,true);
   add(chord[0]-12,b*bar,bar*.9f,.018f,false);
   if(track==0){ // Sunny, gently moving broken chords.
    const int order[8]={0,1,2,3,2,1,3,2};
    for(int n=0;n<8;n++)add(chord[order[n]]+12,b*bar+n*beat*.5f,beat*1.8f,n%2?.025f:.032f,false);
   }else if(track==1){ // Slower descending warm plucks and held harmony.
    const int order[4]={3,2,1,2};for(int n=0;n<4;n++)add(chord[order[n]]+12,b*bar+n*beat,beat*2.5f,.032f,false);
   }else{ // Spacious dusk pads with a sparse upper melody.
    add(chord[2]+12,b*bar+beat,beat*2.6f,.022f,false);
   }
   add(melody[track][b*2],b*bar+beat*.25f,beat*3.4f,.032f,false);
   add(melody[track][b*2+1],b*bar+beat*2.5f,beat*2.7f,.027f,false);
  }
  // Circular, quiet delay tails make the authored loop seamless.
  auto dry=clip.pcm;
  const float times[3]={.217f,.431f,.683f},gains[3]={.12f,.065f,.035f};
  for(int tap=0;tap<3;tap++){size_t delay=(size_t)(times[tap]*clip.rate);for(size_t i=0;i<length;i++)clip.pcm[(i+delay)%length]+=dry[i]*gains[tap];}
  double sum=0;float peak=0;for(float v:clip.pcm){sum+=v*v;peak=std::max(peak,fabsf(v));}
  float rms=(float)sqrt(sum/length),gain=std::min(.048f/std::max(.00001f,rms),.251f/std::max(.00001f,peak));
  for(float& v:clip.pcm)v*=gain;
 }
}
