"""Generate Data/character-sheets.json for every UE-listed defender tier and invader role."""

from __future__ import annotations

import json
from copy import deepcopy
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "Data" / "character-sheets.json"

ARCHETYPE_KITS: dict[str, dict[str, Any]] = {
    "Wanderer": {
        "primary": "Opportunist strike",
        "secondary": "Short sprint",
        "effectTagsPrimary": ["Melee", "Dash"],
        "effectTagsSecondary": ["Dash"],
    },
    "Thief": {
        "primary": "Lockpick",
        "secondary": "Climb walls",
        "effectTagsPrimary": ["PickLock"],
        "effectTagsSecondary": ["ClimbWall"],
    },
    "Assassin": {
        "primary": "Ambush / execute",
        "secondary": "Blink or vanish step",
        "effectTagsPrimary": ["Melee", "Execute"],
        "effectTagsSecondary": ["Blink"],
    },
    "Hero": {
        "primary": "Power cleave",
        "secondary": "Battle shout",
        "effectTagsPrimary": ["Melee", "AOE"],
        "effectTagsSecondary": ["BuffAura"],
    },
    "Champion": {
        "primary": "Charged slam",
        "secondary": "Zone control (slow/root)",
        "effectTagsPrimary": ["Melee", "AOE"],
        "effectTagsSecondary": ["Slow"],
    },
    "Tank": {
        "primary": "Guard / damage soak taunt",
        "secondary": "Shield slam",
        "effectTagsPrimary": ["Melee", "AOE"],
        "effectTagsSecondary": ["Melee", "AOE"],
    },
    "Bruiser": {
        "primary": "Heavy cleave",
        "secondary": "Bull rush",
        "effectTagsPrimary": ["Melee", "Cleave"],
        "effectTagsSecondary": ["Melee", "Dash"],
    },
    "Fighter": {
        "primary": "Combo strike",
        "secondary": "Brace",
        "effectTagsPrimary": ["Melee"],
        "effectTagsSecondary": ["Melee"],
    },
    "Mage": {
        "primary": "Ranged burst",
        "secondary": "Blink or reposition",
        "effectTagsPrimary": ["Ranged", "Ability"],
        "effectTagsSecondary": ["Blink"],
    },
    "Archer": {
        "primary": "Piercing shot",
        "secondary": "Volley or marked target",
        "effectTagsPrimary": ["Ranged", "Line"],
        "effectTagsSecondary": ["Ranged", "AOE"],
    },
    "Cleric": {
        "primary": "Heal pulse",
        "secondary": "Ward or cleanse",
        "effectTagsPrimary": ["Heal"],
        "effectTagsSecondary": ["Shield"],
    },
    "Scout": {
        "primary": "Trap probe / reveal edge",
        "secondary": "Sprint",
        "effectTagsPrimary": ["Ranged"],
        "effectTagsSecondary": ["Dash"],
    },
}

TIER_ARCHETYPE = {
    "Worker": "Fighter",
    "Minion": "Fighter",
    "Elite": "Bruiser",
    "Boss": "Tank",
}

TIER_MULT = {
    "Worker": {"hp": 0.25, "dmg": 0.15, "ref": "Fodder", "abilities": 0},
    "Minion": {"hp": 0.45, "dmg": 0.4, "ref": "Fodder", "abilities": 1},
    "Elite": {"hp": 1.0, "dmg": 1.0, "ref": "Standard", "abilities": 2},
    "Boss": {"hp": 3.0, "dmg": 1.4, "ref": "Veteran", "abilities": 2},
    "Hero": {"hp": 1.15, "dmg": 1.1, "ref": "Veteran", "abilities": 2},
}

BASE_VITALS = {
    "MaxHealth": 72,
    "HealthRegenPerSec": 0,
    "MoveSpeed": 280,
    "BasicDamage": 11,
    "AttackPeriod": 0.95,
    "CleavePct": 0.15,
    "BasicAffinity": "Physical",
    "BasicDefense": 18,
    "AbilityDefense": 14,
}

BASE_RATINGS = {"BasicDamage": 5, "AbilityDamage": 2, "Durability": 5, "Mobility": 4}

DEFENDER_FAMILIES: dict[str, dict[str, str]] = {
    "Orcs": {
        "Worker": "/Game/Monsters/Orcs/StylizedGoblinMinion/SK_StylizedGoblinMinion",
        "Minion": "/Game/Monsters/Orcs/Minion/Orc_Minion_Unreal",
        "Elite": "/Game/Monsters/Orcs/elite_orc",
        "Boss": "/Game/Monsters/Orcs/OrcBoss/Orc_Boss_unreal",
        "Hero": "/Game/Monsters/Orcs/ParagonNarbash/Characters/Heroes/Narbash/Meshes/Narbash",
    },
    "EvilFairies": {
        "Worker": "/Game/Monsters/EvilFairies/dark_fairy_worker_1",
        "Minion": "/Game/Monsters/EvilFairies/dark_fairy_Minion_v1",
        "Elite": "/Game/Monsters/EvilFairies/dark_fairy_elite_v1",
        "Boss": "/Game/Monsters/EvilFairies/dark_fairy_boss_v1",
        "Hero": "/Game/Monsters/EvilFairies/ParagonFey/Characters/Heroes/Fey/Skins/Opaline/Meshes/Fey_Opaline",
    },
    "Fungal": {
        "Worker": "/Game/Monsters/Fungal/Fungal_Worker_v2",
        "Minion": "/Game/Monsters/Fungal/Fungal_Minion",
        "Elite": "/Game/Monsters/Fungal/SK_FungiInfestedKnight",
        "Boss": "/Game/Monsters/Fungal/SK_FungalMyconidGolem",
        "Hero": "/Game/Monsters/Fungal/FungalQueen/Fungal_Queen",
    },
    "Spiders": {
        "Worker": "/Game/Monsters/Spiders/Argiope_v3",
        "Minion": "/Game/Monsters/Spiders/Blue_Tarantula_v3",
        "Elite": "/Game/Monsters/Spiders/Karakurt",
        "Boss": "/Game/Monsters/Spiders/Tarantula",
        "Hero": "/Game/Monsters/Spiders/Peacock",
    },
    "Undead": {
        "Worker": "/Game/Monsters/Undead/Zombie",
        "Minion": "/Game/Monsters/Undead/skeleton_v1",
        "Elite": "/Game/Monsters/Undead/mummy_v1",
        "Boss": "/Game/Monsters/Undead/litch_v1",
        "Hero": "/Game/Monsters/Undead/ParagonSevarog/Characters/Heroes/Sevarog/Meshes/Sevarog",
    },
    "Mutants": {
        "Worker": "/Game/Monsters/Mutants/mutant_worker_v2",
        "Minion": "/Game/Monsters/Mutants/Minion/Mutant_Minion_Unreal",
        "Elite": "/Game/Monsters/Mutants/mutant_worker_v2",
        "Boss": "/Game/Monsters/Mutants/Boss/Mutant_Boss",
        "Hero": "/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage",
    },
}

HERO_META: dict[str, dict[str, Any]] = {
    "Orcs": {
        "DisplayName": "Narbash",
        "ArchetypeId": "Cleric",
        "ParagonRef": "Narbash",
        "StatSource": "ParagonWiki",
        "Primary": "March!",
        "Secondary": "Thump",
        "ParagonKeys": ("March!", "Thump"),
        "UsesParagonAnimSet": True,
    },
    "EvilFairies": {
        "DisplayName": "The Fey",
        "ArchetypeId": "Mage",
        "ParagonRef": "The Fey",
        "StatSource": "ParagonWiki",
        "Primary": "Harvest Nettles",
        "Secondary": "Bramble Patch",
        "ParagonKeys": ("Harvest Nettles", "Bramble Patch"),
        "UsesParagonAnimSet": True,
    },
    "Fungal": {
        "DisplayName": "Fungal Queen",
        "ArchetypeId": "Mage",
        "StatSource": "HandTuned",
        "Primary": "Spores",
        "Secondary": "Roots",
        "EffectTagsPrimary": ["Ranged", "AOE"],
        "EffectTagsSecondary": ["Root", "Slow"],
        "VitalsOverride": {
            "MaxHealth": 58,
            "BasicDamage": 6,
            "BasicDefense": 12,
            "AbilityDefense": 16,
            "MoveSpeed": 260,
            "CleavePct": 0.05,
        },
        "RatingsOverride": {
            "BasicDamage": 3,
            "AbilityDamage": 8,
            "Durability": 3,
            "Mobility": 4,
            "Intelligence": 9,
            "Wisdom": 8,
        },
        "AbilityTuning": [
            {"BaseDamage": 22, "CooldownSec": 8, "RangeUU": 600},
            {"BaseDamage": 18, "CooldownSec": 14, "RangeUU": 450},
        ],
        "DefenderAIOverride": {
            "AggroStyle": "Stationary",
            "LeashRadiusUU": 300,
            "PreferredRange": "Long",
        },
    },
    "Spiders": {
        "DisplayName": "Peacock",
        "ArchetypeId": "Archer",
        "StatSource": "HandTuned",
        "Primary": "Ranged web",
        "Secondary": "Leap attack",
        "EffectTagsPrimary": ["Ranged", "Root", "Slow"],
        "EffectTagsSecondary": ["Melee", "Dash"],
        "VitalsOverride": {
            "MaxHealth": 88,
            "BasicDamage": 10,
            "BasicDefense": 18,
            "AbilityDefense": 14,
            "MoveSpeed": 285,
            "CleavePct": 0.1,
        },
        "RatingsOverride": {
            "BasicDamage": 6,
            "AbilityDamage": 5,
            "Durability": 7,
            "Mobility": 6,
        },
        "AbilityTuning": [
            {"BaseDamage": 16, "CooldownSec": 9, "RangeUU": 650},
            {"BaseDamage": 24, "CooldownSec": 12, "RangeUU": 400},
        ],
        "DefenderAIOverride": {
            "AggroStyle": "Stationary",
            "LeashRadiusUU": 300,
            "PreferredRange": "Long",
        },
    },
    "Undead": {
        "DisplayName": "Sevarog",
        "ArchetypeId": "Tank",
        "ParagonRef": "Sevarog",
        "StatSource": "ParagonWiki",
        "Primary": "Siphon",
        "Secondary": "Subjugate",
        "ParagonKeys": ("Siphon", "Subjugate"),
        "UsesParagonAnimSet": True,
        "EffectTagsPrimary": ["AOE"],
        "EffectTagsSecondary": ["AOE", "Root", "Slow"],
    },
    "Mutants": {
        "DisplayName": "Rampage",
        "ArchetypeId": "Tank",
        "ParagonRef": "Rampage",
        "StatSource": "ParagonWiki",
        "Primary": "Boulder Throw",
        "Secondary": "Rumble",
        "ParagonKeys": ("Boulder Throw", "Rumble"),
        "UsesParagonAnimSet": True,
    },
}

FAMILY_MINION_ARCHETYPE = {
    "EvilFairies": "Mage",
    "Spiders": "Archer",
}

INVADER_BEHAVIOR: dict[str, dict[str, float]] = {
    "Tank": {"Caution": 0.3, "CohesionBias": 0.3, "FollowerWeight": 0.55, "HelperWeight": 0.7, "ScoutRating": 0.2, "LootGreed": 0.35},
    "Bruiser": {"Caution": 0.3, "CohesionBias": 0.3, "FollowerWeight": 0.55, "HelperWeight": 0.7, "ScoutRating": 0.2, "LootGreed": 0.35},
    "Cleric": {"Caution": 0.4, "CohesionBias": 0.5, "FollowerWeight": 0.65, "HelperWeight": 0.85, "ScoutRating": 0.3, "LootGreed": 0.25},
    "Fighter": {"Caution": 0.35, "CohesionBias": 0.2, "FollowerWeight": 0.45, "HelperWeight": 0.6, "ScoutRating": 0.25, "LootGreed": 0.5},
    "Hero": {"Caution": 0.35, "CohesionBias": 0.2, "FollowerWeight": 0.45, "HelperWeight": 0.6, "ScoutRating": 0.25, "LootGreed": 0.5},
    "Champion": {"Caution": 0.35, "CohesionBias": 0.2, "FollowerWeight": 0.45, "HelperWeight": 0.6, "ScoutRating": 0.25, "LootGreed": 0.5},
    "Thief": {"Caution": 0.75, "CohesionBias": -0.25, "FollowerWeight": 0.1, "HelperWeight": 0.3, "ScoutRating": 0.7, "LootGreed": 0.55},
    "Scout": {"Caution": 0.7, "CohesionBias": -0.15, "FollowerWeight": 0.15, "HelperWeight": 0.35, "ScoutRating": 0.9, "LootGreed": 0.3},
    "Archer": {"Caution": 0.6, "CohesionBias": -0.1, "FollowerWeight": 0.25, "HelperWeight": 0.4, "ScoutRating": 0.4, "LootGreed": 0.3},
    "Mage": {"Caution": 0.6, "CohesionBias": -0.1, "FollowerWeight": 0.25, "HelperWeight": 0.4, "ScoutRating": 0.4, "LootGreed": 0.3},
    "Wanderer": {"Caution": 0.2, "CohesionBias": 0.0, "FollowerWeight": 0.35, "HelperWeight": 0.45, "ScoutRating": 0.15, "LootGreed": 0.65},
}

EXPEDITION_BY_ARCHETYPE: dict[str, dict[str, Any]] = {
    "Thief": {"HasRope": False, "RopeLengthUU": 0, "HasLockpick": True, "CanClimbWalls": True, "HasSustainedFlight": False},
    "Scout": {"HasRope": True, "RopeLengthUU": 300, "HasLockpick": False, "CanClimbWalls": False, "HasSustainedFlight": False},
    "Assassin": {"HasRope": False, "RopeLengthUU": 0, "HasLockpick": False, "CanClimbWalls": True, "HasSustainedFlight": False},
}

DEFAULT_EXPEDITION = {
    "HasRope": False,
    "RopeLengthUU": 0,
    "HasLockpick": False,
    "CanClimbWalls": False,
    "HasSustainedFlight": False,
}

PARAGON_INVADER_ABILITIES: dict[str, tuple[str, str, str, str]] = {
    "Terra": ("Ruthless Assault", "Deafening Command", "AOE", "Slow"),
    "Greystone": ("Make Way", "Assault the Gates", "AOE", "Dash"),
    "Rampage": ("Boulder Throw", "Rumble", "Melee", "AOE"),
    "Kwang": ("Judgement of the Heavens", "Light of the Heavens", "Melee", "AOE"),
    "Phase": ("Psychic Flare", "Energy Lance", "Ranged", "Root"),
    "Aurora": ("Glacial Charge", "Hoarfrost", "Blink", "Root"),
    "Sparrow": ("Hail of Arrows", "Piercing Shot", "AOE", "Line"),
    "Zinx": ("Bad Medicine", "Ricochet Stun", "Slow", "Stun"),
    "Countess": ("Dark Tide", "Shadow Slip", "Melee", "Blink"),
    "Crunch": ("Left Crunch", "Right Crunch", "Melee", "Melee"),
    "Dekker": ("Stasis Bomb", "Containment Fence", "Ranged", "Root"),
    "Drongo": ("Rad Rounds", "Guttler", "Ranged", "Melee"),
    "Feng Mao": ("Hamstring", "Precision Strike", "Melee", "Melee"),
    "Gadget": ("Sticky Mine", "Seek & Destroy", "Ranged", "Ranged"),
    "Gideon": ("Cosmic Rift", "Torn Space", "Ranged", "Blink"),
    "GRIM.exe": ("Deflect", "GTFO", "Shield", "Blink"),
    "Howitzer": ("R2000 Missile", "Slow Grenades", "Ranged", "Slow"),
    "IggyScorch": ("Oil Spill", "Flame Turret", "Ranged", "AOE"),
    "Kallari": ("Slash", "Backdash Blade", "Melee", "Blink"),
    "Khaimera": ("Ambush", "Unleash", "Melee", "Melee"),
    "LtBelica": ("Seismic Assault", "Void Bomb", "Ranged", "AOE"),
    "Morigesh": ("Mark", "Curse", "Ranged", "Slow"),
    "Murdock": ("Static Shield", "Buckshot", "Shield", "Ranged"),
    "Muriel": ("Alacrity", "Serenity", "Heal", "Shield"),
    "Revenant": ("Scar", "Hellfire Rounds", "Ranged", "Melee"),
    "Riktor": ("Electrocute", "Blade Rush", "Melee", "Melee"),
    "Serath": ("Blazing Seraph", "Fatal Grace", "Melee", "Execute"),
    "Shinbi": ("All-Kill", "Circle Net", "Melee", "Root"),
    "Steel": ("Shield Bash", "Bulwark", "Melee", "Shield"),
    "TwinBlast": ("Vaporize", "Rocket Dash", "Ranged", "Dash"),
    "Wukong": ("Tongue Lash", "Four-Fisted Fury", "Melee", "AOE"),
    "Wraith": ("Zap", "Kinetic Pulse", "Ranged", "Ranged"),
    "Yin": ("Lash Kick", "Backlash", "Melee", "Melee"),
}

WORKER_ROLE_BY_FAMILY: dict[str, str] = {
    "Orcs": "Dig",
    "Undead": "Dig",
    "EvilFairies": "Build",
    "Fungal": "Dig",
    "Spiders": "None",
    "Mutants": "Both",
}

# Paragon invaders: (display, archetype, paragon_ref key in abilities dict)
PARAGON_INVADER_CATALOG: list[tuple[str, str, str]] = [
    ("Countess", "Assassin", "Countess"),
    ("Crunch", "Bruiser", "Crunch"),
    ("Dekker", "Mage", "Dekker"),
    ("Drongo", "Archer", "Drongo"),
    ("Feng Mao", "Bruiser", "Feng Mao"),
    ("Gadget", "Mage", "Gadget"),
    ("Gideon", "Mage", "Gideon"),
    ("GRIM.exe", "Mage", "GRIM.exe"),
    ("Howitzer", "Mage", "Howitzer"),
    ("Iggy & Scorch", "Mage", "IggyScorch"),
    ("Kallari", "Assassin", "Kallari"),
    ("Khaimera", "Bruiser", "Khaimera"),
    ("Lt. Belica", "Mage", "LtBelica"),
    ("Morigesh", "Mage", "Morigesh"),
    ("Murdock", "Archer", "Murdock"),
    ("Muriel", "Cleric", "Muriel"),
    ("Revenant", "Assassin", "Revenant"),
    ("Riktor", "Tank", "Riktor"),
    ("Serath", "Assassin", "Serath"),
    ("Shinbi", "Assassin", "Shinbi"),
    ("Steel", "Tank", "Steel"),
    ("TwinBlast", "Archer", "TwinBlast"),
    ("Wukong", "Bruiser", "Wukong"),
    ("Wraith", "Mage", "Wraith"),
    ("Yin", "Assassin", "Yin"),
]

PARAGON_INVADER_ASSIGNED = {
    "Terra",
    "Greystone",
    "Rampage",
    "Kwang",
    "Phase",
    "Aurora",
    "Sparrow",
    "Zinx",
}

INVADER_BEHAVIOR["Assassin"] = {
    "Caution": 0.45,
    "CohesionBias": -0.1,
    "FollowerWeight": 0.2,
    "HelperWeight": 0.35,
    "ScoutRating": 0.4,
    "LootGreed": 0.35,
}


def make_abilities_from_kit(
    sheet_prefix: str,
    archetype: str,
    count: int,
    *,
    paragon_primary: str | None = None,
    paragon_secondary: str | None = None,
    dmg_scale: float = 1.0,
    tags_p: list[str] | None = None,
    tags_s: list[str] | None = None,
) -> list[dict[str, Any]]:
    if count == 0:
        return []
    kit = ARCHETYPE_KITS[archetype]
    primary_name = paragon_primary or kit["primary"]
    secondary_name = paragon_secondary or kit["secondary"]
    tags_primary = tags_p or list(kit["effectTagsPrimary"])
    tags_secondary = tags_s or list(kit["effectTagsSecondary"])
    abilities: list[dict[str, Any]] = []
    if count >= 1:
        abilities.append(
            {
                "AbilityId": f"{sheet_prefix}.Primary",
                "Slot": "Primary",
                "DisplayName": primary_name,
                "DamageType": "None" if archetype == "Thief" and not paragon_primary else "Ability",
                "BaseDamage": round(18 * dmg_scale) if archetype != "Thief" else 0,
                "CooldownSec": 8,
                "RangeUU": 150 if archetype == "Thief" else 0,
                "EffectTags": tags_primary,
                **({"ParagonAbilityKey": paragon_primary} if paragon_primary else {}),
            }
        )
    if count >= 2:
        abilities.append(
            {
                "AbilityId": f"{sheet_prefix}.Secondary",
                "Slot": "Secondary",
                "DisplayName": secondary_name,
                "DamageType": "None" if archetype == "Thief" and not paragon_secondary else "Ability",
                "BaseDamage": round(14 * dmg_scale) if archetype != "Thief" else 0,
                "CooldownSec": 12,
                "RangeUU": 0,
                "EffectTags": tags_secondary,
                **({"ParagonAbilityKey": paragon_secondary} if paragon_secondary else {}),
            }
        )
    return abilities


def scale_vitals(mult_hp: float, mult_dmg: float, worker: bool = False) -> dict[str, Any]:
    v = deepcopy(BASE_VITALS)
    v["MaxHealth"] = round(v["MaxHealth"] * mult_hp, 2)
    v["BasicDamage"] = 0 if worker else round(v["BasicDamage"] * mult_dmg, 2)
    return v


def defender_sheet(family: str, tier: str) -> dict[str, Any]:
    sheet_id = f"{family}.{tier}"
    mesh = DEFENDER_FAMILIES[family][tier]
    mult = TIER_MULT[tier]
    if tier == "Hero":
        hero = HERO_META[family]
        archetype = hero["ArchetypeId"]
        display = hero["DisplayName"]
        stat_source = hero["StatSource"]
        paragon_ref = hero.get("ParagonRef")
        vitals = scale_vitals(mult["hp"], mult["dmg"])
        if family == "Undead":
            vitals.update({"MaxHealth": 95, "BasicDefense": 20, "AbilityDefense": 30})
        abilities = make_abilities_from_kit(
            sheet_id.replace(".", "_"),
            archetype,
            2,
            paragon_primary=hero.get("Primary"),
            paragon_secondary=hero.get("Secondary"),
            tags_p=hero.get("EffectTagsPrimary"),
            tags_s=hero.get("EffectTagsSecondary"),
            dmg_scale=mult["dmg"],
        )
        for i, key in enumerate(hero.get("ParagonKeys", ())):
            if i < len(abilities):
                abilities[i]["ParagonAbilityKey"] = key
        for i, tune in enumerate(hero.get("AbilityTuning", ())):
            if i < len(abilities):
                abilities[i].update(tune)
        if hero.get("VitalsOverride"):
            vitals.update(hero["VitalsOverride"])
        ratings = hero.get(
            "RatingsOverride",
            {"BasicDamage": 5, "AbilityDamage": 4, "Durability": 8, "Mobility": 5},
        )
    else:
        archetype = TIER_ARCHETYPE[tier]
        if tier == "Minion" and family in FAMILY_MINION_ARCHETYPE:
            archetype = FAMILY_MINION_ARCHETYPE[family]
        display = f"{family} {tier}"
        stat_source = "TierBaseline"
        paragon_ref = None
        worker = tier == "Worker"
        vitals = scale_vitals(mult["hp"], mult["dmg"], worker=worker)
        abilities = make_abilities_from_kit(
            sheet_id.replace(".", "_"),
            archetype,
            mult["abilities"],
            dmg_scale=mult["dmg"],
        )
        ratings = deepcopy(BASE_RATINGS)
        if tier == "Boss":
            ratings["Durability"] = 8
            ratings["BasicDamage"] = 7

    row: dict[str, Any] = {
        "SheetId": sheet_id,
        "EntityKind": "Defender",
        "ArchetypeId": archetype,
        "FamilyId": family,
        "Tier": tier,
        "DisplayName": display,
        "MeshRef": mesh,
        "StatSource": stat_source,
        "ReferenceTier": mult["ref"],
        "Vitals": vitals,
        "Ratings": ratings,
        "Abilities": abilities,
        "DefenderAI": {
            "WorkerRole": WORKER_ROLE_BY_FAMILY.get(family, "None") if tier == "Worker" else "None",
            "AggroStyle": "Chase",
            "LeashRadiusUU": 1200 if tier in ("Boss", "Hero") else 900,
            "PreferredRange": "Melee" if archetype not in ("Archer", "Mage") else "Long",
            "UsesParagonAnimSet": bool(HERO_META.get(family, {}).get("UsesParagonAnimSet")) if tier == "Hero" else False,
        },
        "StartingKnowledgeTags": [],
    }
    if tier == "Hero" and family in HERO_META:
        ai_override = HERO_META[family].get("DefenderAIOverride")
        if ai_override:
            row["DefenderAI"].update(ai_override)
    if paragon_ref:
        row["ParagonRef"] = paragon_ref
    return row


def invader_sheet(
    sheet_id: str,
    archetype: str,
    display: str,
    mesh: str,
    *,
    line: str = "Common",
    paragon_ref: str | None = None,
    ref_tier: str = "Standard",
    knowledge: list[str] | None = None,
) -> dict[str, Any]:
    mult = 0.4 if line == "Common" else 1.0
    ref = "Fodder" if line == "Common" else ref_tier
    vitals = scale_vitals(1.0 * mult, 1.0 * mult)
    vitals["MoveSpeed"] = 300 if archetype in ("Thief", "Scout", "Assassin") else 285
    abilities: list[dict[str, Any]]
    if paragon_ref and paragon_ref in PARAGON_INVADER_ABILITIES:
        p, s, t1, t2 = PARAGON_INVADER_ABILITIES[paragon_ref]
        abilities = make_abilities_from_kit(
            sheet_id.replace(".", "_"),
            archetype,
            2,
            paragon_primary=p,
            paragon_secondary=s,
            tags_p=[t1],
            tags_s=[t2, "Ability"] if t2 != "Dash" else [t2],
            dmg_scale=mult if line == "Common" else 1.0,
        )
        abilities[0]["ParagonAbilityKey"] = p
        abilities[1]["ParagonAbilityKey"] = s
    else:
        abilities = make_abilities_from_kit(
            sheet_id.replace(".", "_"),
            archetype,
            2,
            dmg_scale=mult,
        )
    if paragon_ref == "Greystone" and line == "Paragon":
        vitals.update({"MaxHealth": 85, "BasicDefense": 15, "AbilityDefense": 30})
        abilities = make_abilities_from_kit(
            "Invader_Tank_Greystone",
            "Tank",
            2,
            paragon_primary="Make Way",
            paragon_secondary="Assault the Gates",
            tags_p=["AOE", "Melee"],
            tags_s=["Dash", "Melee", "AOE"],
        )
        abilities[0].update({"BaseDamage": 50, "CooldownSec": 13, "ParagonAbilityKey": "Make Way"})
        abilities[1].update({"BaseDamage": 36, "CooldownSec": 10, "RangeUU": 450, "ParagonAbilityKey": "Assault the Gates"})

    behavior = INVADER_BEHAVIOR.get(archetype, INVADER_BEHAVIOR["Fighter"])
    expedition = EXPEDITION_BY_ARCHETYPE.get(archetype, DEFAULT_EXPEDITION)

    row: dict[str, Any] = {
        "SheetId": sheet_id,
        "EntityKind": "Invader",
        "ArchetypeId": archetype,
        "DisplayName": display,
        "MeshRef": mesh,
        "InvaderLine": line,
        "StatSource": "ParagonWiki" if paragon_ref else "HandTuned",
        "ReferenceTier": ref if line == "Common" else ref_tier,
        "Vitals": vitals,
        "Ratings": deepcopy(BASE_RATINGS),
        "Abilities": abilities,
        "ExpeditionGear": expedition,
        "InvaderBehavior": behavior,
        "StartingKnowledgeTags": knowledge or [],
    }
    if paragon_ref:
        row["ParagonRef"] = paragon_ref
    return row


def _firefly_cleric_sheet() -> dict[str, Any]:
    row = invader_sheet(
        "Invader.Cleric.FireflySlot",
        "Cleric",
        "Firefly Cleric",
        "",
        line="Common",
        ref_tier="Standard",
    )
    row["MeshPipeline"] = "Hi3D"
    row["MeshTargetPath"] = "/Game/Invaders/Cleric/"
    row["StatSource"] = "HandTuned"
    row["PartyTemplates"] = ["FireflyFour"]
    row["Vitals"].update(
        {
            "MaxHealth": 76,
            "HealthRegenPerSec": 0.5,
            "MaxStamina": 120,
            "StaminaRegenPerSec": 2.5,
            "BasicDamage": 3,
            "BasicDefense": 16,
            "AbilityDefense": 18,
            "MoveSpeed": 275,
            "CleavePct": 0.05,
        }
    )
    row["Ratings"].update(
        {
            "BasicDamage": 3,
            "AbilityDamage": 5,
            "Durability": 4,
            "Mobility": 3,
            "Agility": 3,
            "Intelligence": 3,
            "Wisdom": 9,
        }
    )
    row["Abilities"][0].update(
        {"DamageType": "Ability", "BaseDamage": 20, "EffectTags": ["Heal"]}
    )
    row["Abilities"][1].update(
        {"DamageType": "Ability", "BaseDamage": 16, "EffectTags": ["Shield", "Heal"]}
    )
    row["InvaderBehavior"].update(
        {
            "CohesionBias": 0.55,
            "HelperWeight": 0.9,
            "FollowerWeight": 0.7,
            "Caution": 0.45,
            "LootGreed": 0.2,
        }
    )
    row["StartingKnowledgeTags"] = ["Knows_LeyTraps", "Knows_AllyHeal"]
    return row


def _warrior_queen_sheet() -> dict[str, Any]:
    row = invader_sheet(
        "Invader.Champion.WarriorQueen",
        "Champion",
        "Warrior Queen",
        "/Game/Invaders/WarriorQueen/Warrior_Queen",
        line="Common",
        ref_tier="Standard",
    )
    row["MeshPipeline"] = "InUE"
    row["MeshTargetPath"] = "/Game/Invaders/WarriorQueen/"
    row["StatSource"] = "HandTuned"
    row["Vitals"].update(
        {
            "MaxHealth": 62,
            "BasicDamage": 9,
            "BasicDefense": 17,
            "AbilityDefense": 15,
            "MoveSpeed": 288,
            "CleavePct": 0.12,
        }
    )
    row["Ratings"].update(
        {
            "BasicDamage": 7,
            "AbilityDamage": 6,
            "Durability": 7,
            "Mobility": 6,
            "Agility": 6,
            "Intelligence": 7,
            "Wisdom": 7,
        }
    )
    row["Abilities"][0].update({"BaseDamage": 18, "CooldownSec": 8})
    row["Abilities"][1].update({"BaseDamage": 14, "CooldownSec": 12, "EffectTags": ["Slow", "Root"]})
    row["InvaderBehavior"].update(
        {
            "CohesionBias": 0.45,
            "FollowerWeight": 0.6,
            "HelperWeight": 0.65,
            "Caution": 0.35,
            "LootGreed": 0.45,
        }
    )
    row["StartingKnowledgeTags"] = []
    return row


def _thief_common_sheet() -> dict[str, Any]:
    row = invader_sheet(
        "Invader.Thief.Common",
        "Thief",
        "Thief",
        "",
        line="Common",
        ref_tier="Fodder",
        knowledge=["Knows_MimicRisk", "Knows_LeyTraps", "Knows_MirageVeil"],
    )
    row["MeshPipeline"] = "Hi3D"
    row["MeshTargetPath"] = "/Game/Invaders/Thief/"
    row["StatSource"] = "HandTuned"
    row["Vitals"].update(
        {
            "MaxHealth": 18,
            "BasicDamage": 3,
            "MoveSpeed": 310,
            "CleavePct": 0.08,
            "BasicDefense": 13,
            "AbilityDefense": 10,
        }
    )
    row["Ratings"].update(
        {
            "BasicDamage": 2,
            "Durability": 2,
            "Mobility": 8,
            "Agility": 9,
            "Intelligence": 4,
            "Wisdom": 4,
        }
    )
    row["Abilities"] = [
        {
            "AbilityId": "Invader_Thief_Common.Primary",
            "Slot": "Primary",
            "DisplayName": "Lockpick",
            "DamageType": "None",
            "BaseDamage": 0,
            "CooldownSec": 8,
            "RangeUU": 150,
            "EffectTags": ["PickLock"],
        },
        {
            "AbilityId": "Invader_Thief_Common.Secondary",
            "Slot": "Secondary",
            "DisplayName": "Climb walls",
            "DamageType": "None",
            "BaseDamage": 0,
            "CooldownSec": 10,
            "RangeUU": 300,
            "EffectTags": ["ClimbWall"],
        },
    ]
    row["ExpeditionGear"] = {
        "HasRope": False,
        "RopeLengthUU": 0,
        "HasLockpick": True,
        "CanClimbWalls": True,
        "HasSustainedFlight": False,
    }
    row["StartingKnowledgeTags"] = [
        "Knows_MimicRisk",
        "Knows_LeyTraps",
        "Knows_MirageVeil",
    ]
    return row


def _wanderer_common_sheet() -> dict[str, Any]:
    row = invader_sheet("Invader.Wanderer.Common", "Wanderer", "Wanderer", "", line="Common", ref_tier="Fodder")
    row["MeshPipeline"] = "Hi3D"
    row["MeshTargetPath"] = "/Game/Invaders/Wanderer/"
    row["StatSource"] = "HandTuned"
    row["Vitals"].update(
        {
            "MaxHealth": 22,
            "BasicDamage": 3.5,
            "CleavePct": 0.1,
            "BasicDefense": 15,
            "AbilityDefense": 12,
        }
    )
    row["Ratings"].update(
        {
            "BasicDamage": 3,
            "Durability": 3,
            "Intelligence": 7,
            "Wisdom": 7,
        }
    )
    row["Abilities"][0]["BaseDamage"] = 5
    row["Abilities"][1]["BaseDamage"] = 4
    row["InvaderBehavior"].update({"Caution": 0.45, "ScoutRating": 0.4})
    row["StartingKnowledgeTags"] = ["Knows_LeyTraps", "Knows_MimicRisk"]
    return row


def paragon_mesh_guess(paragon_ref: str) -> str:
    if paragon_ref == "Rampage":
        return "/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage"
    slug = paragon_ref.replace(" ", "").replace(".", "")
    return f"/Game/Invaders/Paragon{slug}"


def sheet_id_for_paragon(archetype: str, display: str) -> str:
    token = (
        display.replace(" ", "")
        .replace("&", "")
        .replace(".", "")
        .replace("'", "")
    )
    return f"Invader.{archetype}.{token}"


def write_character_objects_todo(sheets: list[dict[str, Any]]) -> None:
    todo_path = ROOT / "todo" / "character-objects-create.md"
    lines: list[str] = [
        "# Character objects to create",
        "",
        "**Blocked:** Hi3D credits exhausted — defer **Hi3D generate** rows until credits reset (~couple weeks).",
        "**Sheets:** [character-sheets.json](../Data/character-sheets.json) · rig notes: [Monsters-11-23-26.md](./Monsters-11-23-26.md)",
        "",
        "Legend: `in_ue` mesh path exists in project · `missing` empty `MeshRef` · `rig` in UE but needs AccuRIG/idle fix · `vault` Epic Paragon — migrate via launcher only",
        "",
        "## Priority — Common line (no mesh yet)",
        "",
        "| SheetId | Display | Pipeline | Target `/Game/` path | Status |",
        "| --- | --- | --- | --- | --- |",
        "| `Invader.Wanderer.Common` | Wanderer | Fab / Hi3D | `Invaders/Wanderer/` | missing · blocked_hi3d |",
        "| `Invader.Thief.Common` | Thief | Fab Medieval Assassin Girl | `Invaders/Thief/` | missing · Fab |",
        "| `Invader.Champion.WarriorQueen` | Warrior Queen | Hi3D / CC (rigged) | `Invaders/WarriorQueen/` | missing · blocked_hi3d |",
        "| `Invader.Cleric.FireflySlot` | Firefly Cleric (optional) | Hi3D / Firefly | `Invaders/Cleric/` | missing · use **Zinx** until ready |",
        "",
        "## Firefly party (sheets exist — object work)",
        "",
        "| SheetId | MeshRef | Status |",
        "| --- | --- | --- |",
    ]
    firefly_ids = (
        "Invader.DwarfFighter.Common",
        "Invader.ElfArcher.Common",
        "Invader.Thelma.Common",
        "Invader.Wizard.Common",
        "Invader.ElfMage.Common",
    )
    by_id = {s["SheetId"]: s for s in sheets}
    for fid in firefly_ids:
        s = by_id[fid]
        mesh = s.get("MeshRef") or "—"
        st = "in_ue" if mesh and mesh != "—" else "missing"
        lines.append(f"| `{fid}` | `{mesh}` | {st} · party template `FireflyFour` |")
    lines.extend(
        [
            "",
            "## Defenders — all tiers (rig / scale)",
            "",
            "See [Monsters-11-23-26.md](./Monsters-11-23-26.md) per family. Workers need `WorkerRole` on sheet (`Dig` / `Build` / `Both`).",
            "",
            "## Paragon invaders — vault → UE",
            "",
            "| SheetId | Display | Suggested migrate path | MeshRef on sheet |",
            "| --- | --- | --- | --- |",
        ]
    )
    for s in sorted(
        (x for x in sheets if x.get("EntityKind") == "Invader" and x.get("InvaderLine") == "Paragon"),
        key=lambda x: x["SheetId"],
    ):
        mesh = s.get("MeshRef") or ""
        status = "vault" if not mesh else "guess_path"
        ref = s.get("ParagonRef") or s.get("DisplayName", "")
        lines.append(
            f"| `{s['SheetId']}` | {s.get('DisplayName', '')} | `/Game/Invaders/Paragon{ref.replace(' ', '')}` | `{mesh or '—'}` · {status} |"
        )
    lines.extend(
        [
            "",
            "## Assassin seats (Paragon)",
            "",
            "`Invader.Assassin.*` — Countess, Kallari, Revenant, Yin (+ Serath, Shinbi catalog). Thief remains **Common** Fab only.",
            "",
            "---",
            "",
            f"_Generated with `tmp/generate-character-sheets.py` — {len(sheets)} sheets._",
        ]
    )
    todo_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote todo to {todo_path}")


WORKER_DISPLAY_NAMES: dict[str, str] = {
    "Orcs": "Orc Worker",
    "Undead": "Undead Worker",
    "EvilFairies": "Evil Fairy Worker",
    "Fungal": "Fungal Worker",
    "Spiders": "Spider Worker",
    "Mutants": "Mutant Worker",
}

DEFENDER_SHEET_OVERRIDES: dict[str, dict[str, Any]] = {
    "Orcs.Minion": {
        "DisplayName": "Orc Minion",
        "ReferenceTier": "Standard",
        "StatSource": "HandTuned",
        "Vitals": {
            "MaxHealth": 48,
            "BasicDamage": 7,
            "BasicDefense": 16,
            "AbilityDefense": 13,
            "CleavePct": 0.12,
        },
        "Ratings": {
            "BasicDamage": 5,
            "AbilityDamage": 4,
            "Durability": 5,
            "Mobility": 5,
            "Intelligence": 2,
            "Wisdom": 2,
        },
        "Abilities": [
            {
                "AbilityId": "Orcs_Minion.Primary",
                "Slot": "Primary",
                "DisplayName": "Combo strike",
                "DamageType": "Ability",
                "BaseDamage": 12,
                "CooldownSec": 8,
                "RangeUU": 0,
                "EffectTags": ["Melee"],
            },
            {
                "AbilityId": "Orcs_Minion.Secondary",
                "Slot": "Secondary",
                "DisplayName": "War cry",
                "DamageType": "Ability",
                "BaseDamage": 8,
                "CooldownSec": 14,
                "RangeUU": 0,
                "EffectTags": ["BuffAura", "Melee"],
            },
        ],
        "DefenderAI": {
            "AggroStyle": "Chase",
            "IntruderResponse": "Fight",
            "HarassInvaders": True,
            "LeashRadiusUU": 1200,
        },
    },
    "Orcs.Elite": {
        "DisplayName": "Orc Elite",
        "StatSource": "HandTuned",
        "Vitals": {
            "MaxHealth": 68,
            "BasicDamage": 10,
            "BasicDefense": 17,
            "AbilityDefense": 14,
            "CleavePct": 0.18,
        },
        "Ratings": {
            "BasicDamage": 6,
            "AbilityDamage": 5,
            "Durability": 6,
            "Mobility": 5,
            "Intelligence": 2,
            "Wisdom": 2,
        },
        "Abilities": [
            {
                "AbilityId": "Orcs_Elite.Primary",
                "Slot": "Primary",
                "DisplayName": "Heavy cleave",
                "DamageType": "Ability",
                "BaseDamage": 20,
                "CooldownSec": 8,
                "RangeUU": 0,
                "EffectTags": ["Melee", "Cleave"],
            },
            {
                "AbilityId": "Orcs_Elite.Secondary",
                "Slot": "Secondary",
                "DisplayName": "Bull rush",
                "DamageType": "Ability",
                "BaseDamage": 16,
                "CooldownSec": 11,
                "RangeUU": 350,
                "EffectTags": ["Melee", "Dash"],
            },
        ],
        "DefenderAI": {
            "AggroStyle": "Chase",
            "IntruderResponse": "Fight",
            "HarassInvaders": True,
            "LeashRadiusUU": 1000,
        },
    },
    "Orcs.Boss": {
        "DisplayName": "Orc Boss",
        "StatSource": "HandTuned",
        "Vitals": {
            "MaxHealth": 175,
            "BasicDamage": 13,
            "BasicDefense": 20,
            "AbilityDefense": 16,
            "CleavePct": 0.2,
        },
        "Ratings": {
            "BasicDamage": 7,
            "AbilityDamage": 4,
            "Durability": 8,
            "Mobility": 4,
            "Intelligence": 2,
            "Wisdom": 2,
        },
        "Abilities": [
            {
                "AbilityId": "Orcs_Boss.Primary",
                "Slot": "Primary",
                "DisplayName": "Guard / damage soak taunt",
                "DamageType": "Ability",
                "BaseDamage": 22,
                "CooldownSec": 10,
                "RangeUU": 0,
                "EffectTags": ["Melee", "AOE"],
            },
            {
                "AbilityId": "Orcs_Boss.Secondary",
                "Slot": "Secondary",
                "DisplayName": "Shield slam",
                "DamageType": "Ability",
                "BaseDamage": 28,
                "CooldownSec": 13,
                "RangeUU": 0,
                "EffectTags": ["Melee", "AOE"],
            },
        ],
        "DefenderAI": {
            "AggroStyle": "Chase",
            "IntruderResponse": "Fight",
            "HarassInvaders": True,
            "LeashRadiusUU": 900,
        },
    },
}


def apply_worker_template(sheet: dict[str, Any]) -> dict[str, Any]:
    if sheet.get("Tier") != "Worker":
        return sheet
    family = sheet["FamilyId"]
    ability_prefix = family.replace(".", "_")
    sheet["DisplayName"] = WORKER_DISPLAY_NAMES.get(family, f"{family} Worker")
    sheet["StatSource"] = "HandTuned"
    sheet["Vitals"].update(
        {
            "MaxHealth": 18,
            "BasicDamage": 2,
            "BasicDefense": 14,
            "AbilityDefense": 11,
            "CleavePct": 0.05,
        }
    )
    sheet["Ratings"] = {
        "BasicDamage": 2,
        "AbilityDamage": 2,
        "Durability": 2,
        "Mobility": 3,
        "Intelligence": 2,
        "Wisdom": 2,
    }
    sheet["Abilities"] = [
        {
            "AbilityId": f"{ability_prefix}_Worker.Primary",
            "Slot": "Primary",
            "DisplayName": "Dig",
            "DamageType": "None",
            "BaseDamage": 0,
            "CooldownSec": 6,
            "RangeUU": 0,
            "EffectTags": ["Dig"],
        },
        {
            "AbilityId": f"{ability_prefix}_Worker.Secondary",
            "Slot": "Secondary",
            "DisplayName": "Reset traps",
            "DamageType": "None",
            "BaseDamage": 0,
            "CooldownSec": 10,
            "RangeUU": 150,
            "EffectTags": ["TrapReset"],
        },
    ]
    sheet["DefenderAI"].update(
        {
            "IntruderResponse": "Flee",
            "FleeOnRaidIntel": True,
            "AggroStyle": "Guard",
            "LeashRadiusUU": 600,
            "HarassInvaders": False,
        }
    )
    return sheet


def apply_defender_overrides(sheet: dict[str, Any]) -> dict[str, Any]:
    if sheet.get("Tier") == "Worker":
        sheet = apply_worker_template(sheet)
    patch = DEFENDER_SHEET_OVERRIDES.get(sheet["SheetId"])
    if not patch:
        return sheet
    if "ReferenceTier" in patch:
        sheet["ReferenceTier"] = patch["ReferenceTier"]
    if "DisplayName" in patch:
        sheet["DisplayName"] = patch["DisplayName"]
    if "StatSource" in patch:
        sheet["StatSource"] = patch["StatSource"]
    if "Vitals" in patch:
        sheet["Vitals"].update(patch["Vitals"])
    if "Ratings" in patch:
        sheet["Ratings"].update(patch["Ratings"])
    if "Abilities" in patch:
        sheet["Abilities"] = patch["Abilities"]
    if "DefenderAI" in patch:
        sheet["DefenderAI"].update(patch["DefenderAI"])
    return sheet


def main() -> None:
    sheets: list[dict[str, Any]] = []

    for family in DEFENDER_FAMILIES:
        for tier in ("Worker", "Minion", "Elite", "Boss", "Hero"):
            sheets.append(apply_defender_overrides(defender_sheet(family, tier)))

    invaders = [
        _wanderer_common_sheet(),
        _thief_common_sheet(),
        invader_sheet("Invader.HeroicWarrior.Common", "Hero", "Heroic Warrior", "/Game/Invaders/HeroicWarrior/SM_HeroicWarrior", line="Common"),
        invader_sheet("Invader.DwarfFighter.Common", "Fighter", "Dwarf Fighter", "/Game/Invaders/Dwarf/Dwarf_Fighter", line="Common"),
        invader_sheet("Invader.ElfArcher.Common", "Archer", "Elf Bowman", "/Game/Invaders/Elf-Archer/Elf_Archer", line="Common"),
        invader_sheet("Invader.Thelma.Common", "Fighter", "Thelma", "/Game/Invaders/Thelma/Invader_Thelma", line="Common"),
        invader_sheet("Invader.Wizard.Common", "Mage", "Wizard", "/Game/Invaders/Wizard/Invader_Wizard_Unreal", line="Common"),
        invader_sheet("Invader.ElfMage.Common", "Mage", "Elf Mage", "/Game/Invaders/Elf-Mage/Elf_Mage", line="Common"),
        invader_sheet("Invader.Paladin.Common", "Tank", "Paladin", "/Game/Invaders/Paladin/Paladin", line="Common"),
        invader_sheet("Invader.GothicKnight.Common", "Bruiser", "Gothic Knight", "/Game/Invaders/GKnight/Meshes/SK_GothicKnight_VA", line="Common"),
        invader_sheet("Invader.Champion.Terra", "Champion", "Terra", "/Game/Invaders/ParagonTerra", line="Paragon", paragon_ref="Terra", ref_tier="Veteran"),
        invader_sheet("Invader.Tank.Greystone", "Tank", "Greystone", "/Game/Invaders/ParagonGreystone", line="Paragon", paragon_ref="Greystone", ref_tier="Veteran"),
        invader_sheet("Invader.Tank.Rampage", "Tank", "Rampage", "/Game/ParagonRampage/Characters/Heroes/Rampage/Meshes/Rampage", line="Paragon", paragon_ref="Rampage", ref_tier="Veteran"),
        invader_sheet("Invader.Bruiser.Kwang", "Bruiser", "Kwang", "/Game/Invaders/ParagonKwang", line="Paragon", paragon_ref="Kwang", ref_tier="Standard"),
        invader_sheet("Invader.Mage.Phase", "Mage", "Phase", "/Game/Invaders/ParagonPhase", line="Paragon", paragon_ref="Phase", ref_tier="Standard"),
        invader_sheet("Invader.Mage.Aurora", "Mage", "Aurora", "/Game/Invaders/ParagonAurora", line="Paragon", paragon_ref="Aurora", ref_tier="Standard"),
        invader_sheet("Invader.Archer.Sparrow", "Archer", "Sparrow", "/Game/Invaders/ParagonSparrow", line="Paragon", paragon_ref="Sparrow", ref_tier="Standard"),
        invader_sheet("Invader.Cleric.Zinx", "Cleric", "Zinx", "/Game/Invaders/ParagonZinx", line="Paragon", paragon_ref="Zinx", ref_tier="Standard"),
        _warrior_queen_sheet(),
        _firefly_cleric_sheet(),
    ]
    for display, archetype, ability_key in PARAGON_INVADER_CATALOG:
        ref = display
        if ref in PARAGON_INVADER_ASSIGNED:
            continue
        sid = sheet_id_for_paragon(archetype, display)
        invaders.append(
            invader_sheet(
                sid,
                archetype,
                display,
                paragon_mesh_guess(ability_key),
                line="Paragon",
                paragon_ref=ability_key,
                ref_tier="Standard",
                knowledge=["Knows_MimicRisk"] if archetype == "Assassin" else None,
            )
        )
    sheets.extend(invaders)

    doc = {
        "$schema": "./character-sheet.schema.json",
        "schemaVersion": 3,
        "designDoc": "Design/Character-Sheets.md",
        "rules": {
            "ArchetypeIdRequired": "Every sheet row (Defender and Invader) must set ArchetypeId.",
            "SheetIdConvention": "Defender: Family.Tier — Invader: Invader.<Role>.<Variant>",
        },
        "defenderArchetypeDefaultsByTier": {
            "Worker": "Fighter",
            "Minion": "Fighter",
            "Elite": "Bruiser",
            "Boss": "Tank",
            "Hero": None,
        },
        "archetypeKits": ARCHETYPE_KITS,
        "sheets": sheets,
    }

    OUT.write_text(json.dumps(doc, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {len(sheets)} sheets to {OUT}")
    write_character_objects_todo(sheets)


if __name__ == "__main__":
    main()