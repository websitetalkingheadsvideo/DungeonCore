# Hi3D prompts (Dungeon Core)

Keep prompts under **2000** characters. Align with **`Design/Monsters.md`** and faction rows in **`Design/Creatures and style.md`**.

## Image requirements

- **T-pose** or neutral A-pose for humanoids; arms visible, not occluded by wings/cape unless folded.
- **Single subject**, simple or dark gradient background (model reads silhouette).
- **Front-facing** hero shot; multi-view sets use Hi3D **`source_images`** in view order (not one collage).

## Template (creature export)

```
T-pose [role] for a dark fantasy dungeon game, full body visible, [silhouette / costume / props],
clean polished stylized realism (not photoreal mud), neutral dark background,
game character reference sheet quality, no text, no watermark
```

## Example (Fairy boss — Evil Fairies / Sunken Ruins)

```
T-pose dark celestial fairy boss, full body, four tattered galaxy wings spread wide,
gothic teal corset, combat boots, pale skin, long hair, game character reference,
neutral dark background, stylized fantasy, no text
```

## Evil Fairy Elite — Dark Fairy Sentinel

Two Hi3D generations (do **not** model the glaive on the character mesh). Ref images go under `Design/Reference Images/Dark Fairy/` with **new versioned filenames** (e.g. `Dark-Fairy-Elite-Sentinel-v1.png`, `Dark-Fairy-Elite-Glaive-v1.png`).

**Current 2D refs (Firefly):**

| Asset | Path |
| --- | --- |
| Sentinel body | `Design/Reference Images/Dark Fairy/Dark Fairy-Elite_v2.png` (hands fixed; v1: `Dark Fairy-Elite.png`) |
| Glaive prop | `Design/Reference Images/Dark Fairy/Firefly_Gemini Flash_Isolated crystal glaive game weapon prop, dark celestial fairy sentinel weapon, long  694797.png` |

**Mesh budget (per GLB):** ≤ **1,000,000 faces**. Hi3D has no face-cap API — if the export exceeds 1M, decimate in Character Creator or Blender before merge/socket. AccuRIG humanoid step still requires ≤ **600k tris / 300k quads** on the rigged body.

**Silhouette:** Four tattered galaxy wings (accepted — distinct from queen via circlet vs crown, lighter pauldrons, mid-calf skirt, no floor train).

**Known Hi3D issue (sentinel):** Two body gens both produced **extra/split fingers** (6-ish or bifurcated digits) on both hands. **Root cause (v1 ref):** … **v2:** `Dark Fairy-Elite_v2.png` — correct finger count; use for any future Hi3D regen.

**Hi3D ref hand pose (match worker / boss sheets):**

- Arms horizontal T-pose; palms down or slightly forward.
- **Fingers together or slightly relaxed — not splayed.** No wide vertical gaps between digits; pinky **not** dropped below the hand plane.
- **Exactly five fingers** visible per hand; no overlapping duplicates in the illustration.

**Hand guardrails (append to any humanoid Hi3D prompt):**

```
anatomically correct hands, exactly five fingers on each hand, clearly separated fingers,
no extra digits, no fused or split fingers, visible knuckles
```

**Regen ref tip:** Save as **new file** (e.g. `Dark-Fairy-Elite-HandsFix-v1.png`); do not overwrite `Dark Fairy-Elite.png`. Optional hand close-up inset as second Hi3D view via `source_images`.

**Firefly / 2D fix prompt (same outfit, fix hands only):**

```
Same dark celestial fairy elite sentinel character and costume as reference, T-pose arms horizontal,
both hands empty, fingers relaxed and close together like a mannequin not spread apart,
exactly five normal fingers per hand clearly drawn, no extra digits no split fingers,
pinky aligned with other fingers not dropped vertically, white background, full body front view
```

### Character (empty hands)

```
T-pose dark celestial fairy elite sentinel, full body front view, empty hands slightly bent away from body,
four tattered galaxy wings, gothic teal corset and light pauldrons, mid-calf tattered skirt no floor fabric,
circlet no crown, pale skin purple hair, combat boots, no weapon no props in hands,
anatomically correct hands, exactly five fingers on each hand, clearly separated fingers, no extra digits,
game character reference, white background, stylized fantasy realism, no text
```

**Output slug:** `dark-fairy-elite-sentinel` → `imports/hi3d/dark-fairy-elite-sentinel/`

### Glaive (prop only)

```
Isolated crystal glaive game weapon prop, dark celestial fairy sentinel weapon, long obsidian shaft,
teal and amethyst crystal blade with subtle galaxy sparkle, full weapon centered,
no character no hands, product reference on white background, stylized fantasy realism, no text
```

**Output slug:** `dark-fairy-elite-glaive` → `imports/hi3d/dark-fairy-elite-glaive/`

Socket the glaive to the rig in CC or Unreal after AccuRIG; wings still split or socketed per worker pipeline.

## Version / quality defaults

Unless the user asks otherwise:

- **`version`**: `v3.0`
- **`model_type`**: `high_quality` (v3.0 standard tier — not `master` unless requested)
- **`generation_type`**: `model_with_texture`
- **`pbr`**: `true`
- **`license`**: `private`
