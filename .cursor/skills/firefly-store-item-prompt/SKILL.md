---
name: firefly-store-item-prompt
description: >-
  Write photoreal PBR Adobe Firefly prompts (under 1024 characters) plus Hi3D
  companion text for Dungeon Core store items from Design/Store.md — generators,
  traps, bait, constructs. One item per run. Not cartoon or mobile-game stylized.
  Composition locks to Design/Reference Images/icons; full-res saves to
  Design/Reference Images/Dungeon Items. Use for store item Firefly prompts,
  shop props, trap or generator mesh refs, or Hi3D placement art.
---

# Firefly store item prompts

Writes prompts only. The user runs **Adobe Firefly**. One **store row** per invocation (`Design/Store.md` items 0–9).

Downstream pipeline (do not run Firefly or Hi3D inside this skill unless the user asks):

1. **This skill** — Firefly prompt + Hi3D text prompt.
2. **Firefly** — save full-res to `Design/Reference Images/Dungeon Items/<slug>-vN.png` (never overwrite; bump `-v2`, `-v3`, …).
3. Optional **icon** — crop or regenerate for menu; save to `Design/Reference Images/icons/<basename>-vN.png` (`basename` = Store.md **Icon file** column without extension).
4. **`hi3d-image-to-3d`** — GLB from the **Dungeon Items** image (not the 48×48 crop).

Companion skill for characters: `firefly-character-prompt`. Store items are **props/structures**, not T-pose humanoids.

## Before writing

Read in order:

| Source | Why |
| --- | --- |
| Matching row in `Design/Store.md` | Name, icon basename, design intent in “Needs to open: design docs” |
| Trap / construct bullets in `Design/General.md` | Canonical behavior and look (ley-line, quartz, geodes, obsidian shards) |
| `Design/Reference Images/icons/<basename>.png` if present | **Composition lock:** ¾ isometric, pure black background, basalt + crystal + accent glow (materials render **photoreal PBR**, not toon) |
| `references/realism.md` | Required realism block, closers, negatives |
| Same item in `references/store-items.md` | Slug, silhouette notes, accent color |

If an icon exists but contradicts `General.md` on **function** (e.g. reads as teleporter), keep the **icon’s art language** (materials, lighting, angle) and fix **silhouette semantics** per `General.md`. Say so in the reply.

`Design/*.md` is read-only unless the user permits edits. Do not update `Store.md` after a prompt.

## Visual rules (every store prop)

**Composition** — lock to existing icons (`mana_generator.png`, …): single object, centered, **¾ isometric** (slightly above eye level), readable at **48×48** after downscale; **pure black** background, no floor plane, no vignette.

**Art direction** — **Crystalline Ley-Line Arcana** (`Design/General.md`): dark **basalt**, **riveted iron**, **quartz / geode**; motivated **internal glow** (one accent per item — `references/store-items.md`).

**Rendering** — **photoreal PBR** hero still-life, not illustration. See [references/realism.md](references/realism.md).

| Do | Do not |
| --- | --- |
| Pitted stone, mortar gaps, chipped edges | Smooth plastic stone, flat colors |
| Oxidized iron, rivets, wear on flats | Cartoon metal, glowing edges only |
| Quartz refraction, fractures, subsurface glow | Neon flat emissive, cel shading |
| Cinematic studio key + subtle rim | Mud, torches, dungeon corridor BG |
| | Chibi, mobile-game toon, clip art, inventory frame |

**Scale:** one **10 ft grid cell** (~300 UU); floor traps stay **low profile**.

Do **not** use T-pose, hands block, character sheet language, or the words `stylized`, `game asset icon`, `cute`, `fantasy illustration`.

## Realism block (required in every Firefly prompt)

Copy from [references/realism.md](references/realism.md). Short form — do not paraphrase away the negatives:

```
photoreal PBR materials: weathered stone with pores chips and mortar gaps, oxidized wrought iron with pitting and rivets, natural quartz facets with internal fractures and refraction, believable subsurface glow on magical accents not flat neon, cinematic studio key light and subtle rim light on hard edges, not cartoon not stylized not mobile game not plastic toy,
```

If the prompt is **≥1024** characters after assembly, trim redundant adjectives **before** cutting this block or the required closer.

**Required closer (Firefly):** `photoreal PBR game prop reference, high detail, no text, no watermark`

## Deliver (every run)

1. **Item** — Store `#`, name, slug, suggested Firefly filename.
2. **Firefly prompt** — one fenced block, paste-ready.
3. **Character count** — exact length of the Firefly string (including spaces); must be **under 1024**. If ≥1024, cut adjectives, never cut black background or isometric/centering.

```powershell
python -c "p = '''PASTE PROMPT HERE'''; print(len(p))"
```

4. **Hi3D prompt** — second fenced block, **under 2000** characters (prop export; defaults match `hi3d-image-to-3d`).
5. **Style notes** — 2–4 bullets: what you locked from the icon vs what you pulled from `General.md`.
6. **Save paths** (reminder only):

   - Full-res: `Design/Reference Images/Dungeon Items/<slug>-vN.png`
   - Icon (after approval): `Design/Reference Images/icons/<basename>-vN.png`
   - Hi3D slug: `imports/hi3d/<slug>/`

## Firefly prompt template

Build in order; keep under **1024** characters.

1. `Isolated [ITEM ROLE] game prop for a crystalline ley-line dungeon, single object centered, three-quarter isometric view slightly above, pure black background,`
2. **[Silhouette]** from `General.md` / `store-items.md` (2–3 sentences, concrete nouns).
3. **[Accent glow]** — color + source; subsurface, not flat neon.
4. **Realism block** (required).
5. `fits one dungeon grid cell scale,` + **required closer**.

## Hi3D prompt template

```
Isolated [ITEM ROLE] dungeon game prop mesh reference, same design as Firefly image,
three-quarter isometric, [materials, weathering, glow summary],
single object centered, dark background,
photoreal PBR mesh reference, believable edge wear, not cartoon not stylized,
clean silhouette for real-time game, no text, no watermark
```

Never use `stylized fantasy` or `game icon style` in Hi3D text.

Hi3D **`slug`** = kebab-case from item name (see `references/store-items.md`).

## One item at a time

If the user does not name an item, pick the **lowest store index** that has no file in `Design/Reference Images/Dungeon Items/` matching that slug (or ask which `#` they want).

If they name **regen** or **v2**, read the latest `Dungeon Items` and `icons` versions; write an **edit** prompt:

```
Same [item] as reference image. Keep [3–5 identity locks]. Change only [one thing]: more photoreal PBR microdetail on stone metal and quartz, not cartoon.
Pure black background, three-quarter isometric, [realism block short form], no text.
```

## After the user accepts the image

Do **not** edit design docs. Optionally record in chat: filename, task_id when Hi3D runs, and whether the icon was updated.

## References

- Per-item slugs, accents, silhouettes: [references/store-items.md](references/store-items.md)
- Realism block and negatives: [references/realism.md](references/realism.md)
- Example outputs: [references/examples.md](references/examples.md)