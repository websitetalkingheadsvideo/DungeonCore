# Skeleton Hi3D import

**Canonical mesh:** `Skeleton_v2.glb` (~**299k** tris) — Hi3D web **Advanced → 300,000 faces** (lowest UI setting). Ref `Skeleton.png`. Preview: `Skeleton_v2.jpg`.

| File | Tris | Notes |
| --- | --- | --- |
| **`Skeleton_v2.glb`** | ~299,270 | **Use this** for CC / Unreal |
| `Skeleton_v1.ccProject` | — | CC save (if present) |
| `Skeleton.glb` | ~2,000,000 | Archive — MCP job `b5c8a785…` (no `face` param) |

Manifest: `skeleton_v2_hi3d_manifest.json`. MCP archive: `skeleton_hi3d_manifest.json`.

## Face count

- **Cursor MCP** cannot set `face` (defaults ~2M). Use **hi3d.ai in Chrome/Edge** → Advanced, or Open API / `hi3d_open_api_run.py` with `HI3D_ACCESS_KEY` / `HI3D_SECRET_KEY`.
- Web UI **minimum** used here: **300,000** (not 100k).

**Google login white screen:** use Chrome/Edge, not Cursor’s embedded browser.

Ref: [Create Task API — `face`](https://docs.hi3d.ai/en/api/api-reference/list/create-task)
