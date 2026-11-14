"""Workers: max 3 alive; each reduces dig mana cost 25% (floor 25% of base)."""
from __future__ import annotations

import re
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool, new_session

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

COUNT_WORKERS_DSL = """(fn CountWorkers ()
  (Variables|Default|SetDigCount 0)
  (for _index (range (Utilities|Array|Length (Variables|Default|GetMonsterIdx)))
    (bind _tier (Math|Integer|%(Integer) (Utilities|Array|Get(acopy) (Variables|Default|GetMonsterIdx) _index) 4))
    (if (== _tier 0)
      (Variables|Default|SetDigCount (+ (Variables|Default|GetDigCount) 1))))
  (return (Variables|Default|GetDigCount)))"""

GET_EFFECTIVE_DIG_COST_DSL = """(fn GetEffectiveDigCost ()
  (bind _workers (CallFunction|CountWorkers))
  (bind _workers_capped (Math|Integer|Min(Integer) _workers 3))
  (bind _mult (select (== _workers_capped 3) 0.25 (select (== _workers_capped 2) 0.5 (select (== _workers_capped 1) 0.75 1.0))))
  (return (* (Variables|Default|GetDigCost) _mult)))"""


def ensure_session() -> str:
    sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
    try:
        call_tool(APP, "IsPIERunning", {}, sid)
    except Exception:
        sid = new_session()
    if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
        call_tool(APP, "StopPIE", {}, sid)
    return sid


def ensure_function(sid: str, name: str, code: str, return_type: str) -> None:
    funcs = call_tool(
        BT,
        "list_functions",
        {"blueprint": {"refPath": GRID}},
        sid,
    ).get("returnValue", [])
    names = [f["name"] if isinstance(f, dict) else f for f in funcs]
    if name not in names:
        call_tool(
            BT,
            "add_function_graph",
            {"blueprint": {"refPath": GRID}, "graph_name": name},
            sid,
        )
    gref = {"refPath": f"{GRID}:{name}"}
    try:
        call_tool(
            BT,
            "add_function_param",
            {
                "graph": gref,
                "param_name": "ReturnValue",
                "param_type": return_type,
                "input_param": False,
            },
            sid,
        )
    except RuntimeError as exc:
        if "already exists" not in str(exc):
            raise
    call_tool(BT, "write_graph_dsl", {"graph": gref, "code": code.strip()}, sid)


TRY_DIG_DSL = """(fn TryDig (X Y)
  (bind _gridsize (Variables|Default|GetGridSize))
  (bind _returnvalue (* Y _gridsize))
  (bind _returnvalue_1 (+ _returnvalue X))
  (bind _cells (Variables|Default|GetCells))
  (bind _digcost (CallFunction|GetEffectiveDigCost))
  (if (and (and (>= X 1) (< X (- _gridsize 1))) (and (>= Y 1) (< Y (- _gridsize 1))))
    (if (and (Utilities|Array|Get(acopy) (Variables|Default|GetRevealed) _returnvalue_1) (== (Utilities|Array|Get(acopy) _cells _returnvalue_1) 0))
      (if (or (or (!= (Utilities|Array|Get(acopy) _cells (- _returnvalue_1 1)) 0) (!= (Utilities|Array|Get(acopy) _cells (+ _returnvalue_1 1)) 0)) (or (!= (Utilities|Array|Get(acopy) _cells (- _returnvalue_1 _gridsize)) 0) (!= (Utilities|Array|Get(acopy) _cells (+ _returnvalue_1 _gridsize)) 0)))
        (if (>= (Variables|Default|GetMana) _digcost)
          (Variables|Default|SetMana (- (Variables|Default|GetMana) _digcost))
          (Utilities|Array|SetArrayElem (Variables|Default|GetCells) _returnvalue_1 1)
          (CallFunction|RemoveLampsAt _returnvalue_1)
          (CallFunction|RevealAround X Y)
          (Calibration|Rebuild)
          (CallFunction|PlaceLamp X Y)
          (else
            (Class|WBPDungeonHUD|ShowAlert (Variables|Default|GetHUD) (Utilities|String|Append "NOT ENOUGH MANA TO DIG  -  needs " (Utilities|String|Append (Utilities|String|ToString(Integer) (Math|Float|Round _digcost)) (Utilities|String|Append ", you have " (Utilities|String|ToString(Integer) (Math|Float|Floor (Variables|Default|GetMana)))))))))
        (else
          (Variables|Default|SetStatusMsg "Dig from a tile next to open floor")
          (Variables|Default|SetMsgTimer 3.0)))
      (else
        (Variables|Default|SetStatusMsg "Dig revealed floor only - expand from the edge")
        (Variables|Default|SetMsgTimer 3.0)))
    (else
      (Variables|Default|SetStatusMsg "That tile is outside the dig grid")
      (Variables|Default|SetMsgTimer 3.0))))"""


def patch_try_dig(sid: str) -> None:
    gref = f"{GRID}:TryDig"
    dsl = call_tool(BT, "read_graph_dsl", {"graph": {"refPath": gref}}, sid)[
        "returnValue"
    ]
    if "GetEffectiveDigCost" in dsl:
        print("TryDig already uses GetEffectiveDigCost")
        return
    if "(Variables|Default|GetDigCost)" not in dsl:
        raise RuntimeError(f"TryDig: no GetDigCost\n{dsl}")
    patched = dsl.replace(
        "(Variables|Default|GetDigCost)",
        "(CallFunction|GetEffectiveDigCost)",
    )
    if patched.count("(") != patched.count(")"):
        raise RuntimeError("TryDig: unbalanced parens")
    call_tool(
        BT,
        "write_graph_dsl",
        {"graph": {"refPath": gref}, "code": patched},
        sid,
    )


def patch_buy_monster(sid: str) -> None:
    gref = f"{GRID}:BuyMonster"
    dsl = call_tool(BT, "read_graph_dsl", {"graph": {"refPath": gref}}, sid)[
        "returnValue"
    ]
    if "ONLY THREE WORKERS" in dsl:
        print("BuyMonster worker cap already patched")
        return

    worker_block = """  (if (and (== Tier 3) (Variables|Default|GetBossSpawned))
    (Class|WBPDungeonHUD|ShowAlert (Variables|Default|GetHUD) (Utilities|String|Append "ONLY ONE BOSS ALLOWED  -  " (Utilities|String|Append _returnvalue_10 " is already summoned")))
    (elif (and (== Tier 0) (>= (CallFunction|CountWorkers) 3))
      (Class|WBPDungeonHUD|ShowAlert (Variables|Default|GetHUD) "ONLY THREE WORKERS ALLOWED  -  dismiss one before summoning another"))
      (elif (>= (Variables|Default|GetMana) _returnvalue_16)
        (Variables|Default|SetMana (- (Variables|Default|GetMana) _returnvalue_16))
        (Variables|Default|SetBossSpawned (or (Variables|Default|GetBossSpawned) (== Tier 3)))
        (CallFunction|SpawnMonster Tier)
        (CallFunction|CloseMonsterMenu)
        (else
          (Class|WBPDungeonHUD|ShowAlert (Variables|Default|GetHUD) (Utilities|String|Append "NOT ENOUGH MANA FOR " (Utilities|String|Append _returnvalue_10 (Utilities|String|Append "  -  needs " (Utilities|String|Append (Utilities|String|ToString(Integer) (Math|Float|Round _returnvalue_16)) (Utilities|String|Append ", you have " (Utilities|String|ToString(Integer) (Math|Float|Floor (Variables|Default|GetMana)))))))))))"""

    m = re.search(
        r"\(if \(and \(== Tier 3\) \(Variables\|Default\|GetBossSpawned\)\).*?\)\)\)\)\)",
        dsl,
        flags=re.DOTALL,
    )
    if not m:
        raise RuntimeError(f"BuyMonster: boss if-block not found\n{dsl}")
    patched = dsl[: m.start()] + worker_block + dsl[m.end() :]
    if patched.count("(") != patched.count(")"):
        raise RuntimeError("BuyMonster: unbalanced parens")
    call_tool(BT, "write_graph_dsl", {"graph": {"refPath": gref}, "code": patched}, sid)


def patch_spawn_monster(sid: str) -> None:
    """Register spawned creature in parallel monster arrays (tier = meshIndex % 4)."""
    gref = f"{GRID}:SpawnMonster"
    dsl = call_tool(BT, "read_graph_dsl", {"graph": {"refPath": gref}}, sid)[
        "returnValue"
    ]
    if "GetMonsterIdx" in dsl and "MonsterCell" in dsl:
        print("SpawnMonster already registers monster arrays")
        return

    register_tail = """
  (Utilities|Array|Add (Variables|Default|GetMonsterComps) _returnvalue_3)
  (Utilities|Array|Add (Variables|Default|GetMonsterIdx) _returnvalue_2)
  (Utilities|Array|Add (Variables|Default|GetMonsterCell) (Variables|Default|GetMenuCell))
  (Utilities|Array|Add (Variables|Default|GetMonsterTarget) (Variables|Default|GetMenuCell))
  (Utilities|Array|Add (Variables|Default|GetMonsterNext) (Variables|Default|GetMenuCell))
  (Utilities|Array|Add (Variables|Default|GetMonsterMoving) false)
  (Utilities|Array|Add (Variables|Default|GetMonsterOfs) (Math|Vector|MakeVector 0.0 0.0 0.0))
  (Utilities|Array|Add (Variables|Default|GetMonsterPos) (Math|Vector|MakeVector 0.0 0.0 0.0))"""

    if "(Utilities|Array|Add 0 _returnvalue_3)" in dsl:
        dsl = dsl.replace(
            "(Utilities|Array|Add 0 _returnvalue_3)",
            register_tail.strip(),
            1,
        )
    else:
        insert_at = dsl.rfind("(if (== _returnvalue_2 3)")
        if insert_at < 0:
            raise RuntimeError(f"SpawnMonster: cannot find insert point\n{dsl}")
        dsl = dsl[:insert_at] + register_tail + "\n  " + dsl[insert_at:]

    if dsl.count("(") != dsl.count(")"):
        raise RuntimeError("SpawnMonster: unbalanced parens")
    call_tool(BT, "write_graph_dsl", {"graph": {"refPath": gref}, "code": dsl}, sid)


def main() -> None:
    sid = ensure_session()
    ensure_function(sid, "CountWorkers", COUNT_WORKERS_DSL, "int")
    ensure_function(sid, "GetEffectiveDigCost", GET_EFFECTIVE_DIG_COST_DSL, "real")
    patch_spawn_monster(sid)
    patch_try_dig(sid)
    patch_buy_monster(sid)
    call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
    call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
    verify = call_tool(
        BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TryDig"}}, sid
    )["returnValue"]
    if "GetEffectiveDigCost" not in verify:
        raise RuntimeError("verify TryDig failed")
    bm = call_tool(
        BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:BuyMonster"}}, sid
    )["returnValue"]
    if "ONLY THREE WORKERS" not in bm:
        raise RuntimeError("verify BuyMonster failed")
    print("Worker dig rules applied OK")


if __name__ == "__main__":
    main()