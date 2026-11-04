# Dark Fairy Worker — Hi3D imports

## Rigging (Reallusion)

**Pipeline:** Hi3D GLB → Character Creator (import; **do not** use HD Rewrap on img2model meshes) → **AccuRIG** → FBX to UE.

**AccuRIG — Maximum Polycount:** **600,000 triangles** or **300,000 quads** (stay at or under before AccuRIG; decimate in CC/Blender if Hi3D export is heavier).

**Wings:** AccuRIG is humanoid-only — split wing geo or socket static wings; do not expect AccuRIG to weight wings correctly on a single merged mesh.

## Files

| File | Use |
| --- | --- |
| `dark-fairy-worker_v2.ccProject` | **Saved Character Creator project** (rigging/edits complete). |
| `Dark_Fairy_Worker.glb` | **Full dressed worker** (bald, one mesh) — Hi3D source for CC import (task `7b800574…`). |
| `Dark_Fairy_Worker_Body.glb` | Bald + wings base (task `84e246f3…`); optional face-only path. |
| `Dark_Fairy_Worker_Skirt.glb` | Experimental skirt-only gen (task `0faea5b9…`); optional. |

Abandoned: flat outfit-sheet img2model (one welded mesh).

2D refs: `Design/Reference Images/Dark Fairy/`
