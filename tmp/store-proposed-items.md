## Proposed items

Ideas only, added 2026-11-16. None are built, none have art or icons, and prices are guesses set against the current list. Each one is here because it fills a gap the ten items above leave open.

### Power and economy

| Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | ---: | --- | --- |
| **Ley Conduit** | 25 | A crystal pylon that powers the room it stands in, so the room counts as **powered** without touching the core's own reach. | Mana Generators and Spore Generators are meant to work only in powered rooms. Without this, every working room has to sit next to the core. |
| **Mana Reservoir** | 35 | Raises the mana cap by 25 each. Invaders who break one spill its share of stored mana. | The cap is 100 and the dearest item is 50. A deeper dungeon needs somewhere to bank mana between waves, and it gives invaders a second thing worth attacking. |
| **Siphon Font** | 40 | Invaders who fall in this room give extra essence. | Rewards the player for choosing *where* the fight happens, which is the core loop of the genre ([Overall.md](./Overall.md)). |

### Traps and control

| Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | ---: | --- | --- |
| **Mirror Pillar** | 15 | A free-standing mirror that turns a Prism Beam 90 degrees. The way it faces sets the turn, so the player aims it with edit-mode rotation. | The Prism Beam "needs wall mirrors and sightlines" and nothing in the store supplies them. This is the missing half of that trap. |
| **Drag Lattice** | 20 | A floor lattice of slow light. Invaders crossing it move at half speed. Does no damage. | Every other trap hurts. A cheap slow makes the beam, the plate and the cart land more often, and gives new players an obvious first combo. |
| **Displacement Glyph** | 45 | Bought as a linked pair. An invader who steps on one is moved to the other. | Lets the player send a party back to the entrance, split it, or drop it into the trap room. One-way, so the order placed matters. |
| **Crystal Barricade** | 10 | A waist-high wall of crystal that blocks one square until invaders break it. Defenders can pass. | The cheapest way to buy time or bend a path. Nothing in the store currently changes where invaders can walk. |
| **Mirage Veil** | 30 | Makes one room edge look like solid rock to invaders. A scout or a party that has seen through one before ignores it. | Hides the mana room or the short way to the core. Pairs with Loot Bait: one item pulls invaders in, this one steers them away. |

### Defender support and warning

| Item | Price (mana) | What it would do | Why a dungeon wants it |
| --- | ---: | --- | --- |
| **Mending Node** | 30 | Slowly restores defenders and constructs standing in the room. | Golems and sentinels cost 50 and 25. Without a way to recover them, every fight is a straight loss of mana. |
| **Alarm Chime** | 15 | Rings when invaders enter its room. Defenders within a few squares come to it. | Defenders currently have no reason to move toward a fight they cannot see. Also the natural place for the "under attack" audio cue. |
| **Scrying Lens** | 20 | Clears fog of war in a wide ring around it and shows invaders inside that ring. | Fog only lifts two squares from dug floor. This is the only way to watch an approach the player has not dug up to. |
| **Decoy Core** | 60 | A dim copy of the core. Invaders treat it as the real one until they reach it. Bought once per run. | The core is the only thing that ends the run. A decoy is the classic late answer to a party that knows the way in. |

### Open questions for these

- **Colors:** the Ley Conduit, Scrying Lens and Decoy Core all glow. They must stay off the reserved colors in [General.md](./General.md) (teal, emerald, white-hot, red, violet), and the Decoy Core must not be mistaken for the real core's `CoreColor`.
- **Mirage Veil and Crystal Barricade** sit on a room edge or block a square. Today every item occupies one room square and never blocks movement, so both need a new placement rule.
- **Displacement Glyph** is the only item sold as a pair. Decide whether the second glyph is a second drag or placed automatically.
- **Alarm Chime vs Spore Generator:** spores put invaders to sleep. Decide whether a chime in the same room wakes them.
- **Decoy Core** overlaps with Loot Bait as a lure. Keep both only if the decoy is clearly the late, expensive version.

