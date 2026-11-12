# Hi3D prompts (Dungeon Core)

Keep Hi3D prompts under **2000** characters. **Firefly image prompts must be under 1024 characters.** Align with **`Design/Monsters.md`** and faction rows in **`Design/Creatures and style.md`**.

## Image requirements

- **T-pose** for humanoids (arms horizontal at shoulder height); use **palms facing forward toward camera** when finger/hand read matters—not palms down. Do not mix “A-pose” and “T-pose” in one prompt.
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

**Mesh budget (per GLB):** user target via Hi3D **Advanced → face count** (OpenAPI field **`face`**, 100000–5000000). v3.0 **2048quality** UI/MCP default when unset is **2000000**. **Cursor MCP `quote_generation` schema omits `face` as of 2026-11 — values are stripped server-side; use hi3d.ai UI Advanced, Open Platform `hi3d-cli --face`, or post-decimate until MCP exposes it.** AccuRIG humanoid step still requires ≤ **600k tris / 300k quads** on the rigged body.

**Silhouette:** Four tattered galaxy wings (accepted — distinct from queen via circlet vs crown, lighter pauldrons, mid-calf skirt, no floor train).

**Known Hi3D issue (sentinel):** Two body gens both produced **extra/split fingers** (6-ish or bifurcated digits) on both hands. **Root cause (v1 ref):** … **v2:** `Dark Fairy-Elite_v2.png` — correct finger count; use for any future Hi3D regen.

**Hands:** new Firefly sheets use the `firefly-character-prompt` skill. Arms horizontal, palms facing the camera. Required block (this is what worked on Invader-Dward_v2; cone fingers came from "fingers together like a mannequin"):

```
Each hand has exactly five real human fingers: one shorter thumb set apart, plus index, middle, ring, and pinky. Fingers have knuckles, joints, fingernails, and skin creases, slightly separated so each digit reads clearly. No fused fingers, no cone shapes, no mittens, no extra digits, no missing thumbs.
```

**Regen ref tip:** Save as **new file** (e.g. `Dark-Fairy-Elite-HandsFix-v1.png`); do not overwrite `Dark Fairy-Elite.png`. Optional hand close-up inset as second Hi3D view via `source_images`.

**Historical Firefly hand fix (Dark Fairy elite v2 only — do not reuse):**

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

## Evil Fairy Minion — Dark Fairy fodder

**Approved 2D ref:** `Design/Reference Images/Dark Fairy/Dark-Fairy-Minion-v1.png` (2026-11-07, fal `nano-banana-2` 2K).

Tier between worker and elite: pixie-cut dark blue-purple hair, smoky eye makeup, forest-green tunic under black leather corset,
small dark shoulder caps and bracers with purple gems, knee-length tattered green and purple lace skirt, combat boots,
hip dagger in sheath (mesh prop or socket — do not model glaive). Four tattered iridescent wings green-to-purple, not cosmic-elite scale.
No crown, circlet, or scale-mail pauldrons.

**Hi3D ref hand pose:** same as elite — T-pose, exactly five fingers, hands empty for rigging.

```
T-pose dark celestial fairy minion fodder, full body front view, empty hands,
short messy pixie-cut dark blue hair purple highlights, pale skin dark smoky eye makeup,
black leather corset over forest green tunic, small dark metal shoulder caps and forearm bracers purple gems,
knee-length tattered green skirt purple lace layers, black combat boots, dagger in hip sheath not in hands,
four dragonfly wings iridescent green and purple with small tears, no crown no circlet no scale pauldrons,
anatomically correct hands, exactly five fingers on each hand, no extra digits,
game character reference, white background, stylized fantasy realism, no text
```

**Output slug:** `dark-fairy-minion` → `imports/hi3d/dark-fairy-minion/`

## Version / quality defaults

Unless the user asks otherwise:

- **`version`**: `v3.0`
- **`model_type`**: `high_quality` (v3.0 standard tier — not `master` unless requested)
- **`generation_type`**: `model_with_texture`
- **`pbr`**: `true`
- **`license`**: `private`

**Aesthetic:** Prompt for **stylized realism**—weathered materials, readable silhouette—not mobile toon or chibi. Hi3D meshes usually land **less cartoon** than legacy stylized Fab humanoids; still avoid photoreal mud per [General.md](../../../Design/General.md).

## Mutant Worker — digger base (Hi3D 2D ref)

**Mutants family only** — raw flesh mutant, **no mushrooms, no fungal growth, no spore glow.** Not Fungal Warren.

Blend: **large subterranean eyes**, **normal-sized hands with sharp claws**, **elephant-like padded feet**, **dark mottled raw-flesh skin**, **lean athletic humanoid**. Worker tier: **no circular tooth maw**.

**Style refs:** Fab flesh mutant preview; in-engine bloated maw mutant (skin palette only—drop the mouth); prior featureless claw-hand base (drop oversized hands + sealed face).

**Primary (Firefly / Hi3D input):**

```
T-pose mutant dungeon worker, full body front view, lean muscular humanoid male proportions,
arms straight horizontal at shoulder height, palms facing the camera forward not downward, empty clawed hands fingers together clearly visible,
large pale subterranean eyes wide on face adapted for darkness, no nose small closed mouth slit no teeth visible,
dark reddish-brown mottled raw flesh texture with subtle pores and healed scars not glossy,
normal human arm length and normal sized hands with sharp dark claws not oversized,
elephant-like feet thick padded toes splayed for digging weight, sturdy subterranean digger stance,
compact athletic torso not bloated, no open mouth no ring of teeth no tongue,
no mushrooms no fungal growth on body,
no weapons no props,
game character reference, neutral white background, stylized realism not cartoon not chibi,
dramatic studio lighting, no text, no watermark
```

**Negative / avoid:** palms facing down, back of hands toward camera, open circular maw, rows of teeth, tongue, tiny pin eyes, cute chibi, bright saturated colors, human bare feet, oversized hulk hands, extreme obesity, gore splatter, photoreal horror movie lighting.

**Output slug:** `mutant-worker` → `imports/hi3d/Mutant/` · **Current 2D ref:** [Mutant-Worker_v2.png](../../../Design/Reference%20Images/Mutants/Mutant-Worker_v2.png) · **Complete:** `Mutant-Worker_v2.glb`, `mutant-worker_v3.ccProject`, [mutant-worker.jpg](../../../imports/hi3d/Mutant/mutant-worker.jpg).

**Hi3D 3D:** web Advanced **300000** faces; same pipeline as undead (CC → AccuRIG). No 3D Wrap unless face opens later.

## Fungal Worker — spore digger (Hi3D 2D ref)

**Style refs (mood board):** slug-beast with mushroom colony + gore moisture; gnarled dragon covered in glowing caps; sepia knight with mushrooms in armor cracks; biped fungal humanoid with shoulder caps and sickly purple-tan skin. **Worker read:** lean biped digger like Mutant Worker, not limbless slug, not winged boss, no plate armor.

Blend: **fungal humanoid worker** (Fungal family) + mushrooms on body — **not** Mutants raw-flesh worker. Same digger silhouette as Mutant Worker (eyes, claw hands, elephant feet) but **skin is overtaken by fungus**, not red meat mutant.

**Primary (Firefly / Hi3D input):**

```
T-pose fungal dungeon worker creature, full body front view, lean muscular humanoid male proportions,
arms straight horizontal at shoulder height, palms facing the camera forward not downward, empty clawed hands fingers together clearly visible,
large pale subterranean eyes, small closed mouth no teeth no open maw,
sickly pale tan and muted purple mottled skin with fibrous bark-like fungal patches and moist pores,
clusters of brown and ochre mushrooms sprouting from shoulders upper back and spine,
smaller mushroom caps and thin mycelium veins growing from cracks in torso and arms,
soft bioluminescent pale green and cream glow on mushroom gills and cap spots not bright neon,
normal sized hands with dark claws, elephant-like padded feet for digging,
no armor no weapons no props,
game character reference, neutral white background, dark fantasy stylized realism not cartoon not chibi,
dramatic studio lighting, no text, no watermark
```

**Negative / avoid:** palms facing down, back of hands toward camera, slug body no legs, dragon wings, knight armor, circular tooth maw, blood waterfalls, cute toadstool mascot, chibi, violet glow, oversized hands, human bare feet, photoreal gore.

**Output slug:** `fungal-worker` → `imports/hi3d/fungal-worker/` · **2D ref target:** `Design/Reference Images/Fungal/Fungal-Worker-v1.png`.

**Hi3D 3D:** web Advanced **300000** faces; CC → AccuRIG. Keep cap glow as emissive in UE, not Will violet.
