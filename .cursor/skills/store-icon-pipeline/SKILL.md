---
name: store-icon-pipeline
description: >-
  Dungeon Core HUD store icons: Firefly prompt for the user, user drops remove-bg
  PNG in Design/Reference Images/icons, agent fits 512×512, approval, import
  /Game/UI/Icons via Unreal MCP. One item per run; stop after import until next.
  Use for store icons, spore_generator, map_table, missing /Game/UI/Icons.
---

# Store icon pipeline

Produces **512×512 RGBA** icons for [Design/Store.md](../../../Design/Store.md) rows. **One store item per run** unless the user names a queue.

**Loop policy (user choice):** After step 5, **stop**. Tell the user which item is next; wait for **“next”**, a **store #**, or an **item name** — do not auto-start the following icon in the same turn.

## Roles

| Who | Does what |
| --- | --- |
| **Agent** | Firefly + Hi3D prompts, fit/resize draft `-vN`, present for approval, canonical + Unreal import after **approve** |
| **User** | Run **Adobe Firefly** from the prompt, **Edit Image → Remove background**, save/drop the cut into `Design/Reference Images/icons/`, approve the draft |

Full-res **mesh** art stays in [Design/Reference Images/Dungeon Items/](<../../../Design/Reference%20Images/Dungeon%20Items/>) for Hi3D; only the **HUD cut** goes in `icons/`.

## Workflow (required order)

1. **Provide Firefly prompt** (and wait for the user’s PNG, or process it when they say it’s added)
2. **Resize / fit** → `{basename}-vN.png`
3. **Present for approval**
4. **Revise** per user notes (new Firefly pass or new drop — new `-vN`)
5. **If approved** — canonical `icons/{basename}.png` + `/Game/UI/Icons` import + HUD `IconTex`
6. **Name next row** — wait for user

Do not import to Unreal before explicit approval.

---

## Step 0 — Pick the item

| User says | Action |
| --- | --- |
| Item name or `#N` | That row in Store.md |
| `next` / skill only | Lowest store **#** that still needs work (see queue below) |
| `all` / `batch` | Still **one item per turn**; user must say next after each approval |

Read the row: **Icon file** column (`{basename}.png`), item name, design intent.

**Queue logic (what “needs an icon”):**

1. No canonical `Design/Reference Images/icons/{basename}.png` — start at **step 1** (prompt) or **step 2** if user already dropped a cut
2. Canonical exists but no `/Game/UI/Icons/{basename}` or HUD row not wired — **import only** (after approval if draft changed)
3. Otherwise skip to next store #

`Design/*.md` is read-only unless the user permits edits.

---

## Step 1 — Firefly prompt (agent delivers first)

**Read and follow** [firefly-store-item-prompt/SKILL.md](../../firefly-store-item-prompt/SKILL.md) for the active store row (#0–10).

**Deliver in chat every time** (before expecting a PNG):

1. Store **#**, name, slug, suggested Dungeon Items filename (`<slug>-vN.png`)
2. **Firefly prompt** — one fenced block, **&lt;1024** chars (pure **black** background in Firefly; not white)
3. **Character count** (optional quick check)
4. **Hi3D prompt** — second fenced block (prop mesh ref; user saves full-res under Dungeon Items)
5. **User instructions** (short):
   - Generate in Firefly → **Remove background** on the result
   - Save the cut to `Design/Reference Images/icons/` (any clear name, e.g. `{basename}_v4.png` or `{basename}-firefly-v1.png`)
   - Tell the agent the filename when ready (or `@` the file in chat)

**Do not** run Firefly for the user. **Do not** skip the Firefly prompt unless the user already has an approved Dungeon Items / icon source and says **skip prompt**.

**Legacy path (only if user asks):** `GenerateImage` + white/black post-process — see [references/prompt-spec.md](references/prompt-spec.md) and `postprocess_store_icon.py` without `--from-firefly`.

---

## Step 2 — Resize / fit (after user drops remove-bg PNG)

User file lives under `Design/Reference Images/icons/` (underscore or hyphen names OK). Agent runs from repo root:

```powershell
python .cursor/skills/store-icon-pipeline/scripts/postprocess_store_icon.py `
  --src "Design/Reference Images/icons/{user_drop}.png" `
  --basename {basename} `
  --from-firefly `
  --out-dir "Design/Reference Images/icons"
```

- **`--from-firefly`:** RGBA with alpha → center fit **~80%** of 512, premultiplied resize (no white/black key).
- **`--fill`** optional (default `0.80`).
- Output: new **`{basename}-vN.png`** only (never overwrite prior `-vN` or canonical).
- **Do not** pass `--write-canonical` until step 5 approval.

---

## Step 3 — Present for approval

In chat:

- Read/show the new `-vN` if useful.
- **Link + backtick path** ([markdown-local-links.mdc](../../rules/markdown-local-links.mdc)):
  - [icon draft](<Design/Reference Images/icons/{basename}-vN.png>) — `Design/Reference Images/icons/{basename}-vN.png`
- One-line read at **48×48** (silhouette + accent).
- Ask: **approve**, **revise** (Firefly notes / new cut), or **stop**.

---

## Step 4 — Revise

User revises in Firefly or drops a new remove-bg PNG → repeat **step 2** → **step 3**. Do not import until approved.

---

## Step 5 — If approved, add to project

1. **Canonical reference** (overwrite only after approval):

```powershell
Copy-Item -Force "Design/Reference Images/icons/{basename}-vN.png" `
  "Design/Reference Images/icons/{basename}.png"
```

   Or re-run post-process with `--write-canonical` if re-fitting from source.

2. **Unreal** — [references/unreal-import.md](references/unreal-import.md):
   - Import `Design/Reference Images/icons/{basename}.png` → `/Game/UI/Icons/{basename}` (reimport `replace_existing` if asset exists; re-wire `StoreItem_{N}` `IconTex` if broken)
   - Compile/save `WBP_DungeonHUD`; **stop PIE** first

3. If MCP unavailable: **report and stop** ([mcp-down-stop.mdc](../../rules/mcp-down-stop.mdc)). Do not leave loose PNG in `Content/UI/Icons/`.

4. Link deliverables: canonical PNG + `/Game/UI/Icons/{basename}`.

---

## Step 6 — Next item

Name the **next** store row that still needs an icon. **Do not** start step 1 again until the user replies.

---

## Checklist template (agent)

```
- [ ] Row # / basename identified
- [ ] Step 1 Firefly + Hi3D prompts delivered to user
- [ ] User drop received; step 2 --from-firefly → -vN
- [ ] Step 3 presented; user decision
- [ ] Step 4 revisions (if any)
- [ ] Step 5 canonical + /Game/UI/Icons import
- [ ] Step 6 next row named; waiting for user
```

## References

- **Firefly / Hi3D prompts:** [firefly-store-item-prompt/SKILL.md](../../firefly-store-item-prompt/SKILL.md)
- Legacy GenerateImage JPG path: [references/prompt-spec.md](references/prompt-spec.md)
- UE import: [references/unreal-import.md](references/unreal-import.md)
- Fit script: [scripts/postprocess_store_icon.py](scripts/postprocess_store_icon.py) (`--from-firefly`)
- Legacy checker batch: [tmp/icons-checkerboard-to-png.py](../../../tmp/icons-checkerboard-to-png.py)