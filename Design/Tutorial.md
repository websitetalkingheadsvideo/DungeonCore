# Tutorial — Awakening

Onboarding for a newly awakened core, delivered by [Will](./Will.md). The tutorial is the **first minutes of the real game**, not a separate mode—nothing resets when it ends. Genre loop: [Overall.md](./Overall.md). Breach loop: [Creatures and style.md](./Creatures%20and%20style.md).

## Principles

- **Real actions only.** Every beat completes when the player does the thing—never a "Next" button.
- **Never block input.** No modals, no full-screen slides, no frozen camera (see [Start Screen.md](./Start%20Screen.md) instant-click rule).
- **One goal at a time.** Only one objective on screen.
- **Violet = guidance.** Objective highlights use Will's pale violet and slow facet shimmer, nothing else does.
- **Progressive reveal.** UI appears as each system is taught, so the first screen is nearly empty.
- **Short.** Target 10–15 minutes for all beats.

## Delivery rules

| Rule | Detail |
| --- | --- |
| **Objective line** | One line, telemetry font (Orbitron / Rajdhani), top of HUD |
| **Highlight** | Violet shimmer on the target tile, wall, or button; Will darts to it and orbits |
| **Idle hint** | No progress for ~45 s → one extra hint from Will. No second hint unless the player asks for help |
| **Dismiss** | Any Will line can be dismissed; the objective stays as a quiet HUD line |
| **Combat** | During a defense, Will gets one telemetry ping max; other lines queue until the fight ends |
| **Log** | Every Will line is saved to the codex log for rereading |

## Skip

- **New Game card:** "Guided awakening" toggle. On by default for the first save; remembers the player's last choice after that.
- **Mid-tutorial:** Options → "Silence Will" ends the remaining beats.
- **Esc exits the tutorial.** First press shows a one-line HUD prompt ("Esc again to end the awakening"); second press within ~3 s ends the remaining beats. No modal. During the tutorial, Esc does this instead of opening the pause menu.
- **Skipping reveals everything.** Any UI a skipped beat would have unlocked appears immediately; no system stays hidden.

## Beats

Will lines are **drafts** for voice—concise, slightly archaic, grid/essence vocabulary.

### Phase 1 — Awakening

| # | Beat | Player does | Will (draft) | Done when | Reveals |
| --- | --- | --- | --- | --- | --- |
| 1 | **Wake** | Camera arrives from the New Game breach with Will following (first contact already happened on [New Game](./New%20Game.md)); core is dim. Click the core | "Here, then. Your chamber. Touch your own light." | Core clicked | Essence meter |
| 2 | **First capillary** | Power the core room; a capillary runs to the wall and the fracture grid lights (mirrors **New** hover FX) | "That is your grid. Where it glows, you are." | Core room powered | Influence radius outline |

### Phase 2 — Dig

| # | Beat | Player does | Will (draft) | Done when | Reveals |
| --- | --- | --- | --- | --- | --- |
| 3 | **First worker** | Spend essence to spawn a worker | "Hands. Cheap ones. Spend a little essence." | Worker exists | Spawn menu (worker only) |
| 4 | **Blind dig** | Mark a wall block; worker shatters it into fog of war | "Beyond that stone—nothing yet. Dig and see." | Block cleared | Dig tool, fog of war |
| 5 | **Absorb rubble** | Absorb the dug stone | "Stone feeds you. Little, but it feeds." | First absorb | Pattern library (one entry); **first upgrade alert** taught here |

### Phase 3 — Build

| # | Beat | Player does | Will (draft) | Done when | Reveals |
| --- | --- | --- | --- | --- | --- |
| 6 | **Claim ground** | Extend a capillary into the dug space | "Unpowered ground is not yours. Claim it." | New room powered | Room power controls |
| 7 | **First trap** | Place a Resonance Plate in the powered room | "Something to greet guests." | Trap placed | Trap menu |
| 8 | **First defender** | Spawn one creature from the chosen family | "Teeth, now. Yours to command." | Creature exists | Creature menu |

### Phase 4 — First breach (scripted)

A small, pre-placed pocket of another biome sits two blocks from the core room. Its residents are tuned weak.

| # | Beat | Player does | Will (draft) | Done when | Reveals |
| --- | --- | --- | --- | --- | --- |
| 9 | **Warning** | Dig toward the pocket; Will senses it one room ahead (his scout ability) | "Hollow ahead. Something lives there." | Next block marked | — |
| 10 | **Breach** | Worker breaks in; tiles morph to the pocket's biome | "Not your stone. Not yet." | Breach opens | Biome morph |
| 11 | **Defense** | 2–3 residents follow the capillaries toward the core; trap and defender fight them | Ping only: "They follow your lines. Hold." | Residents dead | Core health bar |
| 12 | **Absorb the dead** | Absorb the fallen residents; essence spikes; a new pattern unlocks | Upgrade alert (what / why / where), then: "They will always come for the core. Remember that." | Pattern unlocked | — |

**Safety:** The core can take damage during beat 11 (so the stakes are felt) but can't drop below ~25% in the tutorial.

### Phase 5 — Hand-off

| # | Beat | Player does | Will (draft) | Done when | Reveals |
| --- | --- | --- | --- | --- | --- |
| 13 | **Hand-off** | — | "The grid is yours. Call me when it doesn't show you something." | Line shown | Help / codex button; free play |

## Loop coverage

The tutorial covers the core loop from [Overall.md](./Overall.md) except **lure adventurers**, which needs free play to happen naturally.

| Loop step | Beat |
| --- | --- |
| Gather essence | 5, 12 |
| Expand territory | 2, 6 |
| Build rooms and traps | 4, 7 |
| Spawn monsters | 3, 8 |
| Lure adventurers | Deferred |
| Absorb the dead | 12 |
| Unlock patterns | 5, 12 |

## Deferred beats (free play, first occurrence only)

One-time Will moments that fire the first time each thing happens. Same delivery rules; none block input.

| Trigger | Will teaches |
| --- | --- |
| First adventurer party enters | Adventurers feed the core; lure them in |
| First loot placed as bait | Loot draws parties; better loot, stronger parties |
| Reputation / danger meter moves | Too deadly brings retaliation, too easy starves the core |
| First named adventurer returns | Named parties remember the dungeon |
| First overload state | White-hot lines mean a zone is overloaded |
| First accidental breach after the tutorial | Short reminder only; full lesson already given in beats 9–12 |

## Open questions

- Does dug stone actually give essence (beat 5)? Assumed from the genre's "absorb dirt and ore" convention.
- Is the Resonance Plate the right first trap? It works in every biome; Prism Beam needs mirrors and sightlines.
- Starting essence: enough for worker + trap + creature with no way to soft-lock.
- ~~Is the worker a family creature or a neutral construct?~~ **Resolved:** each family has a Worker role ([Monsters.md](./Monsters.md)).
- Which biome is the tutorial pocket—always the same, or always one the player didn't pick?
