# Typography

Target layout: **21:9** monitor. Telemetry and grid readouts prioritize legibility over ornament.

## 1. Main menu titles and UI headings

Sharp, high-contrast display faces for large labels (“New Game”, “Continue”)—crystal-matrix feel:

| Candidate | Role |
| --- | --- |
| **Friz Quadrata** | Sharp serifs, corporate-gothic weight; needle serifs like crystal cuts |
| **Cinzel / Cinzel Decorative** | Geometric Roman serif; Decorative adds flourishes for ley-line / celestial UI |
| **Percolator** | Heavy, sharp-cornered classic White Wolf–style headings; strong screen presence |

## 2. Technical data, counters, and grid overlays

Mana costs, coordinates, layout numbers—strict clarity first:

| Candidate | Role |
| --- | --- |
| **Orbitron** | Geometric sans; clean digital telemetry in a high-magic shell |
| **Rajdhani / Exo 2** | Condensed geometric sans for compact resource panels aligned to map UI |

## 3. Core lore, tooltips, and body copy

Paragraph-friendly serifs for descriptions and sheets:

| Candidate | Role |
| --- | --- |
| **Goudy Old Style / Palatino** | Editorial readability; old-school rulebook pairing with sharp titles |

---

## Shipped set (2026-10-29)

All **SIL OFL**. Raw TTFs: `DungeonCore/Content/UI/Fonts`. Loaded via `DungeonStoreStyle::FantasyCompositeFont()`.

| Tier | Licensed pick | Free replacement shipped | Composite typeface | Used for |
| --- | --- | --- | --- | --- |
| Titles | Friz Quadrata, Percolator | Cinzel Regular/Bold, Cinzel Decorative Bold | `Regular`, `Bold`, `Display` | Store/detail titles, list headings, game-over title, item names, run-setup headers |
| Telemetry | Orbitron | Orbitron Regular/Bold (from variable font) | `Telemetry`, `TelemetryBold` | Mana balance, income rate, item cost |
| Panel labels | Rajdhani / Exo 2 | Rajdhani Regular/SemiBold | `Panel`, `PanelBold` | Kickers (MANA, VAULT, CLOSE), tabs, footers, HUD (breach/wave/core), stats, controls hint |
| Body | Goudy Old Style / Palatino | Sorts Mill Goudy Regular/Italic | `Body`, `BodyItalic` | Store subtitle, item descriptions, item role line |

### Notes

- **Friz Quadrata** and **Percolator** are dropped, not substituted: Cinzel Bold = sharp serif titles; Cinzel Decorative Bold = heavy display. Closest free Friz stand-in if needed: **Marcellus** (OFL).
- **Exo 2** dropped; Rajdhani covers condensed panels alone.
- Sorts Mill Goudy has a smaller x-height than Roboto; body sizes bumped +2 (13→15, 12→14, 14→16) for the same optical size.
- Engine default **Roboto** is not used in project UI.

Title menu motion spec: [Start Screen.md](./Start%20Screen.md).
