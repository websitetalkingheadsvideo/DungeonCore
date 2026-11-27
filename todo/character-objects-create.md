# Character objects to create

**Blocked:** Hi3D credits exhausted — defer **Hi3D generate** rows until credits reset (~couple weeks).
**Sheets:** [character-sheets.json](../Data/character-sheets.json) · rig notes: [Monsters-11-23-26.md](./Monsters-11-23-26.md)

Legend: `in_ue` mesh path exists in project · `missing` empty `MeshRef` · `rig` in UE but needs AccuRIG/idle fix · `vault` Epic Paragon — migrate via launcher only

## Priority — Common line (no mesh yet)

| SheetId | Display | Pipeline | Target `/Game/` path | Status |
| --- | --- | --- | --- | --- |
| `Invader.Wanderer.Common` | Wanderer | Fab / Hi3D | `Invaders/Wanderer/` | missing · blocked_hi3d |
| `Invader.Thief.Common` | Thief | Fab Medieval Assassin Girl | `Invaders/Thief/` | missing · Fab |
| `Invader.Champion.WarriorQueen` | Warrior Queen | Hi3D / CC (rigged) | `Invaders/WarriorQueen/` | missing · blocked_hi3d |
| `Invader.Cleric.FireflySlot` | Firefly Cleric (optional) | Hi3D / Firefly | `Invaders/Cleric/` | missing · use **Zinx** until ready |

## Firefly party (sheets exist — object work)

| SheetId | MeshRef | Status |
| --- | --- | --- |
| `Invader.DwarfFighter.Common` | `/Game/Invaders/Dwarf/Dwarf_Fighter` | in_ue · party template `FireflyFour` |
| `Invader.ElfArcher.Common` | `/Game/Invaders/Elf-Archer/Elf_Archer` | in_ue · party template `FireflyFour` |
| `Invader.Thelma.Common` | `/Game/Invaders/Thelma/Invader_Thelma` | in_ue · party template `FireflyFour` |
| `Invader.Wizard.Common` | `/Game/Invaders/Wizard/Invader_Wizard_Unreal` | in_ue · party template `FireflyFour` |
| `Invader.ElfMage.Common` | `/Game/Invaders/Elf-Mage/Elf_Mage` | in_ue · party template `FireflyFour` |

## Defenders — all tiers (rig / scale)

See [Monsters-11-23-26.md](./Monsters-11-23-26.md) per family. Workers need `WorkerRole` on sheet (`Dig` / `Build` / `Both`).

## Paragon invaders — vault → UE

| SheetId | Display | Suggested migrate path | MeshRef on sheet |
| --- | --- | --- | --- |
| `Invader.Archer.Drongo` | Drongo | `/Game/Invaders/ParagonDrongo` | `/Game/Invaders/ParagonDrongo` · guess_path |
| `Invader.Archer.Murdock` | Murdock | `/Game/Invaders/ParagonMurdock` | `/Game/Invaders/ParagonMurdock` · guess_path |
| `Invader.Archer.Sparrow` | Sparrow | `/Game/Invaders/ParagonSparrow` | `/Game/Invaders/ParagonSparrow` · guess_path |
| `Invader.Archer.TwinBlast` | TwinBlast | `/Game/Invaders/ParagonTwinBlast` | `/Game/Invaders/ParagonTwinBlast` · guess_path |
| `Invader.Assassin.Countess` | Countess | `/Game/Invaders/ParagonCountess` | `/Game/Invaders/ParagonCountess` · guess_path |
| `Invader.Assassin.Kallari` | Kallari | `/Game/Invaders/ParagonKallari` | `/Game/Invaders/ParagonKallari` · guess_path |
| `Invader.Assassin.Revenant` | Revenant | `/Game/Invaders/ParagonRevenant` | `/Game/Invaders/ParagonRevenant` · guess_path |
| `Invader.Assassin.Serath` | Serath | `/Game/Invaders/ParagonSerath` | `/Game/Invaders/ParagonSerath` · guess_path |
| `Invader.Assassin.Shinbi` | Shinbi | `/Game/Invaders/ParagonShinbi` | `/Game/Invaders/ParagonShinbi` · guess_path |
| `Invader.Assassin.Yin` | Yin | `/Game/Invaders/ParagonYin` | `/Game/Invaders/ParagonYin` · guess_path |
| `Invader.Bruiser.Crunch` | Crunch | `/Game/Invaders/ParagonCrunch` | `/Game/Invaders/ParagonCrunch` · guess_path |
| `Invader.Bruiser.FengMao` | Feng Mao | `/Game/Invaders/ParagonFengMao` | `/Game/Invaders/ParagonFengMao` · guess_path |
| `Invader.Bruiser.Khaimera` | Khaimera | `/Game/Invaders/ParagonKhaimera` | `/Game/Invaders/ParagonKhaimera` · guess_path |
| `Invader.Bruiser.Kwang` | Kwang | `/Game/Invaders/ParagonKwang` | `/Game/Invaders/ParagonKwang` · guess_path |
| `Invader.Bruiser.Wukong` | Wukong | `/Game/Invaders/ParagonWukong` | `/Game/Invaders/ParagonWukong` · guess_path |
| `Invader.Champion.Terra` | Terra | `/Game/Invaders/ParagonTerra` | `/Game/Invaders/ParagonTerra` · guess_path |
| `Invader.Cleric.Muriel` | Muriel | `/Game/Invaders/ParagonMuriel` | `/Game/Invaders/ParagonMuriel` · guess_path |
| `Invader.Cleric.Zinx` | Zinx | `/Game/Invaders/ParagonZinx` | `/Game/Invaders/ParagonZinx` · guess_path |
| `Invader.Mage.Aurora` | Aurora | `/Game/Invaders/ParagonAurora` | `/Game/Invaders/ParagonAurora` · guess_path |
| `Invader.Mage.Dekker` | Dekker | `/Game/Invaders/ParagonDekker` | `/Game/Invaders/ParagonDekker` · guess_path |
| `Invader.Mage.GRIMexe` | GRIM.exe | `/Game/Invaders/ParagonGRIM.exe` | `/Game/Invaders/ParagonGRIMexe` · guess_path |
| `Invader.Mage.Gadget` | Gadget | `/Game/Invaders/ParagonGadget` | `/Game/Invaders/ParagonGadget` · guess_path |
| `Invader.Mage.Gideon` | Gideon | `/Game/Invaders/ParagonGideon` | `/Game/Invaders/ParagonGideon` · guess_path |
| `Invader.Mage.Howitzer` | Howitzer | `/Game/Invaders/ParagonHowitzer` | `/Game/Invaders/ParagonHowitzer` · guess_path |
| `Invader.Mage.IggyScorch` | Iggy & Scorch | `/Game/Invaders/ParagonIggyScorch` | `/Game/Invaders/ParagonIggyScorch` · guess_path |
| `Invader.Mage.LtBelica` | Lt. Belica | `/Game/Invaders/ParagonLtBelica` | `/Game/Invaders/ParagonLtBelica` · guess_path |
| `Invader.Mage.Morigesh` | Morigesh | `/Game/Invaders/ParagonMorigesh` | `/Game/Invaders/ParagonMorigesh` · guess_path |
| `Invader.Mage.Phase` | Phase | `/Game/Invaders/ParagonPhase` | `/Game/Invaders/ParagonPhase` · guess_path |
| `Invader.Mage.Wraith` | Wraith | `/Game/Invaders/ParagonWraith` | `/Game/Invaders/ParagonWraith` · guess_path |
| `Invader.Tank.Greystone` | Greystone | `/Game/Invaders/ParagonGreystone` | `/Game/Invaders/ParagonGreystone` · guess_path |
| `Invader.Tank.Rampage` | Rampage | `/Game/Invaders/ParagonRampage` | `/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage` · guess_path |
| `Invader.Tank.Riktor` | Riktor | `/Game/Invaders/ParagonRiktor` | `/Game/Invaders/ParagonRiktor` · guess_path |
| `Invader.Tank.Steel` | Steel | `/Game/Invaders/ParagonSteel` | `/Game/Invaders/ParagonSteel` · guess_path |

## Assassin seats (Paragon)

`Invader.Assassin.*` — Countess, Kallari, Revenant, Yin (+ Serath, Shinbi catalog). Thief remains **Common** Fab only.

---

_Generated with `tmp/generate-character-sheets.py` — 75 sheets._
