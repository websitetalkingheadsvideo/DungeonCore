# Store icon — GenerateImage prompt

Source of truth: [Design/Store.md](../../../../Design/Store.md) row, [Design/General.md](../../../../Design/General.md), [firefly-store-item-prompt/references/store-items.md](../../firefly-store-item-prompt/references/store-items.md).

## Fixed spec (match existing set)

- **512×512** effective output (post-process resize)
- **Flat pure white** in the GenerateImage prompt; post-process keys **edge-connected white** to transparent (`--backdrop white`, default)
- **Legacy** batch icons used checkerboard JPGs (`--backdrop legacy`)
- **One object**, centered, ~80% frame, bold at **48×48** HUD size
- **Crystalline Ley-Line Arcana**: stone grey, dark iron, obsidian, quartz; reserved colors in General.md (no violet for non-Will items)
- **Lighting:** strong key from upper left, high contrast
- **No** text, border, inventory frame, UI chrome

## Reference images

Pass `reference_image_paths` with 1–2 **existing** icons closest in role/material:

- `Design/Reference Images/icons/mana_generator.png`
- `Design/Reference Images/icons/loot_bait.png`

## Prompt skeleton

```
Game store UI icon, square 512 pixels, flat pure white background only, no black floor, white visible in all air gaps between supports. Crystalline Ley-Line Arcana dark fantasy strategy. Grounded realistic painted rendering, strong key light from upper left, high contrast deep shadow. One object centered filling 80% of frame, bold silhouette readable at 64 pixels. Palette stone grey, dark iron, obsidian black, [ACCENT from store-items.md]. No text, letters, numbers, border, or frame. Subject: [ITEM NAME] — [SILHOUETTE from Store.md / store-items.md].
```

**Filename** for GenerateImage: `{basename}.jpg` (saved under Cursor assets; post-process writes repo icons).

## Regeneration (step 3)

Increment version only (`{basename}-vN.png`). Regenerate with user's notes prefixed:

```
Same store UI icon as before for {item}. Keep: three-quarter view, Ley-Line materials, readable silhouette. Change only: {USER NOTES}.
```

Optionally pass the latest `-vN` or canonical PNG as `reference_image_paths`.