# Aura — Run Difficulty system (plan + implement)

**You are planning and building Dungeon Core’s run-wide Difficulty system.** Do **not** jump straight into UE wiring until you have a written plan the user can approve. This prompt is your brief.

---

## Read first (repo)

| Doc | Why |
| --- | --- |
| [Design/New Game.md](../Design/New%20Game.md) | `WBP_NewGame` card; **Random Events** and **Randomness** rows are the UI pattern to copy |
| [todo/time-passage-plan.md](../todo/time-passage-plan.md) | Strategic time, workers, raid lock, **first Difficulty consumer** (worker job interrupt banking) |
| [todo/ideas.md](../todo/ideas.md) | Trap / raid context |
| [Design/Random Events.md](../Design/Random%20Events.md) | Frequency-only scaling pattern |
| [Design/Monsters.md](../Design/Monsters.md) | Randomness → Hero spawn % |

**Existing design name:** “Crisis Discipline” in New Game.md is **the worker-interrupt slice** of Difficulty. Your plan should either **fold it into one Run Difficulty enum** or **rename clearly** so one New Game control drives **many** outcomes (worker interrupt is v1; more hooks follow).

---

## Product goal

1. **New Game card:** Add a **Difficulty** row alongside **Random Events** and **Randomness** — same interaction (icon + label per row, highlight selection, three options, saved with the run).
2. **One saved enum per run** (e.g. `EDungeonRunDifficulty::Sanctuary | Standard | IronLey`) readable everywhere gameplay needs a tier.
3. **Extensible outcomes:** Difficulty is not a one-off for workers. Ship a **registry / data table** so new features ask `GetDifficultyOutcome(Key)` instead of hard-coding three branches.
4. **First implemented outcome:** **Worker task interrupt** (flee to safe room on raid warning, or death mid-task) — progress banking per tier (below).

**Independent knobs (do not merge):** Theme, Monsters, **Random Events**, **Randomness**, and **Difficulty** are separate save fields with separate design intent.

---

## Difficulty tiers (player-facing — locked for v1)

| Enum | Card label | Gamer tone |
| --- | --- | --- |
| `Sanctuary` | **Sanctuary Accord** | Forgiving — progress/streaks preserved where the table allows |
| `Standard` | **Standard Delve** | **Default** |
| `IronLey` | **Iron Ley Compact** | Punishing — wipes and full resets where the table allows |

Will FAQ (draft): *"Iron Ley means a panicked worker drops their tools. Sanctuary means they bag the work first."* — expand in plan for hover tooltips on the New Game row.

---

## First outcome spec: `WorkerJobInterrupt`

When a worker **does not finish** a job (`DigCell`, `ResetTrap`, …) because of **FleeOrdered** or **death**:

| Tier | Banked work |
| --- | --- |
| Sanctuary | `Banked = CompletedWork` (100%) |
| Standard | `Banked = min(CompletedWork, TotalWork × 0.5)` |
| Iron Ley | `Banked = 0` |

- `CompletedWork = TotalWork − WorkRemaining` at interrupt.
- Trap / layout only **commits** when job hits **100%** (partial bank = resume time only — see time-passage-plan).
- Same rules for flee and death.

Future worker/raid wiring may not exist yet — plan should separate **Difficulty subsystem** (save, lookup, UI) from **WorkerTaskSubsystem** (consumer).

---

## Future outcome hooks (plan only — no implement unless trivial stubs)

Document these as **rows in a Difficulty Outcomes table** with `NotImplemented` until design assigns numbers:

| Outcome key | Idea |
| --- | --- |
| `WorkerJobInterrupt` | **v1** — above |
| `RaidApproachWarningTime` | Longer telegraph on Sanctuary, shorter on Iron Ley |
| `CaptiveStarvationRate` | Idol vault days-to-death |
| `EssenceOnWorkerDeath` | Respawn cost multiplier |
| `TrapResetManaFee` | Worker re-arm mana surcharge |
| `InvaderTrapLiteracy` | More veterans on Iron Ley |
| `RandomEventSeverityBias` | *Optional* — only if design wants coupling; default **do not** tie to Random Events frequency |

The plan must explain **how to add a new row** without touching New Game UI again.

---

## New Game UI requirements

- **Row:** **Difficulty** — 3 options (Sanctuary Accord / Standard Delve / Iron Ley Compact).
- **Default:** Standard Delve.
- **Persistence:** Written into run save / game instance on **Start Game** with theme, family, random events, randomness.
- **Layout:** Match existing stacked panels (see New Game.md). Card may need a **fourth/fifth row** — plan should show mock copy and whether Randomness + Difficulty order is: Theme, Monsters, Random Events, Randomness, Difficulty (or group Randomness + Difficulty under one “Run rules” sub-panel — **propose**, user picks).
- **Start Game chain:** Today button only prints choices — plan notes what must be passed to the subsystem that owns run config.

---

## Architecture (plan must propose)

Suggested shape (adjust if repo already has a better home):

- `UDungeonRunSettings` or extend existing save struct: `Theme`, `Family`, `RandomEventsTier`, `RandomnessTier`, **`Difficulty`**.
- `UDifficultySubsystem` (or function library): `GetDifficulty()`, `GetOutcomeFloat(FName Key)`, `GetOutcomeEnum(...)`.
- `UDifficultyProfile` **DataTable** or **Primary Data Asset**: rows = `(OutcomeKey, Sanctuary, Standard, IronLey)` with typed columns or JSON curve — **no scattered magic numbers in C++**.
- **Blueprint exposure:** `Get Worker Interrupt Bank Ratio` only if needed; prefer C++ authoritative.

Follow project rules: explicit includes, minimal diff, no `.uasset` hand-edits if Unreal MCP is required for widgets — **state MCP dependency in plan** ([mcp-down-stop](../.cursor/rules/mcp-down-stop.mdc)).

---

## Planning deliverables (due before heavy coding)

Write the plan to **`tmp/difficulty-system-plan.md`** (new file) containing:

1. **Current state** — what exists in `WBP_NewGame`, save game, related C++.
2. **Data model** — enum names, save field, default, migration for old saves (default Standard).
3. **UI spec** — wire names, row order, tooltips, Will strings.
4. **Outcome registry** — table schema + v1 row for `WorkerJobInterrupt` + stub rows for future keys.
5. **Integration map** — which systems call Difficulty (worker tasks when built, New Game on start).
6. **Phased tasks** — Phase A: save + subsystem + DataTable; Phase B: New Game UI; Phase C: worker consumer when worker queue exists; Phase D: tests / log verification.
7. **Open questions** for the user (only what you cannot infer).
8. **Design doc deltas** — propose edits to [Design/New Game.md](../Design/New%20Game.md) (rename Crisis Discipline → subsection under **Difficulty**); **do not edit `Design/` until user approves plan**.

End plan with a short **“Implement after approval”** checklist.

---

## Out of scope for this task

- Full worker task queue, raid phases, safe rooms (design only in todo docs).
- Balancing numbers beyond worker-interrupt formulas above.
- Merging Random Events or Randomness into Difficulty.
- Tutorial changes unless one line from Will on first New Game visit.

---

## Success criteria

- [ ] `tmp/difficulty-system-plan.md` exists and is reviewable.
- [ ] After approval: Difficulty selectable on New Game like Randomness; value survives Start Game into run save.
- [ ] `WorkerJobInterrupt` resolves via DataTable/registry, not copy-pasted switch in worker code.
- [ ] Adding a fake second outcome key in the table does not require widget changes.

---

## Voice for the user

They are an expert. Be terse in the plan. No high-level fluff. Show actual enum names, file paths, and table columns you will create.

**Start by writing the plan file, then stop and wait for user approval before implementation.**