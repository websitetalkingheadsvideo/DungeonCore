"""Post-process a generated store icon JPG into 512x512 RGBA PNG for HUD."""
from __future__ import annotations

import argparse
from collections import deque
from pathlib import Path

from PIL import Image

TARGET_SIZE = 512


def center_crop_square(im: Image.Image) -> Image.Image:
    """Center-crop to 1:1 when Firefly hides aspect ratio (e.g. 1408×768)."""
    w, h = im.size
    if w == h:
        return im
    side = min(w, h)
    left = (w - side) // 2
    top = (h - side) // 2
    return im.crop((left, top, left + side, top + side))


def is_neutral_light(rgb: tuple[int, int, int], min_channel: int) -> bool:
    r, g, b = rgb
    if abs(r - g) > 15 or abs(g - b) > 15 or abs(r - b) > 15:
        return False
    return min(r, g, b) >= min_channel


def is_white_backdrop(rgb: tuple[int, int, int], white_threshold: int) -> bool:
    return is_neutral_light(rgb, white_threshold)


def is_checkerboard_bg(rgb: tuple[int, int, int]) -> bool:
    return is_neutral_light(rgb, 198)


def flood_mask(
    px,
    w: int,
    h: int,
    predicate,
) -> list[list[bool]]:
    visited = [[False] * w for _ in range(h)]
    transparent = [[False] * w for _ in range(h)]
    q: deque[tuple[int, int]] = deque()

    def try_seed(x: int, y: int) -> None:
        if visited[y][x] or not predicate(px[x, y]):
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
            if visited[ny][nx] or not predicate(px[nx, ny]):
                continue
            visited[ny][nx] = True
            transparent[ny][nx] = True
            q.append((nx, ny))

    return transparent


def to_transparent_png(
    im: Image.Image,
    backdrop: str,
    white_threshold: int,
    black_threshold: int,
) -> Image.Image:
    rgb = center_crop_square(im).convert("RGB")
    w, h = rgb.size
    px = rgb.load()

    remove = [[False] * w for _ in range(h)]

    if backdrop == "white":

        def is_backdrop_white(pixel: tuple[int, int, int]) -> bool:
            return is_white_backdrop(pixel, white_threshold)

        white_edge = flood_mask(px, w, h, is_backdrop_white)
        for y in range(h):
            for x in range(w):
                remove[y][x] = white_edge[y][x]
    else:
        checker = flood_mask(px, w, h, is_checkerboard_bg)

        def is_near_pure_black(pixel: tuple[int, int, int]) -> bool:
            r, g, b = pixel
            return max(r, g, b) <= black_threshold

        black_edge = flood_mask(px, w, h, is_near_pure_black)
        for y in range(h):
            for x in range(w):
                remove[y][x] = checker[y][x] or black_edge[y][x]

    rgba = rgb.convert("RGBA")
    out_px = rgba.load()
    for y in range(h):
        for x in range(w):
            if remove[y][x]:
                out_px[x, y] = (0, 0, 0, 0)

    if backdrop == "white":

        def is_gap_white(pixel: tuple[int, int, int]) -> bool:
            return is_white_backdrop(pixel, white_threshold)

        grow_backdrop_touching_alpha(out_px, w, h, is_gap_white, max_passes=512)

    sized = resize_rgba_premultiplied(rgba, TARGET_SIZE)

    if backdrop == "white":
        sw, sh = sized.size
        s_px = sized.load()

        def is_fringe_white(pixel: tuple[int, int, int]) -> bool:
            return is_white_backdrop(pixel, 215)

        grow_backdrop_touching_alpha(s_px, sw, sh, is_fringe_white, max_passes=64)
        grow_void_into_transparent(s_px, sw, sh, void_threshold=40, max_passes=96)

    return sized


def resize_rgba_premultiplied(rgba: Image.Image, target: int) -> Image.Image:
    """Resize without white matte bleeding into transparent edges (straight-alpha safe)."""
    w, h = rgba.size
    src = rgba.load()
    premul = Image.new("RGBA", (w, h))
    ppx = premul.load()
    for y in range(h):
        for x in range(w):
            r, g, b, a = src[x, y]
            if a == 0:
                ppx[x, y] = (0, 0, 0, 0)
                continue
            scale = a / 255.0
            ppx[x, y] = (int(r * scale), int(g * scale), int(b * scale), a)

    small = premul.resize((target, target), Image.Resampling.LANCZOS)
    sw, sh = small.size
    spx = small.load()
    out = Image.new("RGBA", (target, target))
    opx = out.load()
    for y in range(sh):
        for x in range(sw):
            r, g, b, a = spx[x, y]
            if a == 0:
                opx[x, y] = (0, 0, 0, 0)
                continue
            scale = 255.0 / a
            opx[x, y] = (
                min(255, int(r * scale)),
                min(255, int(g * scale)),
                min(255, int(b * scale)),
                a,
            )
    return out


def grow_backdrop_touching_alpha(
    px,
    w: int,
    h: int,
    is_backdrop,
    max_passes: int,
) -> None:
    """Remove interior backdrop islands (e.g. white between legs) touching keyed alpha."""
    for _ in range(max_passes):
        to_clear: list[tuple[int, int]] = []
        for y in range(h):
            for x in range(w):
                r, g, b, a = px[x, y]
                if a == 0 or not is_backdrop((r, g, b)):
                    continue
                for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                    if nx < 0 or ny < 0 or nx >= w or ny >= h:
                        continue
                    if px[nx, ny][3] == 0:
                        to_clear.append((x, y))
                        break
        if not to_clear:
            return
        for x, y in to_clear:
            px[x, y] = (0, 0, 0, 0)


def grow_void_into_transparent(
    px,
    w: int,
    h: int,
    void_threshold: int,
    max_passes: int,
) -> None:
    """Clear near-black backdrop pixels touching transparent (leg gaps, under shelf)."""
    for _ in range(max_passes):
        to_clear: list[tuple[int, int]] = []
        for y in range(h):
            for x in range(w):
                r, g, b, a = px[x, y]
                if a == 0 or max(r, g, b) > void_threshold:
                    continue
                for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                    if nx < 0 or ny < 0 or nx >= w or ny >= h:
                        continue
                    if px[nx, ny][3] == 0:
                        to_clear.append((x, y))
                        break
        if not to_clear:
            return
        for x, y in to_clear:
            px[x, y] = (0, 0, 0, 0)


def firefly_rgba_to_icon(im: Image.Image, target: int = TARGET_SIZE, fill: float = 0.80) -> Image.Image:
    """Fit a Firefly remove-background PNG into a square HUD icon."""
    rgba = center_crop_square(im).convert("RGBA")
    alpha = rgba.getchannel("A")
    bbox = alpha.getbbox()
    if bbox is None:
        raise ValueError("source has no opaque pixels")

    full_w, full_h = rgba.size
    x0, y0, x1, y1 = bbox
    pad_x = max(4, int((x1 - x0) * 0.04))
    pad_y = max(4, int((y1 - y0) * 0.04))
    x0 = max(0, x0 - pad_x)
    y0 = max(0, y0 - pad_y)
    x1 = min(full_w, x1 + pad_x)
    y1 = min(full_h, y1 + pad_y)
    crop = rgba.crop((x0, y0, x1, y1))
    cw, ch = crop.size
    side = max(cw, ch)
    square = Image.new("RGBA", (side, side), (0, 0, 0, 0))
    square.paste(crop, ((side - cw) // 2, (side - ch) // 2))
    inner = max(1, int(target * fill))
    fitted = resize_rgba_premultiplied(square, inner)
    out = Image.new("RGBA", (target, target), (0, 0, 0, 0))
    offset = (target - inner) // 2
    out.paste(fitted, (offset, offset))
    return out


def next_versioned_path(out_dir: Path, basename: str) -> tuple[Path, Path]:
    """Return (canonical basename.png, new basename-vN.png). Never overwrites v paths."""
    canonical = out_dir / f"{basename}.png"
    version = 1
    while (out_dir / f"{basename}-v{version}.png").exists():
        version += 1
    return canonical, out_dir / f"{basename}-v{version}.png"


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert generated store icon JPG to HUD PNG")
    parser.add_argument("--src", required=True, type=Path, help="Source JPG or Firefly RGBA PNG")
    parser.add_argument("--basename", required=True, help="Icon basename without extension")
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("Design/Reference Images/icons"),
        help="Repo-relative icons folder",
    )
    parser.add_argument(
        "--backdrop",
        choices=("white", "legacy"),
        default="white",
        help="Backdrop key: white = flat white JPG (default); legacy = checker + edge black",
    )
    parser.add_argument(
        "--white-threshold",
        type=int,
        default=235,
        help="Min R,G,B for edge-connected white removal when --backdrop white",
    )
    parser.add_argument(
        "--black-threshold",
        type=int,
        default=10,
        help="Legacy only: max RGB for edge-connected black removal (default 10)",
    )
    parser.add_argument(
        "--from-firefly",
        action="store_true",
        help="Source is Firefly remove-background RGBA; fit to 512 (no backdrop key)",
    )
    parser.add_argument(
        "--fill",
        type=float,
        default=0.80,
        help="Firefly only: max object size as fraction of 512 (default 0.80)",
    )
    parser.add_argument(
        "--write-canonical",
        action="store_true",
        help="Also write basename.png (after approval only)",
    )
    args = parser.parse_args()

    out_dir = args.out_dir
    out_dir.mkdir(parents=True, exist_ok=True)

    im = Image.open(args.src)
    if args.from_firefly:
        cut = firefly_rgba_to_icon(im, TARGET_SIZE, args.fill)
    else:
        cut = to_transparent_png(
            im,
            args.backdrop,
            args.white_threshold,
            args.black_threshold,
        )

    canonical, versioned = next_versioned_path(out_dir, args.basename)
    cut.save(versioned, "PNG")

    if args.write_canonical:
        cut.save(canonical, "PNG")

    alpha = cut.getchannel("A").getextrema()
    print(f"versioned={versioned}")
    print(f"canonical={canonical if args.write_canonical else '(skipped)'}")
    print(f"size={cut.size} alpha_range={alpha}")


if __name__ == "__main__":
    main()