"""Generate tangent-space normal map from albedo (luminance height + Sobel)."""
from __future__ import annotations

import sys
from pathlib import Path

import numpy as np
from PIL import Image


def albedo_to_normal(
    rgba: np.ndarray,
    strength: float,
    black_threshold: int,
) -> np.ndarray:
    rgb = rgba[..., :3].astype(np.float32)
    lum = (
        0.2126 * rgb[..., 0] + 0.7152 * rgb[..., 1] + 0.0722 * rgb[..., 2]
    ) / 255.0
    bg = np.all(rgb < black_threshold, axis=-1)

    h, w = lum.shape
    padded = np.pad(lum, 1, mode="edge")
    dz_dx = (padded[1:-1, 2:] - padded[1:-1, :-2]) * 0.5
    dz_dy = (padded[2:, 1:-1] - padded[:-2, 1:-1]) * 0.5

    nx = -dz_dx * strength
    ny = -dz_dy * strength
    nz = np.ones_like(nx, dtype=np.float32)

    norm = np.sqrt(nx * nx + ny * ny + nz * nz)
    nx /= norm
    ny /= norm
    nz /= norm

    out = np.zeros((h, w, 3), dtype=np.uint8)
    out[..., 0] = np.clip((nx * 0.5 + 0.5) * 255.0, 0, 255).astype(np.uint8)
    out[..., 1] = np.clip((ny * 0.5 + 0.5) * 255.0, 0, 255).astype(np.uint8)
    out[..., 2] = np.clip((nz * 0.5 + 0.5) * 255.0, 0, 255).astype(np.uint8)

    flat = np.array([128, 128, 255], dtype=np.uint8)
    out[bg] = flat
    return out


def main() -> None:
    src = Path(r"e:\Dungeon Core\imports\hi3d\Orcs\textures\Image_0.png")
    dst = Path(r"e:\Dungeon Core\imports\hi3d\Orcs\textures\Image_0-Normal-v1.png")
    strength = float(sys.argv[1]) if len(sys.argv) > 1 else 4.0

    im = Image.open(src).convert("RGBA")
    rgba = np.array(im)
    normal_rgb = albedo_to_normal(rgba, strength=strength, black_threshold=12)
    Image.fromarray(normal_rgb, mode="RGB").save(dst, optimize=True)
    print(f"Wrote {dst} (strength={strength})")


if __name__ == "__main__":
    main()