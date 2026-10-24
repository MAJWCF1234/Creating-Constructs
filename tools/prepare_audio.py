"""Curate quiet, compact mono audio from the local Complex pack; originals stay untouched."""
from pathlib import Path
import subprocess, json, hashlib, struct, tempfile
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
SOURCE=Path(r'O:\retro official\Complex')
OUT=ROOT/'assets/audio'; OUT.mkdir(parents=True,exist_ok=True)
# name, path, offset, duration (0=full), sample rate, LUFS, crossfade seconds
CLIPS=[
 ('music','Music/expedition_pianoroom.wav',0,0,16000,-26,2),
 ('wind','SFX/House_Ambience_Wind.wav',4,14,12000,-32,1),
 ('crickets','SFX/LightsAndAmb/CricketsAmbienceEnd.wav',0,8,12000,-34,1),
 ('placeStone','SFX/Movement/Footsteps/Asphalt/FS-Asphalt_1.wav',0,.45,16000,-23,0),
 ('placeWood','SFX/Movement/Footsteps/Wood/FS-Wood_1.wav',0,.45,16000,-23,0),
 ('placeMetal','SFX/Movement/Footsteps/Stairs/Metal/MetalStairs_FS1.wav',0,.45,16000,-24,0),
 ('breakStone','SFX/Cutscenes/StatueHeadFall_Ending_2.wav',0,.65,16000,-24,0),
 ('breakWood','SFX/LadderStepOn.wav',0,.65,16000,-24,0),
 ('breakMetal','SFX/Cutscenes/MetalClash.wav',0,.8,16000,-26,0),
 ('click','SFX/UI/UI_Click.wav',0,.22,16000,-26,0),
 ('hover','SFX/UI/UI_Hover.wav',0,.14,16000,-30,0),
 ('back','SFX/UI/UI_Back.wav',0,.26,16000,-26,0),
 ('slider','SFX/UI/UI_Slider.wav',0,.12,16000,-30,0),
 ('musicKey','Music/BassPluckLoop.wav',0,.85,16000,-26,0),
 ('overcastComplex','Music/expedition_pianoroom.wav',0,0,44100,-26,3),
 ('goldenComplex','Music/ThePoolroom.wav',0,0,44100,-26,2),
 ('blueComplex','Music/Eastside_Story_-_Mall.wav',0,0,44100,-26,2),
 ('stepStone1','SFX/Movement/Footsteps/Asphalt/FS-Asphalt_2.wav',0,.5,22050,-27,0),
 ('stepStone2','SFX/Movement/Footsteps/Asphalt/FS-Asphalt_3.wav',0,.5,22050,-27,0),
 ('stepWood1','SFX/Movement/Footsteps/Wood/FS-Wood_2.wav',0,.5,22050,-27,0),
 ('stepWood2','SFX/Movement/Footsteps/Wood/FS-Wood_3.wav',0,.5,22050,-27,0),
 ('stepMetal1','SFX/Movement/Footsteps/Stairs/Metal/MetalStairs_FS2.wav',0,.5,22050,-29,0),
 ('stepMetal2','SFX/Movement/Footsteps/Stairs/Metal/MetalStairs_FS3.wav',0,.5,22050,-29,0),
 ('stepWater','SFX/Movement/Water/Water_WalkingSFX.wav',0,.6,22050,-29,0),
 ('jump','SFX/Movement/Cloth_Movement.wav',0,.3,22050,-32,0),
 ('land','SFX/CameraLand_Carpet.wav',0,.55,22050,-27,0),
]
manifest=[]; blobs=[]
with tempfile.TemporaryDirectory() as temp:
 for name,rel,offset,duration,rate,lufs,loop in CLIPS:
  channels=2 if name.endswith("Complex") else 1
  source=SOURCE/rel
  args=['ffmpeg','-v','error','-ss',str(offset),'-i',str(source)]
  if duration: args+=['-t',str(duration)]
  args+=['-vn','-af',f'highpass=f=55,lowpass=f={min(rate*.44,6800)},loudnorm=I={lufs}:TP=-9:LRA=9','-ac',str(channels),'-ar',str(rate),'-f','f32le','-']
  samples=np.frombuffer(subprocess.check_output(args),dtype='<f4').copy()
  if loop:
   n=min(int(loop*rate)*channels,(len(samples)//(4*channels))*channels); t=np.repeat(np.linspace(0,1,n//channels,dtype=np.float32),channels)
   samples=np.concatenate([samples[n:-n],samples[-n:]*(1-t)+samples[:n]*t])
  else:
   n=min(int(.007*rate),len(samples)//4)
   samples[:n]*=np.linspace(0,1,n); samples[-n:]*=np.linspace(1,0,n)
  # Leave source dynamics intact beneath the explicit peak ceiling.
  samples*=min(1.,.251/max(1e-9,float(np.max(np.abs(samples)))))
  dest=Path(temp)/(name+'.wav')
  subprocess.run(['ffmpeg','-v','error','-y','-f','f32le','-ar',str(rate),'-ac',str(channels),'-i','-','-c:a','adpcm_ima_wav','-block_size',str(1024 if channels==2 else 256),str(dest)],input=samples.astype('<f4').tobytes(),check=True)
  blob=dest.read_bytes();blobs.append(blob)
  manifest.append(dict(id=name,source=str(source),sha256=hashlib.sha256(source.read_bytes()).hexdigest(),sourceOffset=offset,duration=len(samples)/rate/channels,rate=rate,channels=channels,targetLUFS=lufs,peakDBFS=float(20*np.log10(max(1e-9,np.max(np.abs(samples))))),rmsDBFS=float(20*np.log10(max(1e-9,np.sqrt(np.mean(samples*samples))))),encodedBytes=len(blob)))
pack=struct.pack('<II',0x3141554d,len(blobs))+b''.join(struct.pack('<I',len(x))+x for x in blobs)
(OUT/'soundbank.bin').write_bytes(pack)
(OUT/'manifest.json').write_text(json.dumps(manifest,indent=2))
(ROOT/'src/generated/audio_assets.h').write_text('enum AudioClipId { '+', '.join('A_'+c[0] for c in CLIPS)+', A_Count };\n')
print(f'Audio bank: {len(pack)} bytes; {len(blobs)} curated clips')

# Movement has separate transient mastering; retain the approved first 17 clips.
subprocess.run([__import__("sys").executable,str(ROOT/"tools/remaster_movement.py")],check=True)
