
🌌 The Idle State (The Unpowered Core)

When the game first launches, the environment is dead silent and cast in deep, graphic shadows.
• The Nexus Core: A massive, unpolished geode floats in the absolute center of a dark basalt chamber. It rotates on a slow, continuous vertical axis. Only a faint, moody amethyst or sapphire glimmer pulses deep within its crystal fractures, casting soft ambient light across the empty obsidian floor tiles.
• The Text Menu: The options (Continue, New, Load, Options) appear as crisp, translucent, sharp-bordered text elements floating to the left of the core, matching an in-game tactical HUD layout.

✨ Dynamic Hover States (Sensing the Network)

Moving your mouse cursor over each option sends a small shockwave of energy through the room, providing a visual sneak peek of what that choice means:
• Hovering over New (The Spark of Creation):
	• The central geode suddenly locks onto its axis and spins faster.
	• A single, bright capillary line of liquid light shoots from the base of the core, tearing across the obsidian floor toward the front wall.
	• The wall blocks hit by the light hairline-fracture into a faint, glowing geometric grid pattern, symbolizing an uncarved canvas waiting for your first worker.
• Hovering over Continue / Load (The Memory Pulse):
	• The core emits a slow, expanding horizontal ring of pale teal light that sweeps across the entire room.
	• As the ring passes, it momentarily illuminates a phantom, wireframe holographic overlay of a sprawling dungeon layout mapped out on the dark floor and walls. It fades after a second, teasing your last active layout.
• Hovering over Options (The Calibration):
	• The core shifts its light from a deep amethyst to a stable, industrial emerald green.
	• Two smaller, sharp-edged satellite crystal shards orbital-lock onto the main core and begin spinning around it like a calibration mechanism, reacting smoothly to your mouse movements.

💥 The Click Execution (The Chain Reaction)

Clicking an option triggers an un-skippable, instantaneous visual transition that flows seamlessly straight into the gameplay or interface screens:
• Clicking New Game: The core flares with blinding, white-hot ley-energy. A heavy acoustic shatter echo plays as the front wall completely fractures into dust, instantly pulling the camera forward through the newly carved breach and dropping you directly into the active UI of your opening room. No loading screen—just an immediate transition into your first dig command.
• Clicking Load or Options: The main core drifts backward into the darkness, scaling down smoothly while a secondary stone or crystal menu slate glides into the foreground from the right side of the screen to display your save files or settings toggles without breaking the scene.

## New Game setup screen — implemented (2026-10-29)

- Hero shot: the real 3×3 core chamber (`ADungeonShowcaseActor`), camera pitch −28°, yaw 0, arm 1300 UU, target offset (0, −340, +60) from chamber centre so the core sits ~63 % across. Camera-side (min-X) wall is left open as a cutaway.
- Card is left-anchored (56 px inset, ≤ ~30 % of a 1920 frame); tiles are a 2×2 grid, 190×54. A 55 % alpha scrim covers only the left 30 % of the frame.
- Defaults: theme Gothic, family Orcs. Clicking a tile re-skins floor/wall materials, the four corner accent meshes and the fill-light colour in place; clicking a family swaps the creature on block (1,2).
- Theme kits: Gothic = GothicCathedral `MI_Floor_Tile_B` / `MI_Stone_Tile_A` / `SM_Pillar_A`, oil-lamp gold (1.0, 0.78, 0.45). Goblin Tunnels = FantasyDungeon `MI_Dirt_Floor_v01` / GreenCave `MI_LargeWall_b` / `SM_SupportBeam`, torch amber (1.0, 0.55, 0.2). Fungal Warren = GreenCave `MI_FlatStone_a` / `MI_LargeWall_b` / Crystal_Cave `SM_CrystalCluster_D_01` @160 UU, spore green (0.35, 1.0, 0.45). Sunken Ruins = `MI_FlatStone_a` / `MI_Stone_Tile_A` / CavernMahal `SM_ColosseumPillar_01` (circular), teal (0.2, 0.9, 0.8). Accents 250 UU unless noted.
- Wall fixtures (one per standing wall; camera-side wall is the cutaway): Gothic `SM_Oil_Lamp` @50 UU tall, mount 155, 45 cd. Goblin `SM_Torch` @70 UU, mount 150, 50 cd. Fungal `SM_LargeMushrooms_a` @110 UU, floor-seated (mount 0), 35 cd. Sunken `SM_Lamp_A` (Ashes of Bazaar) @55 UU, mount 40, 40 cd. Each fixture has its own point light in `AccentLightColor`. Lost_Cave `SM_GlowingPlant_a` was rejected: source pack refs a missing `MI_Atlas`.
- Creature art is pending (see `TODO.md`); until installed the card shows "Creature art pending — Fab: <pack>" under the summary.