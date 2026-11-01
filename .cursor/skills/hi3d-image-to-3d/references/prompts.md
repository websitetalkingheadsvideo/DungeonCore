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

## Version / quality defaults

Unless the user asks otherwise:

- **`version`**: `v3.0`
- **`model_type`**: `high_quality` (v3.0 standard tier — not `master` unless requested)
- **`generation_type`**: `model_with_texture`
- **`pbr`**: `true`
- **`license`**: `private`
