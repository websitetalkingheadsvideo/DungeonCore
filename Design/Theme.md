# Theme

Four biome skins. Creature family is a separate choice ([Creatures and style.md](./Creatures%20and%20style.md)). A breached room swaps to that biome's kit. The core chamber takes the chosen theme too: picking a theme on the [New Game](./New%20Game.md) screen re-skins its walls, floor and ceiling and recolors its torch lights. It no longer keeps a fixed obsidian skin.

Every skin keeps one lighting treatment: strong key, high contrast, deep shadow. The color in the table is the room's point light. It is not `CoreColor`.

Assets below are in the project. Imported packs mount under `/Game/Environments/<Pack>/`. Cavern Mahal is the exception: `/Game/Environments/CavernMahal/`. Still absent: `MI_Floor_Tile_B`, `MI_Dirt_Floor_v01`, `MI_LargeWall_b`, `SM_Oil_Lamp`, `SM_ColosseumPillar_01`.

## Kit

| Theme | Light source | Wall | Floor | Ceiling |
| --- | --- | --- | --- | --- |
| **Gothic** | Wall lantern `SM_LanternWall` (25 × 28 × 45 UU, Megascans "Lantern Wall Metal Old") on `M_LanternWall`, with flame card `SM_CandleFire_Plane_B` / `MI_CandleFire` inside. Oil-lamp gold `(1.0, 0.78, 0.45)`. `SM_LightCandle_01` is a table candle — not for walls | `MI_NG_Wall` — instance of `M_WA_Inst` (`T_Cobblestone`) | `MI_NG_Floor` — instance of `M_WA_Inst4` | `MI_NG_Wall` — same material as the wall |
| **Goblin Tunnels** | Sconce `SM_Torch_01` (30 × 34 × 68 UU). Assign `MI_Torch_Emissive` — the mesh currently wears `MI_AssetsBook`. Torch amber `(1.0, 0.55, 0.2)` | `MI_Wall_Mine_Rock_01_A` — `T_Wall_Mine_Rock_01` | `MI_Floor_Mine_Coal_01` — `T_Floor_Mine_Coal_01` | `MI_Wall_Mine_Rock_01_B` — same rock, darker soffit. Timber is structure, not the ceiling texture |
| **Fungal Warren** | `SM_LargeMushrooms_a`. Emissive retinted spore green `(0.35, 1.0, 0.45)`. `SM_GlowingPlant_a` stays rejected — its material references a missing `MI_Atlas` | `M_WA_Inst2` — `T_Stone4` blended with `T_Stone_Moss` / `T_Stone_Moss2` | `M_TileableAsset_Irregularstone` — `T_GroundIrregularStones` | `M_TileableAsset_Inst7` — `T_Stone_Moss2`. Hang `SM_Cobweb_01a` (`M_Web`) from it |
| **Sunken Ruins** | Shaft `SM_Godray_Cone` / `MI_Godray_Inst`. Native size 1696 × 1696 × 3036 UU — scale it into the 300 UU room. Teal `(0.2, 0.9, 0.8)`. Room light only; default `CoreColor` stays deep blue or seafoam-silver | `MI_blind_Arch_1` — `T_Dungeon_Blind_Arch` (arch course). Flat field: `M_WA_Inst` — `T_Cobblestone` plus puddle normal `T_Puddle_N` | Open water `MI_Water_Inst` on `SM_WaterFloor_01`. Silt edge `MI_Sewer_Mud_01a` — `T_Dungeon_Mud_01a` | `M_WA_Inst` — wet cobble vault, same puddle normal, no arch relief |

Paths:

| Asset | Path |
| --- | --- |
| `SM_LanternWall` | `/Game/Props/LanternWall/SM_LanternWall` |
| `M_LanternWall` | `/Game/Props/LanternWall/M_LanternWall` (textures `T_LanternWall_BaseColor`, `_Normal`, `_Roughness`, `_AO` beside it) |
| `SM_LightCandle_01` | `/Game/Environments/AncientCrypt/Meshes/SM_LightCandle_01` |
| `SM_CandleFire_Plane_B` | `/Game/Environments/AncientCrypt/Meshes/SM_CandleFire_Plane_B` |
| `MI_CandleFire` | `/Game/Environments/AncientCrypt/Materials/MI_CandleFire` |
| `SM_Candelabro_01` | `/Game/Environments/AncientCrypt/Meshes/Props/SM_Candelabro_01` |
| `MI_NG_Wall` | `/Game/Maps/Materials/MI_NG_Wall` |
| `MI_NG_Floor` | `/Game/Maps/Materials/MI_NG_Floor` |
| `M_WA_Inst4` | `/Game/Environments/AncientCrypt/Materials/M_WA_Inst4` |
| `M_WA_Inst6` | `/Game/Environments/AncientCrypt/Materials/M_WA_Inst6` |
| `SM_Main_Room_Roof_01` | `/Game/Environments/AncientCrypt/Meshes/Modular/SM_Main_Room_Roof_01` |
| `SM_Torch_01` | `/Game/Environments/AncientCrypt/Meshes/SM_Torch_01` |
| `MI_Torch_Emissive` | `/Game/Environments/AncientCrypt/Materials/MI_Torch_Emissive` |
| `MI_Wall_Mine_Rock_01_A` | `/Game/Environments/Scene_OldMine/Assets/Megascans/Surfaces/Wall_Mine_Rock_01/MI_Wall_Mine_Rock_01_A` |
| `MI_Wall_Mine_Rock_01_B` | `/Game/Environments/Scene_OldMine/Assets/Megascans/Surfaces/Wall_Mine_Rock_01/MI_Wall_Mine_Rock_01_B` |
| `MI_Floor_Mine_Coal_01` | `/Game/Environments/Scene_OldMine/Assets/Megascans/Surfaces/Floor_Mine_Coal_01/MI_Floor_Mine_Coal_01` |
| `M_WA_Inst2` | `/Game/Environments/AncientCrypt/Materials/M_WA_Inst2` |
| `M_TileableAsset_Irregularstone` | `/Game/Environments/AncientCrypt/Materials/M_TileableAsset_Irregularstone` |
| `M_TileableAsset_Inst7` | `/Game/Environments/AncientCrypt/Materials/M_TileableAsset_Inst7` |
| `SM_Cobweb_01a` | `/Game/Environments/AncientCrypt/Meshes/SM_Cobweb_01a` |
| `SM_Godray_Cone` | `/Game/Environments/AncientCrypt/Meshes/SM_Godray_Cone` |
| `MI_Godray_Inst` | `/Game/Environments/AncientCrypt/Materials/MI_Godray_Inst` |
| `MI_blind_Arch_1` | `/Game/Environments/AncientCrypt/Materials/MI_blind_Arch_1` |
| `M_WA_Inst` | `/Game/Environments/AncientCrypt/Materials/M_WA_Inst` |
| `MI_Water_Inst` | `/Game/Environments/AncientCrypt/Materials/MI_Water_Inst` |
| `SM_WaterFloor_01` | `/Game/Environments/AncientCrypt/Meshes/Modular/SM_WaterFloor_01` |
| `MI_Sewer_Mud_01a` | `/Game/Environments/AncientCrypt/Materials/MI_Sewer_Mud_01a` |

## Per theme

**Gothic.** Cathedral stone, no moss. Floor candelabra `SM_Candelabro_01` is 221 × 221 × 196 UU if a room needs a standing source as well as the wall candle. `SM_LightCandle_01` is also on `MI_AssetsBook` today; the flame is the card, not that atlas.

**Goblin Tunnels.** Raw rock and coal. Timber supports: `SM_Ind_Min_Beam_Round_Wood_Worn_01`, `SM_Ind_Min_Beam_Square_Wood_Worn_03`, `SM_Ind_Min_Tunnel_Wall_Log_Modular_01` under `/Game/Environments/Scene_OldMine/Assets/Megascans/3D_Assets/`.

**Fungal Warren.** Moss on cut stone, irregular floor, moss ceiling. Wall ivy: `SM_Ivy_Wall_A`–`D` with `MI_Ivy_A`. Moss decal: `M_MossDecal`. The green source is `SM_LargeMushrooms_a`.

**Sunken Ruins.** Water on the floor, arch masonry on the wall, wet cobble overhead. `MI_Water_Inst1` is the second water instance if a room needs a variant.

## Props

Same gameplay object, four shells. Door, archway, chest, mana generator, and monster generator each occupy one face or one 300 UU cube. Floor debris scatters inside the cube and does not block the grid.

Door and archway are the threshold. The door seals it. The archway does not. Open behavior is shared ([General.md](./General.md)): the leaves part and an energy sheet fills the gap.

Chest is invader bait ([Future.md](./Future.md)). Mana generator ticks essence in a powered room. Monster generator is only the spawner housing.

Where the cell says **none**, that mesh is not in the project.

| | Gothic | Goblin Tunnels | Fungal Warren | Sunken Ruins |
| --- | --- | --- | --- | --- |
| **Treasure chest** | **None.** No coffer, reliquary, or sarcophagus. Do not use the tomb (that is the spawner) or the altar (that is the mana generator) | `SM_Chest` with `MI_Chest`. Timber dungeon chest. Viking `SM_Chest_01` is the alternate if this one reads as a room module | **None.** No pod or sac. Do not use the cobweb (spawner) or the large mushroom (light) | `SM_ChestMd` with `MI_ChestMd`. Ruin stone chest. Confirm it is not stylized. Urn `SM_Vase_02` stays debris |
| **Floor debris** | `SM_Skull`, `SM_Bone_01`, `SM_Bone_02`, `SM_Vase_01_Damaged`. Cathedral `SM_BushDebris01` is outdoor shrub — skip | `SM_Ind_Rub_Debris_Board_Wood_Old_02`, `SM_Ind_Aba_Supplies_Plank_Wood_Worn_01`, `SM_For_Tro_Rock_S_03`, `SM_Ind_Min_Cart_Wheel_Metal_Old_01` | `SM_SmallMushrooms_a`, `SM_GroundMushrooms_a`, plus `SM_Cobweb_01a` and `M_MossDecal` | `SM_RubblesKit_01`, plus `SM_Dungeon_Mud_01a` and `SM_ceramic000` |
| **Door** | `SM_Entrance_Door`. `SM_Entrance_Door_Big_Asset` is the full facade — too big for one cube | `SM_Door_01` + `SM_Frame_Door_01` (Medieval Village). Plank house door; drop the village frame if it reads domestic and seat the leaf in the mine arch | Web sheet `SM_Cobweb_01a` over the energy sheet. **No door mesh** | `SM_Door_01` (Mayan Cave). Stone slab, distinct from the cathedral door. Sill stays `SM_WaterFloor_01` |
| **Archway** | `SM_Entrance_Module`, flanked by `SM_Pillar_A`. If the module is a whole facade, use a pair of `SM_Pillar_A`. Skip `SM_ArchedFlowerPort` | `SM_MineArch`. Replaces the beam kitbash | `SM_LargeArch` with `SM_Ivy_Wall_A` on the crown | `SM_EntranceArch_01` (Cavern Mahal). Water sill `SM_WaterFloor_01` |
| **Mana generator** | `SM_Altar_A`. Pulse oil-lamp gold. Optional glow: `SM_CrystalCluster_D_01` on `MI_GlowCrystalls`, retinted gold. Not purple, not red | `SM_Crystalls_A` in the timber crib, emissive torch amber. Not `MI_Crystalls_Red` | `SM_MiddleMushrooms_a`. Emissive spore green. The large mushroom is the light, not this | Drowned font. `SM_WaterFloor_01` plus a scaled-down `SM_Godray_Cone`. Pulse teal. Room light only, not `CoreColor`. Optional crystal: `SM_SingleCrystal_Blue_a`, retinted off a CoreColor teal |
| **Monster generator** | Sarcophagus `SM_Tomb_Crypt_Damage_Var_01` (damaged vars 02–04 are the opened states) | `SM_Cage01`. Replaces the stake pen | Cocoon. A cluster of `SM_Cobweb_01a`. The creature tears out | Rising pool. `SM_WaterFloor_01`. The creature surfaces |

Fit gates, arches, the tomb, and the ritual table to the 300 UU cube. Do not use their import scale. The godray's import size is 1696 × 1696 × 3036 UU.

Paths:

| Asset | Path |
| --- | --- |
| `SM_Vase_02` | `/Game/Environments/AncientCrypt/Meshes/Props/SM_Vase_02` |
| `SM_Vase_02_Damaged` | `/Game/Environments/AncientCrypt/Meshes/Props/SM_Vase_02_Damaged` |
| `SM_Vase_01_Damaged` | `/Game/Environments/AncientCrypt/Meshes/Props/SM_Vase_01_Damaged` |
| `SM_Skull` | `/Game/Environments/AncientCrypt/Meshes/Props/SM_Skull` |
| `SM_Bone_01` | `/Game/Environments/AncientCrypt/Meshes/Props/SM_Bone_01` |
| `SM_Bone_02` | `/Game/Environments/AncientCrypt/Meshes/Props/SM_Bone_02` |
| `SM_Gate` | `/Game/Props/Gothic_props_1/Static_meshes/SM_Gate` |
| `SM_Arc` (Gothic props) | `/Game/Props/Gothic_props_1/Static_meshes/SM_Arc` |
| `SM_Column1` | `/Game/Props/Gothic_props_1/Static_meshes/SM_Column1` |
| `SM_Gate_01` | `/Game/Environments/AncientCrypt/Meshes/Modular/SM_Gate_01` |
| `SM_Arc` (crypt) | `/Game/Environments/AncientCrypt/Meshes/Modular/SM_Arc` |
| `SM_Main_Room_Arc_Wall_01` | `/Game/Environments/AncientCrypt/Meshes/Modular/SM_Main_Room_Arc_Wall_01` |
| `SM_RitualStoneTable_low` | `/Game/Environments/AncientCrypt/Meshes/SM_RitualStoneTable_low` |
| `SM_Candle_low` | `/Game/Environments/AncientCrypt/Meshes/SM_Candle_low` |
| `SM_Tomb_Crypt_Damage_Var_01` | `/Game/Environments/AncientCrypt/Meshes/SM_Tomb_Crypt_Damage_Var_01` |
| `SM_ceramic000` | `/Game/Environments/AncientCrypt/Meshes/Props/SM_ceramic000` |
| `SM_Dungeon_Mud_01a` | `/Game/Environments/AncientCrypt/Meshes/SM_Dungeon_Mud_01a` |
| `SM_InteriorBush_LineGround` | `/Game/Environments/AncientCrypt/Meshes/Foliage/SM_InteriorBush_LineGround` |
| `SM_Ivy_Wall_A` | `/Game/Environments/AncientCrypt/Meshes/Foliage/SM_Ivy_Wall_A` |
| `M_MossDecal` | `/Game/Environments/AncientCrypt/Materials/Masters/M_MossDecal` |
| `SM_Ind_Rub_Debris_Board_Wood_Old_02` | `/Game/Environments/Scene_OldMine/Assets/Megascans/3D_Assets/Ind_Rub_Debris_Board_Wood_Old_02/SM_Ind_Rub_Debris_Board_Wood_Old_02` |
| `SM_Ind_Aba_Supplies_Plank_Wood_Worn_01` | `/Game/Environments/Scene_OldMine/Assets/Megascans/3D_Assets/Ind_Aba_Supplies_Plank_Wood_Worn_01/SM_Ind_Aba_Supplies_Plank_Wood_Worn_01` |
| `SM_For_Tro_Rock_S_03` | `/Game/Environments/Scene_OldMine/Assets/Megascans/3D_Assets/For_Tro_Rock_S_03/SM_For_Tro_Rock_S_03` |
| `SM_Ind_Min_Cart_Wheel_Metal_Old_01` | `/Game/Environments/Scene_OldMine/Assets/Megascans/3D_Assets/Ind_Min_Cart_Wheel_Metal_Old_01/SM_Ind_Min_Cart_Wheel_Metal_Old_01` |
| `SM_Ind_Min_Beam_Square_Wood_Worn_03` | `/Game/Environments/Scene_OldMine/Assets/Megascans/3D_Assets/Ind_Min_Beam_Square_Wood_Worn_03/SM_Ind_Min_Beam_Square_Wood_Worn_03` |
| `SM_Ind_Min_Beam_Round_Wood_Worn_01` | `/Game/Environments/Scene_OldMine/Assets/Megascans/3D_Assets/Ind_Min_Beam_Round_Wood_Worn_01/SM_Ind_Min_Beam_Round_Wood_Worn_01` |
| `SM_Ind_Min_Equipment_Stakes_Metal_Pack_01` | `/Game/Environments/Scene_OldMine/Assets/Megascans/3D_Assets/Ind_Min_Equipment_Stakes_Metal_Pack_01/SM_Ind_Min_Equipment_Stakes_Metal_Pack_01` |

Imported pack paths:

| Asset | Path |
| --- | --- |
| `SM_Entrance_Door` | `/Game/Environments/GothicCathedral/Meshes/SM_Entrance_Door` |
| `SM_Entrance_Module` | `/Game/Environments/GothicCathedral/Meshes/SM_Entrance_Module` |
| `SM_Altar_A` | `/Game/Environments/GothicCathedral/Meshes/SM_Altar_A` |
| `SM_Pillar_A` | `/Game/Environments/GothicCathedral/Meshes/SM_Pillar_A` |
| `SM_Chest` | `/Game/Environments/FantasyDungeon/Meshes/SM_Chest` |
| `MI_Chest` | `/Game/Environments/FantasyDungeon/Materials/MI_Chest` |
| `SM_Cage01` | `/Game/Environments/FantasyDungeon/Meshes/SM_Cage01` |
| `SM_Door_01` (village) | `/Game/Environments/MedievalVillage/Meshes/House/SM_Door_01` |
| `SM_Frame_Door_01` | `/Game/Environments/MedievalVillage/Meshes/House/SM_Frame_Door_01` |
| `SM_MineArch` | `/Game/Environments/GreenCave/Meshes/SM_MineArch` |
| `SM_LargeArch` | `/Game/Environments/GreenCave/Meshes/SM_LargeArch` |
| `SM_LargeMushrooms_a` | `/Game/Environments/GreenCave/Meshes/SM_LargeMushrooms_a` |
| `SM_MiddleMushrooms_a` | `/Game/Environments/GreenCave/Meshes/SM_MiddleMushrooms_a` |
| `SM_SmallMushrooms_a` | `/Game/Environments/GreenCave/Meshes/SM_SmallMushrooms_a` |
| `SM_GroundMushrooms_a` | `/Game/Environments/GreenCave/Meshes/SM_GroundMushrooms_a` |
| `SM_Crystalls_A` | `/Game/Environments/Crystal_Cave/Meshes/SM_Crystalls_A` |
| `SM_CrystalCluster_D_01` | `/Game/Environments/Crystal_Cave/Meshes/SM_CrystalCluster_D_01` |
| `MI_GlowCrystalls` | `/Game/Environments/Crystal_Cave/Materials/MI_GlowCrystalls` |
| `SM_SingleCrystal_Blue_a` | `/Game/Environments/GreenCave/Meshes/SM_SingleCrystal_Blue_a` |
| `SM_EntranceArch_01` | `/Game/Environments/CavernMahal/Meshes/SM_EntranceArch_01` |
| `SM_Door_01` (Mayan) | `/Game/Environments/Mayan_Cave/Meshes/SM_Door_01` |
| `SM_RubblesKit_01` | `/Game/Environments/Mayan_Cave/Meshes/SM_RubblesKit_01` |
| `SM_ChestMd` | `/Game/Environments/JungleRuins/Meshes/SM_ChestMd` |
| `MI_ChestMd` | `/Game/Environments/JungleRuins/Materials/MI_ChestMd` |

## Rejected

| Asset | Why |
| --- | --- |
| `SM_Ind_War_Light_Bulkhead_Metal_Caged_01` | Caged industrial lamp. Wrong light for Goblin Tunnels |
| `SM_Lamp_Wall`, `SM_Lamp_Ceiling` | Starter-content lamps under Medieval Weapons. Not an aqueduct fixture |
| `MI_Tile_Ceiling`, `M_Glow`, `M_Glow_Ceiling`, `M_Glow_Ceiling_Cold` | Old Mine demo-room show gloss |
| `MI_AssetsBook` on `SM_Torch_01` and `SM_LightCandle_01` | Shared prop atlas, not the emissive |
