# Future Update Possibilities

Ideas parked for later. Not in current scope.

## Core crystal gameplay affinity

**Current state:** Core crystal color is cosmetic only (see [General.md](./General.md)).

**Idea:** Tie the core's color to a mana affinity (e.g. fire, frost, void) that affects gameplay.

Possible effects:

- Affinity-flavored traps and constructs (e.g. frost Prism Beam slows, fire Resonance Plate burns)
- Monster buffs or evolutions keyed to affinity
- Intruders with resistances or weaknesses, so affinity choice matters per run

**Design notes if adopted:**

- Plan the curated color palette around the affinities so each color maps to one.
- Keep reserved colors free: teal (zone states), emerald (Options), white-hot (under attack), red (warnings), violet (Will / guidance).
- Existing saves need a default affinity for their cosmetic color.

## Ore veins

**Idea:** Workers digging into fog of war can hit ore veins (Old Mine kit). A vein gives a visible reward: a burst of mana, a rare material, or a pattern unlock.

- Veins make blind digging a risk/reward choice alongside breaches.
- Could tie into absorb-to-unlock if that loop is adopted.

## Rails as worker routes

**Idea:** The player builds rail lines; carts move workers or resources faster along them.

- Shares rail infrastructure with the Runaway Cart trap ([General.md](./General.md)), so one rail network serves both logistics and defense.
- Invaders could sabotage or ride the rails.

## Mana generators

**Idea:** Placeable objects that generate essence over time inside powered rooms.

- Asset candidates (Fab library): **FREE Stylized Glowing Stones** or **Enchanted Crystal 02**.
- Retexture the stones toward the grounded look; retint emissive so the glow avoids reserved colors (teal, emerald, white-hot, red, violet).
- Gives invaders a secondary target besides the core.

## Invader rewards

**Idea:** Loot placed in a room as a reward for invaders who reach it—bait that draws stronger parties (the genre's loot-as-bait balance, [Overall.md](./Overall.md)).

- Asset candidate: **Enchanted Dual-Core Fantasy Sword**.
- Ties to the deferred tutorial beat "first loot placed as bait" ([Tutorial.md](./Tutorial.md)).

## Found locations

**Idea:** Rare pre-existing places the dungeon breaks into while digging. Self-contained pockets, so they don't have to match the surrounding biome.

- Asset candidates (Humble Ultimate Cyber November Game Dev Bundle, not yet bought): **Demonic Village Environment**, **Forgotten Echoes Environment**. Possible extras from the same bundle: **The Grand Ossuary**, **Merlin's Cave**, **Sorcerer's Hut**.
- Carve a pocket a few grid cubes wide (300 uu cubes), not the whole environment.
- Retint lights and emissives away from reserved colors (teal, emerald, white-hot, red, violet); Demonic Village likely glows red.
- Open question: what a find does (reward such as mana, a relic, or a new creature type; a hazard that wakes up; lore). This decides how often they appear.
## New creature families: Plants, Demons, Gargoyles

**Idea:** Possible future families beyond the current six. All candidates are in the Fab library (added Nov 2026), not yet in the project. Check fidelity against the art direction ([Monsters.md](./Monsters.md)); stylized or low-poly picks need retexturing or get rejected.

| Family | Candidates |
| --- | --- |
| **Plants** | Thornvine Pouncer, Helianthus Stalker, Bramble Thornweaver |
| **Demons** | Indian Monster Demon (Rakshasa), Abyssal Reaper (free demonic wraith boss, idle anim), Giang Demon_19 |
| **Gargoyles / constructs** | Gargoyle Statue (already downloaded to `F:\Epic`, OBJ; could be dressing or a statue that wakes as an elite), Cobblestone Golem, Amethyst Geode Beast, Amethyst_Guardian, Free Stylized Elementals (low poly) |

**Status:** try these when there's time.

- **Crystal family (Andy likes this one):** **Amethyst Geode Beast** as boss, **Amethyst_Guardian** as elite. Worker and minion still needed: much smaller crystal creatures (scaled-down versions or separate assets). Tint TBD, since violet is reserved for Will; candidates are smoky quartz or blue-grey.
- **Fungi Infested Knight:** Andy's pick for the **Fungal** family elite (added to [Monsters.md](./Monsters.md)).

### Other new library items worth a look
- **Spiders - characters with animations:** may fill the Spiders family in one pack (worker/minion/elite gap).
- **Mermaid Warrior:** Sunken Ruins resident or found-location guardian.
- **Armored Troll (free, with sword)**, **Mossy Swamp Troll:** Orcs-family brute or a future Trolls family.
- **Warrior Queen (rigged)**, **Fantasy RPG Humanoid Pack:** intruder heroes.
- **Loot for [Invader rewards](#invader-rewards):** Leather Treasure Chest, Vintage Wooden Treasure Chest, Realistic PBR Treasure Chest, Hoard: coin stack, Chalice, Bottled Wonders, Free Stylized Dark Sword, 20 Unique Textures Vol.8 Fantasy Treasure.
- **Audio:** Fantasy RPG & Dungeon Crawler, 200 free SFX (magic, blades, loot).