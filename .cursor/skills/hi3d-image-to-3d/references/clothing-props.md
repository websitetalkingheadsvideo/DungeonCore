# Hi3D — clothing & props (from docs.hi3d.ai + Context7 `/websites/hi3d_ai_en`)

## What Hi3D expects

- **One task → one 3D object** reconstructed from what the image shows ([FAQ](https://docs.hi3d.ai/en/api/getting-started/faq)).
- **Single-view img2model:** 1 image; model copies **pose and layout** in the photo (flat lay → flat mesh; top-down boot → top-down mesh).
- **Multi-view img2model:** 2–4 images, **same item**, order **front → back → left → right**; front required. Use for props/clothes when one view is thin.
- **`rmbg`:** background removal default **on** ([Create Task API](https://docs.hi3d.ai/en/api/api-reference/list/create-task)) — still need a **3D-like product pose**, not a 2D catalog sheet.
- **3D Model Split:** splits an **uploaded GLB** (character templates / general parts) — **not** for separating items on a flat outfit sheet ([split-create-task](https://docs.hi3d.ai/en/api/api-reference/list/split-create-task)).
- Studio guidance (marketing FAQ): clear, well-lit, **full subject visible**, minimal occlusion, simple background. API examples use **single props** (e.g. `glove_1.jpg`).

## Do not

- One image with **many pieces** (outfit sheet) → one welded mesh.
- **Top-down / side-by-side** shoe layout → non-wearable shoe shapes.
- Expect **skinned separate materials** per garment from img2model without DCC cleanup.

## Dungeon Core workflow (one item at a time)

1. **2D edit** (MCP `create_image_generation`, `operation=edit`): isolate **one** piece; pose as **product / mannequin / hanging drape**, front hero, white background.
2. Optional: **multi_view** 2D (`multi_view=true`) for front/back/left/right of that **same** piece, then img2model with all views if MCP/API supports `file_tokens` × 4.
3. **quote_generation** → user confirms → **create_generation**; download **original** GLB to `imports/hi3d/<slug>/`.
4. UE: import as separate static/skeletal mesh; parent to socket; **mirror** second boot if only one side modeled.

## Prompt hints (3D)

```
Single [item] game prop, [natural worn/product pose], hollow [waist/neck] opening if garment,
front view, stylized dark fantasy, clean white background, no other objects, no text
```

## Piece order for Dark Fairy Worker

1. Skirt (drape on waist form)  
2. Vest  
3. Belt + pouches (may stay one prop)  
4. Pickaxe  
5. Boots — **one boot, side profile standing on ground**, then mirror in UE  
