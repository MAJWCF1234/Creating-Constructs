"""Turn the Overgrown Construct overview capture into an itch.io page background."""
from __future__ import annotations

import math
import random
from pathlib import Path

from PIL import Image, ImageEnhance, ImageFilter, ImageOps

ROOT = Path(__file__).resolve().parents[1]
RAW_BMP = Path.home() / "AppData/Local/Temp/creating-constructs-vines.bmp"
FALLBACK = ROOT / "output" / "itch" / "overgrown-background-raw.png"
OUT_DIR = ROOT / "output" / "itch"
OUT_PNG = ROOT / "creating-constructs-itch-background.png"
OUT_JPG = ROOT / "creating-constructs-itch-background.jpg"
# itch backgrounds are soft full-bleed; 1920x1080 reads well on most pages.
W, H = 1920, 1080


def load_source() -> Image.Image:
    if RAW_BMP.exists():
        im = Image.open(RAW_BMP)
        # Vulkan readback BMP is often bottom-up BGRA — Pillow handles BMP;
        # if it looks upside-down, flip.
        return im.convert("RGB")
    if FALLBACK.exists():
        return Image.open(FALLBACK).convert("RGB")
    raise SystemExit("No capture found. Run native --capture first.")


def cover(im: Image.Image, tw: int, th: int) -> Image.Image:
    sw, sh = im.size
    scale = max(tw / sw, th / sh) * 1.04  # slight overscan so edges soft-crop
    nw, nh = max(1, int(round(sw * scale))), max(1, int(round(sh * scale)))
    r = im.resize((nw, nh), Image.Resampling.LANCZOS)
    left, top = (nw - tw) // 2, (nh - th) // 2
    return r.crop((left, top, left + tw, top + th))


def soft_vignette(im: Image.Image, strength: float = 0.55) -> Image.Image:
    base = im.convert("RGBA")
    # Build a small radial mask, then upscale — much faster than per-pixel at full res.
    sw, sh = 160, 90
    mask = Image.new("L", (sw, sh), 0)
    mp = mask.load()
    cx, cy = (sw - 1) / 2, (sh - 1) / 2
    max_r = math.hypot(cx, cy)
    for y in range(sh):
        for x in range(sw):
            t = math.hypot(x - cx, y - cy) / max_r
            mp[x, y] = int(255 * strength * max(0.0, (t - 0.32) / 0.68) ** 1.55)
    mask = mask.resize(base.size, Image.Resampling.LANCZOS).filter(ImageFilter.GaussianBlur(radius=24))
    overlay = Image.new("RGBA", base.size, (12, 14, 18, 255))
    overlay.putalpha(mask)
    return Image.alpha_composite(base, overlay).convert("RGB")


def grain(im: Image.Image, amount: float = 0.04, seed: int = 77) -> Image.Image:
    rng = random.Random(seed)
    # Small noise tile, tiled + slight blur.
    tile = Image.new("L", (128, 128))
    tp = tile.load()
    for y in range(128):
        for x in range(128):
            tp[x, y] = rng.randint(0, 255)
    g = Image.new("L", im.size)
    for y in range(0, im.size[1], 128):
        for x in range(0, im.size[0], 128):
            g.paste(tile, (x, y))
    g = g.filter(ImageFilter.GaussianBlur(radius=0.6))
    return Image.blend(im, g.convert("RGB"), amount)


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    src = load_source()
    frame = cover(src, W, H)
    # Atmosphere: cooler, slightly darker so page text stays legible.
    frame = ImageEnhance.Color(frame).enhance(0.92)
    frame = ImageEnhance.Brightness(frame).enhance(0.72)
    frame = ImageEnhance.Contrast(frame).enhance(1.08)
    frame = soft_vignette(frame, strength=0.58)
    frame = grain(frame, amount=0.035)
    # Extra top/bottom wash for itch header/footer readability
    wash = Image.new("RGBA", frame.size, (0, 0, 0, 0))
    wp = wash.load()
    for y in range(frame.size[1]):
        top = max(0.0, 1.0 - y / 140.0)
        bot = max(0.0, 1.0 - (frame.size[1] - 1 - y) / 180.0)
        a = int(255 * 0.42 * max(top, bot))
        if a:
            for x in range(frame.size[0]):
                wp[x, y] = (10, 12, 16, a)
    frame = Image.alpha_composite(frame.convert("RGBA"), wash).convert("RGB")

    preview = OUT_DIR / "overgrown-background-raw.png"
    src.save(preview)
    frame.save(OUT_PNG, "PNG", optimize=True)
    frame.save(OUT_JPG, "JPEG", quality=90, optimize=True)
    print(f"wrote {OUT_PNG} ({W}x{H})")
    print(f"wrote {OUT_JPG}")


if __name__ == "__main__":
    main()
