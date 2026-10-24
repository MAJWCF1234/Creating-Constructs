"""Embed curated MUAD SFX+ambience clips into Creating Constructs.html (skip long Complex music)."""
from pathlib import Path
import re, struct

ROOT = Path(__file__).resolve().parents[1]
BANK = ROOT / "assets/audio/soundbank.bin"
HTML = ROOT / "Creating Constructs.html"
IDS = [
    "music", "wind", "crickets", "placeStone", "placeWood", "placeMetal",
    "breakStone", "breakWood", "breakMetal", "click", "hover", "back", "slider",
    "musicKey", "overcastComplex", "goldenComplex", "blueComplex",
    "stepStone1", "stepStone2", "stepWood1", "stepWood2", "stepMetal1", "stepMetal2",
    "stepWater", "jump", "land", "stepStone3", "stepStone4", "stepWood3", "stepWood4",
    "stepMetal3", "stepMetal4", "stepWater2", "stepWater3", "stepWater4", "stepWater5",
    "stepWater6", "stepSoft1", "stepSoft2", "stepSoft3", "stepSoft4",
]
# SFX + ambience + Quiet Rooms (A_music). Skip retired Complex OST beds.
# Procedural Noon/Golden/Blue scores are synthesized in-page from musicKey (src/audio/mixer.inc).
KEEP = {i for i, n in enumerate(IDS) if n not in ("overcastComplex", "goldenComplex", "blueComplex")}


def parse_bank(data: bytes):
    magic, count = struct.unpack_from("<II", data, 0)
    assert magic == 0x3141554D and count == len(IDS), (magic, count)
    off = 8
    clips = []
    for i in range(count):
        (n,) = struct.unpack_from("<I", data, off)
        off += 4
        clips.append(data[off : off + n])
        off += n
    assert off == len(data)
    return clips


def main():
    clips = parse_bank(BANK.read_bytes())
    keep_ids = [IDS[i] for i in sorted(KEEP)]
    keep_blobs = [clips[i] for i in sorted(KEEP)]
    out = bytearray()
    out += struct.pack("<II", 0x3141554D, len(keep_blobs))
    for blob in keep_blobs:
        out += struct.pack("<I", len(blob))
        out += blob
    import base64

    b64 = base64.b64encode(bytes(out)).decode("ascii")
    # chunk for readability / diffs
    chunks = [b64[i : i + 120] for i in range(0, len(b64), 120)]
    pack = "const AUDIO_BANK_IDS=" + repr(keep_ids) + ";\n"
    pack += "const AUDIO_BANK_B64=[\n" + ",\n".join(f'"{c}"' for c in chunks) + "\n].join('');\n"
    html = HTML.read_text(encoding="utf-8")
    marker = "/*AUDIO_BANK_PACK*/"
    end = "/*AUDIO_BANK_PACK_END*/"
    block = f"{marker}\n{pack}{end}"
    if marker in html:
        html = re.sub(re.escape(marker) + r"[\s\S]*?" + re.escape(end), block, html, count=1)
    else:
        needle = "// Procedural audio recipes"
        if needle not in html:
            raise SystemExit("insert point missing")
        html = html.replace(needle, block + "\n" + needle, 1)
    HTML.write_text(html, encoding="utf-8")
    print(f"embedded {len(keep_blobs)} clips, raw {len(out)} bytes, b64 {len(b64)} chars")


if __name__ == "__main__":
    main()
