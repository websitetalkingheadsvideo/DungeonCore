# Aura Plan prompt — Monster tier QA fixes (Dungeon Core)

Copy everything below the line into **Aura → Plan mode**. Attach `@todo/Monsters-11-23-26.md`, `@Design/Monsters.md`, and optionally `@tmp/monster-gallery-level-for-aura.md` (validation level `L_MonsterTierGallery`).

---

## Mode

**Plan only first** — no content edits, no Blender/Unreal changes, no Hi3D jobs until the human approves the plan.

The plan must be executable in phases (human can ship family-by-family). Output structure is mandatory (sections below).

## Project context

**Dungeon Core** — UE5 dungeon defender. Canonical tier meshes live under `/Game/Monsters/` per [Monsters.md](Design/Monsters.md). Humanoid pipeline: Hi3D → Character Creator → AccuRIG → UE; goal is **shared CC skeleton** + shared idle/locomotion for non-Paragon humanoids ([TODO.md](TODO.md) — “Convert Fab creatures to the CC skeleton”). **Exceptions:** Paragon Heroes (Narbash, Fey, Sevarog, Rampage), 8-leg spiders, Hollow Walker (Meshy embedded anims).

**Validation surface:** Dev gallery `L_MonsterTierGallery` (`/Game/Maps/Dev/L_MonsterTierGallery`) — 6 families × 5 tiers; use for before/after PIE checks and foot labels.

**Hard rules**

- **Do not edit** `Design/*.md` unless the human explicitly permits it in chat (propose doc updates as quoted blocks in the plan only).
- **Never overwrite** reference images or mesh basenames; use **versioned** outputs (`-vN`, new slug) per [reference-images-versioning.mdc](.cursor/rules/reference-images-versioning.mdc).
- Failed mesh/anim load: **log error** — no silent cube/material fallback.
- Grid scale: **1 gameplay block = 300 UU** (10 ft) unless a file states otherwise. **“Normal human height”** for Undead tier normalization = ~**180 UU** standing eye line (state assumption if you use bounds-based math instead).

## Objective

Produce a **fix plan** for every issue in [Monsters-11-23-26.md](todo/Monsters-11-23-26.md), grouped by **root cause** (bind pose, skeleton retarget, idle anim missing, mesh penetration, pivot/ground offset, scale, materials/textures, regen vs in-engine fix). Each tier row must end with **concrete steps**, **tooling** (Blender MCP, Unreal MCP, CC/AccuRIG, Hi3D regen, material-only UE), **acceptance test** on gallery level, and **risk/rollback** (what asset path version to keep).

## Source issue list (canonical — expand in plan)

### Orcs

| Tier | Issue |
| --- | --- |
| Worker, Minion, Elite, Boss | **A-pose** — no idle; only **Hero (Narbash)** has working idle |
| Hero | OK (idle) — reference pattern only |

### Evil Fairies

| Tier | Issue |
| --- | --- |
| Worker, Minion, Elite, Boss (4 tiers) | Idle plays but **hands mostly inside torsos**; **slight forward lean** on all four |
| Hero | Not listed — call out if gallery shows issues |

### Fungal

| Tier | Issue |
| --- | --- |
| Worker | **Facing backward**; **feet in ground**; **no idle** |
| Minion | Idle OK sometimes; **left arm clips into leg** |
| Elite | **No idle**; **no texture** (material?) |
| Boss | **Increase height (scale)**; **no texture**; idle OK but **hands behind legs** |
| Hero | **Feet in ground**; idle **slides / wobble** |

### Undead

| Tier | Issue |
| --- | --- |
| All except Hero | **Normalize to same height** = normal human height (Hero exempt) |
| Worker | Idle; **hands crossed**; **elbows inside body** |
| Minion | Idle but **backward**, **tilted**; **arms behind back**; **legs crossed** |
| Elite | **Shoulders and elbows arched back** (pose / weights / anim?) |

### Mutants

| Tier | Issue |
| --- | --- |
| Worker | **Feet in ground**; **neck mesh hole** |
| Minion, Elite | Todo note incomplete: **“for some reason the original flesh mutant”** — **inspect both** `Mutants/Character_001/Meshes/Monster_001` and `Mutants/FleshMutantMonster/Meshes/SK_FleshMutantMonster`; plan must **inventory** what’s wrong vs Worker/Boss/Hero |
| Boss (Hollow Walker) | **Texture** to match other Mutants’ style; idle described as **scream** — confirm acceptable gameplay idle or pick alternate from Meshy set |
| Hero (Rampage) | Not in todo — note if gallery scale/anim differs |

### Spiders (scale-only per todo)

| Tier | Scale change |
| --- | --- |
| Worker | **−25%** |
| Minion | **−25%** |
| Elite | **+25%** |
| Boss | **+10%** |
| Hero | **+20%** |

Apply via actor/component scale on gallery + document spawn-table scale policy (uniform vs bone — **recommend one approach**).

## Required plan sections

1. **Executive summary** — counts: how many fixes are anim/skeleton vs mesh regen vs material vs scale-only; suggested **phase order** (e.g. Spiders scale first as quick win; Orc idle batch via CC skeleton; etc.).

2. **Root-cause taxonomy** — map each issue type to fix families:
   - Missing idle / A-pose → CC retarget, Single Node idle, or Paragon anim set
   - Clip / interpenetration → bind pose, weight paint, Blender edit, or Hi3D regen with better T-pose refs ([firefly-character-prompt](.cursor/skills/firefly-character-prompt/SKILL.md))
   - Feet in ground / backward facing → import transform, root motion, capsule offset, mesh origin in Blender
   - Missing texture → MI assignment, CC export materials, Hi3D PBR reimport
   - Height normalization → bounds-based scale factor table per Undead tier

3. **Per-family tables** — for **each** of 29 placements (+ Mutants Hero if in gallery):
   - Current `/Game/Monsters/...` path (from Monsters.md)
   - Issue(s) from list above
   - **Proposed fix path** (A/B if uncertain)
   - **Effort** (S/M/L) and **dependencies** (e.g. Boss CC skeleton blocks shared idle)
   - **Acceptance criteria** (one line: what you see in front view + idle 10s loop on `L_MonsterTierGallery`)

4. **Orcs + shared skeleton** — explicit tie-in to [TODO.md](TODO.md) CC conversion list: Worker (`SK_StylizedGoblinMinion`), Elite (`elite_orc`), Boss (`Orc_Boss_unreal`) vs done Minion (`MHC_MetaGoblin`). Plan whether Worker stays Fab stylized goblin with **local idle** or gets CC skeleton. Narbash stays **out of CC**.

5. **Evil Fairies** — hands/lean: compare bind pose vs anim; check if all four share one skeleton; recommend **Blender** penetration fix vs **Firefly vN ref + Hi3D regen** (cite existing Dark Fairy elite hand lessons in hi3d skill refs).

6. **Fungal** — separate Elite/Boss **texture gap** from Elite **no idle**; Boss **scale target** (relative to Minion or human 180 UU). Queen idle wobble: foot IK vs anim strip vs ground offset.

7. **Undead** — publish a **scale factor table** (Worker…Boss, Hero unchanged) from mesh bounds; pose fixes for skeleton_v1 / mummy / Zombie — prioritize Minion “backward” as import rotation vs anim.

8. **Mutants** — neck hole on worker: mesh repair in Blender vs regen; Boss material direction (reference Worker/Minion/Elite MI); clarify Minion/Elite todo gap after MCP asset inspection (list bones, materials, anims present).

9. **Spiders** — uniform scale on skeletal mesh actor vs asset metadata; warn on physics/capsule if gameplay uses fixed bounds.

10. **Tooling & execution order** — numbered steps per phase: which MCP namespaces (`user-unreal-mcp`, Blender, `user-hi3d`), which skills, where FBX lands (`D:\DungeonCore\` per TODO), manifest files under `tmp/`.

11. **Out of scope** — gameplay ABP wiring, Hero-tier spawn roll, invaders, store icons, editing Monsters.md without approval.

12. **Open decisions for human** — bullet list only where the plan cannot choose (e.g. Hollow Walker scream idle, Evil Fairy regen vs weight paint, Orc Worker CC vs bespoke idle).

## References to inspect before planning

- `L_MonsterTierGallery` + `L_NewGame` (`NewGame > Creatures`) — idle patterns (`elite_orc` Single Node)
- [Monsters-11-23-26.md](todo/Monsters-11-23-26.md) — human QA notes
- [Design/Monsters.md](Design/Monsters.md) — paths and pipeline notes per tier
- [Design/General.md](Design/General.md) — art pillar for Mutant Boss retexture brief
- Project TODO: CC skeleton conversion + creature animations prereq

## Acceptance (for the **plan document**, not implementation)

- Every line item in Monsters-11-23-26.md appears in the plan with a fix strategy.
- Phases are ordered with clear deps; quick wins called out (Spiders scale).
- No silent assumption that all humanoids share idle until CC conversion is done — plan states interim vs target state.
- Gallery-level test steps are repeatable (PIE, family row, tier label).