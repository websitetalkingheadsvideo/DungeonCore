# Dark Fairy Sentinel (Evil Fairy Elite)

**Character Hi3D only** — no glaive in this mesh. Glaive: `../dark-fairy-elite-glaive/`.

| Rule | Value |
| --- | --- |
| Max faces (per GLB) | **1,000,000** — decimate in CC/Blender if Hi3D export is heavier |
| AccuRIG body | ≤ **600k tris / 300k quads** before rigging |
| CC import | No HD Rewrap on img2model meshes |

Prompts and ref naming: `.cursor/skills/hi3d-image-to-3d/references/prompts.md` (Evil Fairy Elite section).

2D ref (canonical): `Design/Reference Images/Dark Fairy/Dark Fairy-Elite_v2.png`

## Files

| File | Notes |
| --- | --- |
| `Dark_Fairy_Elite_Sentinel.glb` | Hi3D export — **~1.99M tris** (decimate before CC). **Hands:** two gens had split/extra fingers — fix in CC (see below) before AccuRIG. |
| `dark-fairy-elite-sentinel_hi3d_manifest.json` | [Hi3D model page](https://www.hi3d.ai/3d-model/3eba44cc856f4ed2afd1f2b79007c6a7) |

### Hand fix (before AccuRIG)

1. **CC:** Delete or weld the split finger geo on each hand; or **Replace Mesh → hands** from `dark-fairy-worker_v2.ccProject` if worker hands are clean (match skin shader).
2. **Regen (45 pts):** Only if sculpt swap fails — regen with `Dark Fairy-Elite.png` + hand guardrails in `prompts.md`; optional hand close-up as second Hi3D view via `source_images`.
