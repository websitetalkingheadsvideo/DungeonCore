"""Convert generated store icons: remove baked checkerboard, resize 512, save PNG."""
from __future__ import annotations

import os
from collections import deque

from PIL import Image

SRC_DIR = r"C:\Users\paris\.cursor\projects\e-Dungeon-Core\assets"
OUT_DIR = r"E:\Dungeon Core\Design\Reference Images\icons"
NAMES = [
    "monster_generator",
    "mana_generator",
    "prism_beam_trap",
    "resonance_plate_trap",
    "gravity_well_trap",
    "runaway_cart_trap",
    "loot_bait",
    "unshackled_golem",
    "shard_sentinel",
]
TARGET_SIZE = 512


def is_checkerboard_bg(rgb: tuple[int, int, int]) -> bool:
    r, g, b = rgb
    if abs(r - g) > 12 or abs(g - b) > 12 or abs(r - b) > 12:
        return False
    avg = (r + g + b) / 3.0
    return avg >= 198.0


def flood_transparent(im: Image.Image) -> Image.Image:
    rgb = im.convert("RGB")
    w, h = rgb.size
    px = rgb.load()
    visited = [[False] * w for _ in range(h)]
    transparent = [[False] * w for _ in range(h)]
    q: deque[tuple[int, int]] = deque()

    def try_seed(x: int, y: int) -> None:
        if not is_checkerboard_bg(px[x, y]):
            return
        if visited[y][x]:
            return
        visited[y][x] = True
        transparent[y][x] = True
        q.append((x, y))

    for x in range(w):
        try_seed(x, 0)
        try_seed(x, h - 1)
    for y in range(h):
        try_seed(0, y)
        try_seed(w - 1, y)

    while q:
        x, y = q.popleft()
        for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if nx < 0 or ny < 0 or nx >= w or ny >= h:
                continue
            if visited[ny][nx]:
                continue
            if not is_checkerboard_bg(px[nx, ny]):
                continue
            visited[ny][nx] = True
            transparent[ny][nx] = True
            q.append((nx, ny))

    rgba = rgb.convert("RGBA")
    out_px = rgba.load()
    for y in range(h):
        for x in range(w):
            if transparent[y][x]:
                out_px[x, y] = (0, 0, 0, 0)
    return rgba


def has_transparency(im: Image.Image) -> bool:
    if im.mode != "RGBA":
        return False
    alpha = im.getchannel("A")
    extrema = alpha.getextrema()
    return extrema[0] < 255


def main() -> None:
    os.makedirs(OUT_DIR, exist_ok=True)
    report: list[str] = []
    for name in NAMES:
        src_path = os.path.join(SRC_DIR, f"{name}.jpg")
        out_path = os.path.join(OUT_DIR, f"{name}.png")
        im = Image.open(src_path)
        cut = flood_transparent(im)
        cut = cut.resize((TARGET_SIZE, TARGET_SIZE), Image.Resampling.LANCZOS)
        cut.save(out_path, "PNG")
        verify = Image.open(out_path)
        ok_size = verify.size == (TARGET_SIZE, TARGET_SIZE)
        ok_alpha = has_transparency(verify)
        report.append(
            f"{out_path} size={verify.size} transparent={ok_alpha} ok={ok_size and ok_alpha}"
        )
    print("\n".join(report))


if __name__ == "__main__":
    main()
