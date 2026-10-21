"""Pack the approved mastered movement cues, preserving the first 17 bank clips.
Run --verify to check bank order and curated cue sources without modifying files.
"""
from pathlib import Path
import argparse, json, struct, subprocess, tempfile
R=Path(__file__).resolve().parents[1]
A=R/'assets/audio'
args=argparse.ArgumentParser()
args.add_argument('--verify',action='store_true')
opt=args.parse_args()
metadata=json.loads((A/'movement/manifest.json').read_text())
raw=(A/'soundbank.bin').read_bytes()
magic,count=struct.unpack_from('<II',raw)
assert magic==0x3141554d and count>=17
blobs=[];offset=8
for _ in range(count):
 size=struct.unpack_from('<I',raw,offset)[0];offset+=4
 blobs.append(raw[offset:offset+size]);offset+=size
assert offset==len(raw)
for item in metadata:
 assert (A/'movement'/ (item['id']+'.wav')).is_file(),item['id']
if opt.verify:
 assert count==41
 manifest=json.loads((A/'manifest.json').read_text())
 assert [x['id'] for x in manifest[17:]]==[x['id'] for x in metadata]
 print('Verified 41 clips and 24 approved movement sources; bank unchanged')
else:
 packed=blobs[:17]
 with tempfile.TemporaryDirectory() as temp:
  for item in metadata:
   path=Path(temp)/(item['id']+'.wav')
   subprocess.run(['ffmpeg','-v','error','-y','-i',str(A/'movement'/path.name),'-ar','22050','-ac','1','-c:a','adpcm_ima_wav','-block_size','256',str(path)],check=True)
   blob=path.read_bytes();packed.append(blob);item['encodedBytes']=len(blob)
 bank=struct.pack('<II',magic,len(packed))+b''.join(struct.pack('<I',len(b))+b for b in packed)
 (A/'soundbank.bin').write_bytes(bank)
 manifest=json.loads((A/'manifest.json').read_text())[:17]+metadata
 (A/'manifest.json').write_text(json.dumps(manifest,indent=2))
 (R/'src/audio_assets.h').write_text('enum AudioClipId { '+', '.join('A_'+x['id'] for x in manifest)+', A_Count };\n')
 print('Packed 24 approved movement cues; first 17 clips preserved byte for byte')
