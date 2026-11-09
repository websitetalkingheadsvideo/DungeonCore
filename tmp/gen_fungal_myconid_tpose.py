"""Repose fungal myconid reference into T-pose via fal nano-banana-2."""

from __future__ import annotations

import json
import os
import urllib.request
from pathlib import Path

import fal_client

ROOT = Path(r"E:\Dungeon Core")
REF = Path(
    r"C:\Users\paris\.cursor\projects\e-Dungeon-Core\assets"
    r"\image-c75a2910-ed23-4c5f-9535-8ed6a40bd4c9.png"
)
OUT_DIR = ROOT / "Design" / "Reference Images" / "Fungal"
OUT = OUT_DIR / "Fungal-Myconid-Golem-Tpose-v1.png"
MANIFEST = ROOT / "tmp" / "Fungal-Myconid-Golem-Tpose-v1_manifest.json"

PROMPT = """
Same mushroom myconid golem creature as the reference image — identity lock.
Keep: gnarled dark bark-wood body, dense brown mushroom caps clustered on shoulders as head,
white speckles on caps, glowing cyan-blue bioluminescence on mushroom gills and bark cracks,
green moss patches on shoulders and limbs, long muscular arms, three-fingered wooden hands,
thick trunk legs, root-like feet with toe roots.

Repose ONLY: strict anatomical T-pose for game rigging.
Standing straight upright, full body front view, feet planted shoulder width,
both arms extended perfectly horizontal at shoulder height, palms facing forward toward camera,
fingers relaxed together, empty hands, legs straight.
Plain white background, studio character reference, stylized fantasy realism, no text, no watermark.
""".strip()


def load_fal_key() -> None:
    for line in (ROOT / ".env").read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line.startswith("FAL_KEY="):
            os.environ["FAL_KEY"] = line.split("=", 1)[1].strip()
            return
    raise RuntimeError("FAL_KEY missing in .env")


def main() -> None:
    load_fal_key()
    if not REF.is_file():
        raise FileNotFoundError(REF)

    image_url = fal_client.upload_file(str(REF))
    result = fal_client.subscribe(
        "fal-ai/nano-banana-2",
        arguments={
            "prompt": PROMPT,
            "image_urls": [image_url],
            "aspect_ratio": "9:16",
            "resolution": "2K",
            "num_images": 1,
            "output_format": "png",
            "safety_tolerance": "4",
        },
        client_timeout=600,
    )
    images = result.get("images") or []
    if not images or not images[0].get("url"):
        raise RuntimeError(json.dumps(result)[:800])

    url = images[0]["url"]
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    req = urllib.request.Request(url, headers={"User-Agent": "DungeonCore-fal-client/1.0"})
    with urllib.request.urlopen(req, timeout=120) as resp:
        OUT.write_bytes(resp.read())

    MANIFEST.write_text(
        json.dumps(
            {
                "output": str(OUT),
                "model": "fal-ai/nano-banana-2",
                "source": str(REF),
                "image_url": url,
                "seed": images[0].get("seed"),
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    print(f"Wrote {OUT} ({OUT.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
