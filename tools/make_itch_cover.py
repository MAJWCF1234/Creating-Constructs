"""Procedural itch.io cover from real game screenshots. Output: 630x500."""
from __future__ import annotations

import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageEnhance, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "creating-constructs-itch-cover.png"
W, H = 630, 500

# Prefer the strongest real captures (garden / overgrown / close).
SOURCES = [
    ROOT / "output" / "nature" / "garden.png",
    ROOT / "output" / "overgrown-construct" / "overview.png",
    ROOT / "output" / "nature" / "close.png",
    ROOT / "output" / "performance" / "final-close-day.png",
]


def cover_crop(im: Image.Image, tw: int, th: int) -> Image.Image:
    """Scale to cover target size, then center-crop (slightly biased upward)."""
    sw, sh = im.size
    scale = max(tw / sw, th / sh)
    nw, nh = max(1, int(round(sw * scale))), max(1, int(round(sh * scale)))
    resized = im.resize((nw, nh), Image.Resampling.LANCZOS)
    left = (nw - tw) // 2
    top = max(0, (nh - th) // 2 - th // 20)  # slight upward bias for title room
    return resized.crop((left, top, left + tw, top + th))


def film_grain(im: Image.Image, amount: float = 0.08, seed: int = 42) -> Image.Image:
    """Subtle procedural grain matching the sandbox look."""
    rng = random.Random(seed)
    grain = Image.new("L", im.size)
    px = grain.load()
    for y in range(im.size[1]):
        for x in range(im.size[0]):
            px[x, y] = rng.randint(0, 255)
    grain = grain.convert("RGB")
    return Image.blend(im.convert("RGB"), grain, amount)


def vignette(im: Image.Image, strength: float = 0.42) -> Image.Image:
    base = im.convert("RGBA")
    overlay = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    cx, cy = base.size[0] / 2, base.size[1] / 2
    max_r = math.hypot(cx, cy)
    steps = 48
    for i in range(steps):
        t = i / (steps - 1)
        a = int(255 * strength * (t ** 2.2))
        r = max_r * (0.55 + 0.55 * t)
        bbox = [cx - r, cy - r, cx + r, cy + r]
        draw.ellipse(bbox, outline=(0, 0, 0, a), width=max(2, int(max_r / steps) + 1))
    # Soften banding
    overlay = overlay.filter(ImageFilter.GaussianBlur(radius=18))
    return Image.alpha_composite(base, overlay).convert("RGB")


def pick_font(size: int) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
    candidates = [
        r"C:\Windows\Fonts\segoeuib.ttf",
        r"C:\Windows\Fonts\arialbd.ttf",
        r"C:\Windows\Fonts\calibrib.ttf",
        r"C:\Windows\Fonts\consolab.ttf",
    ]
    for path in candidates:
        try:
            return ImageFont.truetype(path, size)
        except OSError:
            continue
    return ImageFont.load_default()


def draw_title(im: Image.Image, text: str = "Creating Constructs") -> Image.Image:
    layer = im.convert("RGBA")
    # Soft dark band behind title for readability on light grey void
    band = Image.new("RGBA", layer.size, (0, 0, 0, 0))
    bd = ImageDraw.Draw(band)
    bd.rectangle((0, 0, layer.size[0], 118), fill=(18, 18, 20, 150))
    band = band.filter(ImageFilter.GaussianBlur(radius=22))
    layer = Image.alpha_composite(layer, band)

    draw = ImageDraw.Draw(layer)
    font = pick_font(46)
    # Measure
    bbox = draw.textbbox((0, 0), text, font=font)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    x = (layer.size[0] - tw) // 2
    y = 28

    # Soft shadow
    for ox, oy, a in ((2, 3, 160), (0, 2, 90), (1, 1, 70)):
        draw.text((x + ox, y + oy), text, font=font, fill=(0, 0, 0, a))
    # Cream title matching UI
    draw.text((x, y), text, font=font, fill=(236, 232, 220, 255))
    return layer.convert("RGB")


def main() -> None:
    src = next((p for p in SOURCES if p.exists()), None)
    if src is None:
        raise SystemExit("No source screenshots found.")

    im = Image.open(src).convert("RGB")
    frame = cover_crop(im, W, H)
    # Mild contrast punch so the construct reads at thumbnail size
    frame = ImageEnhance.Contrast(frame).enhance(1.12)
    frame = ImageEnhance.Color(frame).enhance(1.08)
    frame = vignette(frame, strength=0.38)
    frame = film_grain(frame, amount=0.045, seed=20261020)
    frame = draw_title(frame)

    frame.save(OUT, "PNG", optimize=True)
    # Also a JPEG for itch upload convenience
    jpg = OUT.with_suffix(".jpg")
    frame.save(jpg, "JPEG", quality=92, optimize=True)
    print(f"wrote {OUT} ({W}x{H}) from {src.relative_to(ROOT)}")
    print(f"wrote {jpg}")


if __name__ == "__main__":
    main()
