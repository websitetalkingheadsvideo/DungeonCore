---
name: hi3d-image-to-3d
description: Generate and download GLB 3D models from reference images via Hi3D MCP (user-hi3d) or the local OAuth CLI. Use when the user wants image-to-3D, Hi3D, hi3d.ai, character mesh export, or to pull a completed task GLB into imports/hi3d/.
---

# Hi3D image-to-3D (generate + download)

MCP server: **`user-hi3d`** → `https://agent.hi3d.ai/mcp` (OAuth in Cursor Settings → MCP → hi3d).

**Costs points.** Always **`quote_generation`** first, show **`estimated_points`** and **`can_submit`**, then **`create_generation`** only after the user explicitly confirms spend (`confirmed: true`).

## Output layout (Dungeon Core)

```
imports/hi3d/<slug>/
  model.glb                    # or <slug>.glb via --glb-name
  <slug>_hi3d_manifest.json    # task_id, prompt, source_image, model_page_url, points
```

After download, next step is usually **`fab-asset-migration`** (GLB → Unreal) or manual import — not covered here.

## Workflow checklist

```
- [ ] 1. Confirm MCP connected (user-hi3d ready) or Windows CLI path (Cursor logged into Hi3D)
- [ ] 2. Prepare reference image (see references/upload-limits.md)
- [ ] 3. get_balance (optional) + quote_generation — do not re-quote identical params
- [ ] 4. User confirms spend → create_generation(quote_id, confirmed: true) once per quote
- [ ] 5. Poll get_task until status succeeded (respect next_poll_after)
- [ ] 6. get_task_outputs → download original GLB (not preview) + write manifest
- [ ] 7. Share model_page_url; grep log / verify file size on disk
```

## Path A — Agent in Cursor (small/medium uploads)

1. **`GetDynamicTools`** / **`CallDynamicTool`** on namespace **`user-hi3d`**.
2. **`upload_input`**: `image_base64` + `mime_type` (`image/png` | `image/jpeg` | `image/webp`). Returns **`file_token`** (24h). Do **not** pass local paths or public URLs in tool args.
3. Large Firefly PNGs: convert to **lossless WebP** at full resolution before upload (see upload-limits). If inline base64 is too large for the tool, use Path B.
4. **`quote_generation`**: `prompt`, `file_tokens`, defaults — `version: v3.0`, `model_type: high_quality`, `generation_type: model_with_texture`, `license: private`, `pbr: true`. **`face`** (100000–5000000) = web UI **Advanced → face count** / OpenAPI `face`; pass via `hi3d_run.py --face` (forwarded to MCP; **currently ignored by `agent.hi3d.ai` — verify `parameters.face` in quote response**). For prior Hi3D 2D tasks use **`source_images`** (not stale `file_tokens`).
5. Stop and show quote. On confirm → **`create_generation`** with `{ "quote_id": "...", "confirmed": true }`.
6. **`get_task`** loop until `succeeded` | `failed`.
7. **`get_task_outputs`**: prefer asset URL containing **`/target/model/original/`** and format **GLB**. Download with shell (`urllib`, `curl`, or **`hi3d_run.py download`**).

If namespace needs auth: **`mcp_auth`** on **`user-hi3d`**, then retry.

## Path B — CLI (large files, full pipeline, download)

Requires **Windows**, **same user session as Cursor**, Hi3D OAuth stored in Cursor (`Settings → MCP → hi3d → Login`). Scripts read encrypted tokens from Cursor’s state DB (DPAPI + Chromium `Local State`) — not for committing or sharing.

**Dependencies** (once per machine):

```powershell
pip install cryptography pillow
```

Run from repo root; **`PYTHONPATH`** must include the scripts folder:

```powershell
$env:PYTHONPATH = "E:\Dungeon Core\.cursor\skills\hi3d-image-to-3d\scripts"
cd "E:\Dungeon Core\.cursor\skills\hi3d-image-to-3d\scripts"

# Quote only (upload + price, no spend)
python hi3d_run.py quote `
  --image "E:\Dungeon Core\images\Fairy_Boss.png" `
  --prompt "T-pose dark celestial fairy boss, game character reference, ..."

# Spend + poll + download (requires explicit flag)
python hi3d_run.py run `
  --image "E:\Dungeon Core\images\Fairy_Boss.png" `
  --prompt "..." `
  --slug fairy-boss `
  --out-dir "E:\Dungeon Core\imports\hi3d\fairy-boss" `
  --glb-name Fairy_Boss.glb `
  --confirm-spend

# Recover an existing succeeded task
python hi3d_run.py download `
  --task-id "<task_id>" `
  --slug fairy-boss `
  --out-dir "E:\Dungeon Core\imports\hi3d\fairy-boss" `
  --prompt "..." `
  --source-image "E:\Dungeon Core\images\Fairy_Boss.webp"
```

Override Cursor paths if needed: **`CURSOR_STATE_DB`**, **`CURSOR_LOCAL_STATE`**.

Subcommands: **`quote`**, **`generate`** (quote_id only), **`poll`**, **`download`**, **`run`**.

## Prompts

Character prompts should match **`Design/Monsters.md`** and creature docs (T-pose, clear silhouette, no busy background). See **`references/prompts.md`**.

## Rules

| Rule | Why |
|------|-----|
| **`face` for Advanced poly count** | Open API / web UI **Advanced → face count** = field **`face`** (100k–5M). Cursor MCP omits it — use `hi3d_open_api_run.py` + `HI3D_ACCESS_KEY`/`HI3D_SECRET_KEY`, or web UI. `hi3d_run.py run --face` aborts if quote does not echo `face`. |
| One quote → one `create_generation` | Quote IDs are single-use |
| No auto-retry on failed generation | Server policy; diagnose and re-quote if needed |
| Prefer **`source_images`** over re-upload | Avoids expired tokens |
| Download **original** GLB | Preview meshes are lower quality |
| Signed URLs expire | Re-call **`get_task_outputs`** before download |

## References

- **`references/upload-limits.md`** — PNG size cliff, WebP default
- **`references/prompts.md`** — prompt patterns for Dungeon Core creatures
- **`scripts/hi3d_client.py`** — OAuth + MCP HTTP client
- **`scripts/hi3d_run.py`** — quote / run / poll / download CLI
