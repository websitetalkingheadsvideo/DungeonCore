"""One-shot: Dark Fairy Minion 2K ref via fal-ai/nano-banana-2."""

from __future__ import annotations

import json
import os
import urllib.request
from pathlib import Path

import fal_client

ROOT = Path(r"E:\Dungeon Core")
REF_DIR = ROOT / "Design" / "Reference Images" / "Dark Fairy"
OUT = REF_DIR / "Dark-Fairy-Minion-v1.png"
MANIFEST = ROOT / "tmp" / "Dark-Fairy-Minion-v1_manifest.json"

PROMPT = """
Create a NEW full-body game character reference of a dark celestial evil fairy MINION (combat fodder tier).
Use the three reference images only for shared franchise DNA — do NOT copy any single character verbatim.

T-pose: arms horizontal at shoulder height, palms facing slightly forward, empty hands, front view, single subject.
Plain white background, no text, no watermark.

POWER TIER (critical): visibly BETWEEN worker and elite.
- MORE than worker: fitted black leather corset over forest-green tunic, small dark metal shoulder caps and forearm bracers,
  knee-length tattered layered skirt (green + purple lace), sturdy lace-up combat boots, cleaner skin (no soot/dirt),
  compact curved dagger in hip sheath (no pickaxe, no tool belt).
- LESS than elite: NO circlet or crown, NO scale-mail pauldrons, NO glaive, NO ornate filigree armor,
  wings smaller and simpler than elite cosmic nebula wings but sharper and more colorful than worker's worn wings
  (four dragonfly wings, forest-green to purple iridescent gradient, small tears OK).

Face and hair: pale skin, heavy dark smoky eye makeup, short messy PIXIE CUT dark blue-black hair with purple highlights
(same family as worker, NOT ponytail or long hair).

Color scheme: forest green, teal accents, black leather, muted purple gems — match evil fairy line in references.
Stylized fantasy realism for 3D game character sheet, anatomically correct hands with exactly five fingers each.
""".strip()

REF_FILES = [
    REF_DIR / "Dark Fairy-Worker.png",
    REF_DIR / "Dark Fairy-Elite_v2.png",
    REF_DIR / "Fairy_Boss.png",
]


def load_fal_key() -> None:
    env_path = ROOT / ".env"
    if not env_path.is_file():
        raise FileNotFoundError(f"Missing {env_path}")
    for line in env_path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line.startswith("FAL_KEY="):
            os.environ["FAL_KEY"] = line.split("=", 1)[1].strip()
            return
    raise RuntimeError("FAL_KEY not found in .env")


def main() -> None:
    load_fal_key()
    for p in REF_FILES:
        if not p.is_file():
            raise FileNotFoundError(p)

    image_urls = [fal_client.upload_file(str(p)) for p in REF_FILES]
    print("Uploaded refs:", len(image_urls))

    result = fal_client.subscribe(
        "fal-ai/nano-banana-2",
        arguments={
            "prompt": PROMPT,
            "image_urls": image_urls,
            "aspect_ratio": "9:16",
            "resolution": "2K",
            "num_images": 1,
            "output_format": "png",
            "safety_tolerance": "4",
        },
        client_timeout=600,
    )

    images = result.get("images") or []
    if not images:
        raise RuntimeError(f"No images in result: {json.dumps(result)[:500]}")

    url = images[0].get("url")
    if not url:
        raise RuntimeError(f"Missing image url: {images[0]}")

    OUT.parent.mkdir(parents=True, exist_ok=True)
    req = urllib.request.Request(url, headers={"User-Agent": "DungeonCore-fal-client/1.0"})
    with urllib.request.urlopen(req, timeout=120) as resp:
        OUT.write_bytes(resp.read())

    manifest = {
        "output": str(OUT),
        "model": "fal-ai/nano-banana-2",
        "resolution": "2K",
        "aspect_ratio": "9:16",
        "refs": [str(p) for p in REF_FILES],
        "image_url": url,
        "request_id": result.get("request_id"),
        "seed": images[0].get("seed"),
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"Wrote {OUT} ({OUT.stat().st_size} bytes)")
    print(f"Manifest {MANIFEST}")


if __name__ == "__main__":
    main()
