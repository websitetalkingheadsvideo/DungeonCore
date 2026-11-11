"""Remove hand, center and scale geode to fill frame. Output versioned ref path."""
from __future__ import annotations

import sys
from pathlib import Path

import cv2
import numpy as np
from PIL import Image

SRC = Path(r"e:\Dungeon Core\Design\Reference Images\Core\DungeonCore-small.png")
OUT = Path(r"e:\Dungeon Core\Design\Reference Images\Core\DungeonCore-small-v2-no-hand.png")
OUT_SIZE = 1024
FILL_FRAC = 0.96


def rgb_to_hsv(rgb: np.ndarray) -> np.ndarray:
    bgr = cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR)
    hsv = cv2.cvtColor(bgr, cv2.COLOR_BGR2HSV)
    return hsv


def hand_mask(rgb: np.ndarray) -> np.ndarray:
    hsv = rgb_to_hsv(rgb)
    h, s, v = hsv[:, :, 0], hsv[:, :, 1], hsv[:, :, 2]
    skin = (
        (h < 25) & (s > 25) & (s < 200) & (v > 80)
    ) | (
        (h > 160) & (h < 180) & (s > 20) & (v > 90)
    )
    # pink nail highlights
    pink = (h < 15) & (s > 40) & (v > 140)
    mask = (skin | pink).astype(np.uint8) * 255
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (7, 7))
    mask = cv2.dilate(mask, kernel, iterations=4)
    return mask


def geode_mask(rgb: np.ndarray) -> np.ndarray:
    """Stone shell + blue crystals; excludes skin and deep black."""
    hsv = rgb_to_hsv(rgb)
    h, s, v = hsv[:, :, 0], hsv[:, :, 1], hsv[:, :, 2]
    crystal = (h > 85) & (h < 140) & (s > 35) & (v > 35)
    stone = (s < 55) & (v > 28) & (v < 200)
    gray = cv2.cvtColor(rgb, cv2.COLOR_RGB2GRAY)
    bright_core = gray > 24
    subject = bright_core & (crystal | stone) & ~(hand_mask(rgb) > 0)
    mask = subject.astype(np.uint8) * 255
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (11, 11))
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel, iterations=3)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel, iterations=1)
    return mask


def subject_mask(rgb: np.ndarray) -> np.ndarray:
    """Non-background pixels (geode only)."""
    subject = geode_mask(rgb) > 0
    mask = subject.astype(np.uint8) * 255
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (9, 9))
    mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel, iterations=2)
    mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel, iterations=1)
    return mask


def inpaint_hand(rgb: np.ndarray) -> np.ndarray:
    bgr = cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR)
    mask = hand_mask(rgb)
    h, w = mask.shape
    band = np.zeros_like(mask)
    band[int(h * 0.52) :, :] = 255
    mask = cv2.bitwise_or(mask, cv2.bitwise_and(band, (geode_mask(rgb) == 0).astype(np.uint8) * 255))
    if mask.max() == 0:
        return rgb
    return cv2.cvtColor(cv2.inpaint(bgr, mask, 18, cv2.INPAINT_NS), cv2.COLOR_BGR2RGB)


def compose_framed(rgb: np.ndarray) -> Image.Image:
    cleaned = inpaint_hand(rgb)
    mask = subject_mask(cleaned)
    ys, xs = np.where(mask > 0)
    if len(xs) == 0:
        raise RuntimeError("No subject pixels found after hand removal")

    x0, x1 = int(xs.min()), int(xs.max())
    y0, y1 = int(ys.min()), int(ys.max())
    crop = cleaned[y0 : y1 + 1, x0 : x1 + 1]
    crop_mask = mask[y0 : y1 + 1, x0 : x1 + 1]

    ch, cw = crop.shape[:2]
    side = max(ch, cw)
    pad_y = (side - ch) // 2
    pad_x = (side - cw) // 2
    square = np.zeros((side, side, 3), dtype=np.uint8)
    square_mask = np.zeros((side, side), dtype=np.uint8)
    square[pad_y : pad_y + ch, pad_x : pad_x + cw] = crop
    square_mask[pad_y : pad_y + ch, pad_x : pad_x + cw] = crop_mask

    target = int(OUT_SIZE * FILL_FRAC)
    scale = target / side
    new_side = max(1, int(round(side * scale)))
    scaled = cv2.resize(square, (new_side, new_side), interpolation=cv2.INTER_LANCZOS4)
    scaled_mask = cv2.resize(square_mask, (new_side, new_side), interpolation=cv2.INTER_NEAREST)

    canvas = np.zeros((OUT_SIZE, OUT_SIZE, 3), dtype=np.uint8)
    ox = (OUT_SIZE - new_side) // 2
    oy = (OUT_SIZE - new_side) // 2
    roi = canvas[oy : oy + new_side, ox : ox + new_side]
    m = scaled_mask.astype(bool)
    roi[m] = scaled[m]
    # Drop any residual flesh-tone pixels
    flesh = hand_mask(canvas) > 0
    canvas[flesh] = (0, 0, 0)

    # soft vignette on background only
    yy, xx = np.ogrid[:OUT_SIZE, :OUT_SIZE]
    cx, cy = OUT_SIZE / 2, OUT_SIZE / 2
    dist = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2) / (OUT_SIZE * 0.55)
    vignette = np.clip(dist**2 * 0.35, 0, 1)
    bg = canvas.max(axis=2) < 8
    for c in range(3):
        channel = canvas[:, :, c]
        channel[bg] = (channel[bg] * (1 - vignette[bg])).astype(np.uint8)
        canvas[:, :, c] = channel

    return Image.fromarray(canvas)


def main() -> int:
    if not SRC.is_file():
        print(f"Missing source: {SRC}", file=sys.stderr)
        return 1
    rgb = np.array(Image.open(SRC).convert("RGB"))
    result = compose_framed(rgb)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    result.save(OUT, format="PNG", optimize=True)
    print(f"Wrote {OUT} ({result.size[0]}x{result.size[1]})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
