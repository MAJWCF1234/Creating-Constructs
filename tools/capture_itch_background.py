"""Load Overgrown Construct in the HTML sandbox and capture an itch background."""
from __future__ import annotations

import http.server
import json
import socketserver
import threading
from pathlib import Path

from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parents[1]
HTML = ROOT / "Creating Constructs.html"
WORLD = ROOT / "worlds" / "Overgrown Construct.json"
OUT_DIR = ROOT / "output" / "itch"
RAW = OUT_DIR / "overgrown-background-raw.png"
PORT = 8765


def serve() -> socketserver.TCPServer:
    root = str(ROOT)

    class Handler(http.server.SimpleHTTPRequestHandler):
        def __init__(self, *a, **k):
            super().__init__(*a, directory=root, **k)

        def log_message(self, *_):
            pass

    httpd = socketserver.TCPServer(("127.0.0.1", PORT), Handler)
    threading.Thread(target=httpd.serve_forever, daemon=True).start()
    return httpd


def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    world = json.loads(WORLD.read_text(encoding="utf-8"))
    httpd = serve()
    try:
        with sync_playwright() as p:
            browser = p.chromium.launch(headless=True)
            page = browser.new_page(viewport={"width": 1600, "height": 1000}, device_scale_factor=1)
            page.goto(f"http://127.0.0.1:{PORT}/{HTML.name}", wait_until="domcontentloaded", timeout=120000)
            # Seed autosave before the boot path finishes reading it — clear and reload.
            page.evaluate("(w) => localStorage.setItem('creating-constructs', JSON.stringify(w))", world)
            page.reload(wait_until="networkidle", timeout=120000)
            page.wait_for_timeout(1800)

            page.select_option("#mood", "Blue hour")
            page.wait_for_timeout(500)
            page.keyboard.press("h")  # Hide UI — button lives in a closed <details>
            page.wait_for_timeout(300)

            canvas = page.locator("canvas").first
            box = canvas.bounding_box()
            assert box
            cx, cy = box["x"] + box["width"] / 2, box["y"] + box["height"] / 2

            for _ in range(20):
                page.mouse.wheel(0, 200)
                page.wait_for_timeout(35)

            page.mouse.move(cx, cy)
            page.mouse.down(button="right")
            page.mouse.move(cx + 260, cy + 90, steps=28)
            page.mouse.up(button="right")
            page.wait_for_timeout(200)

            page.mouse.move(cx, cy)
            page.mouse.down(button="middle")
            page.mouse.move(cx - 30, cy + 80, steps=14)
            page.mouse.up(button="middle")
            page.wait_for_timeout(2000)

            page.screenshot(path=str(RAW), full_page=False)
            browser.close()
    finally:
        httpd.shutdown()
    print(f"wrote {RAW}")


if __name__ == "__main__":
    main()
