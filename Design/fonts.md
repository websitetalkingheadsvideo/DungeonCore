Main Menu Titles & UI Headings

To mimic that sleek, ethereal crystal matrix look for the big text ("New Game", "Continue"), use a clean, sharp, or slightly stylized serif or high-contrast display font:
• Friz Quadrata: Highly recognizable, sharp-edged, and carries a classic corporate-gothic weight. It has subtle, needle-sharp serifs that mirror hard crystal cuts perfectly.
• Cinzel / Cinzel Decorative: A beautiful, modern geometric serif inspired by classical Roman proportions. The decorative variant adds elegant, floating flourishes that fit right into a celestial ley-line vibe.
• Percolator: A heavy, prominent, sharp-cornered font originally used for classic White Wolf Revised edition headings. It provides a punchy, ancient-yet-structured look that commands screen presence.

2. Technical Data, Resource Counters & Grid Overlays

When your game displays active layout numbers, grid coordinates, or crystal energy costs, the display must prioritize strict clarity over styling to remain readable on your 21:9 monitor layout:
• Orbitron: A sharp, geometric sans-serif that looks like clean, futuristic digital telemetry without feeling out-of-place in a high-magic setting.
• Rajdhani / Exo 2: Crisp, condensed geometric sans-serifs that keep resource panels compact, legible, and structurally aligned with a map builder layout.

3. Core / Lore Text & Descriptions

For tooltip boxes, block descriptions, or character sheets, you need an elegant but highly legible reading font that pairs well with sharp titles:
• Goudy Old Style / Palatino: Excellent, classic editorial serifs that are incredibly easy on the eyes for paragraphs of text while maintaining an old-school rulebook aesthetic.

---

Decision 2026-10-29 — shipped set (all SIL OFL, raw TTFs in `DungeonCore/Content/UI/Fonts`, loaded via `DungeonStoreStyle::FantasyCompositeFont()`)

| Tier | Licensed pick | Free replacement shipped | Composite typeface name | Used for |
| --- | --- | --- | --- | --- |
| Titles | Friz Quadrata, Percolator | Cinzel Regular/Bold, Cinzel Decorative Bold | `Regular`, `Bold`, `Display` | Store/detail titles, list headings, game-over title, item names, run-setup headers |
| Telemetry | Orbitron | Orbitron Regular/Bold (static instances from the variable font) | `Telemetry`, `TelemetryBold` | Mana balance, income rate, item cost |
| Panel labels | Rajdhani / Exo 2 | Rajdhani Regular/SemiBold | `Panel`, `PanelBold` | Kickers (MANA, VAULT, CLOSE), tabs, footers, HUD readouts (breach/wave/core), stats lines, controls hint |
| Body | Goudy Old Style / Palatino | Sorts Mill Goudy Regular/Italic | `Body`, `BodyItalic` | Store subtitle, item descriptions, item role line |

Notes
• Friz Quadrata and Percolator are dropped, not substituted: Cinzel Bold covers the sharp-serif title role and Cinzel Decorative Bold covers the heavy display role. If a closer Friz stand-in is ever wanted, Marcellus (OFL) is the nearest free match.
• Exo 2 dropped; Rajdhani alone covers the condensed-panel role.
• Sorts Mill Goudy has a smaller x-height than Roboto; body sizes were bumped +2 (13→15, 12→14, 14→16) to keep the same optical size.
• Engine default Roboto is no longer used anywhere in the UI.
