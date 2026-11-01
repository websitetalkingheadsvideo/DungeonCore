# Hi3D upload limits (observed)

Documented MCP limit: **10 MiB** per image (`upload_input` schema).

Observed on `agent.hi3d.ai` (2026-11):

| Payload | Result |
|--------|--------|
| Full-res PNG ~5 MB (`1536×2752`) | `INTERNAL_ERROR` (not a client auth failure) |
| PNG raw **> ~3 MB** | Usually `INTERNAL_ERROR` |
| PNG raw **≤ ~2.6 MB** | OK |
| Full-res **lossless WebP** ~3.2 MB (same pixels) | OK |
| Full-res JPEG q≥85 | OK |

**Default fix for Firefly PNGs:** lossless WebP at original resolution before `upload_input`.

**Do not** downscale for quote/generate unless the user asks for a proxy; WebP preserves resolution under the server cliff.

**Agent payloads:** inline base64 in `CallDynamicTool` is fine up to ~100k chars; larger uploads use `scripts/hi3d_run.py` (reads file on disk) or encode to WebP first to stay under MCP/tool limits when possible.
