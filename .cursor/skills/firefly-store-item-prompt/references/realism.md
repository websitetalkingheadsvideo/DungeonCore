# Store prop realism (Firefly + Hi3D)

Dungeon Core store art is **Crystalline Ley-Line Arcana** (`Design/General.md`): polished geometry and crystal, not mud-and-torches — but meshes and icons target **photoreal PBR**, not toon or mobile-game stylization.

## Required realism block (Firefly)

Copy into every Firefly prompt (adapt metal/crystal nouns per item). Do not shorten to “high quality” or “detailed.”

```
photoreal PBR materials: weathered stone with pores chips and mortar gaps, oxidized wrought iron with pitting and rivets, natural quartz facets with internal fractures and refraction, believable subsurface glow on magical accents not flat neon,
cinematic studio key light and subtle rim light on hard edges,
not cartoon not stylized not mobile game not plastic toy,
```

## Required closer (Firefly)

Always end with (after item-specific content):

```
photoreal PBR game prop reference, high detail, no text, no watermark
```

## Material vocabulary (use in prompts)

| Surface | Phrases that work |
| --- | --- |
| Stone | basalt, pitted mortar, chipped arrises, crack fill, circular recess |
| Metal | wrought iron, rivets, cross-bracing, oxidation, brushed wear on flats |
| Crystal | smoky quartz, hexagonal prisms, frosted panels, fractured ice texture |
| Glow | emissive from within, subsurface scatter, light leaking through stone cracks; one accent hue per item |

## Lighting

- **Background:** pure black only — no gray gradient, no floor, no environment.
- **Key:** single cinematic studio key (soft but directional).
- **Rim:** thin rim on metal and crystal silhouettes for icon readability at 48×48.
- **Internal:** trap/generator glow is **motivated** (core, runes, geode) — not full-object bloom.

## Hi3D text

Mirror the Firefly materials; add:

```
photoreal PBR mesh reference, believable weathering and edge wear, not cartoon,
```

Do **not** ask Hi3D for “stylized fantasy” or “game icon style” — that skews toon.

## Negatives (Firefly — explicit in prompt when space allows)

`not cartoon, not chibi, not cel shaded, not smooth plastic, not oversaturated, not clip art, not diorama base, not inventory frame`

## Relation to icons

`Design/Reference Images/icons/` locks **composition** (¾ isometric, black, silhouette). Older icons may look slightly more stylized; **new** Firefly gens push **more realism** while keeping the same silhouette and materials family.

## Regen / v2

When the user says too toony, add the realism block if missing, swap `stylized`/`game asset icon` for `photoreal PBR`, and name 2–3 **microdetail** locks (pitting, fractures, mortar).