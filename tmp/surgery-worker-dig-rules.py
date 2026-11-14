"""Pin surgery: worker cap in BuyMonster; dig discount via RefreshDigCost + DigCostBase."""
from __future__ import annotations

import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool, new_session

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

REFRESH_DSL = """(fn RefreshDigCost ()
  (bind _base (Variables|Default|GetDigCostBase))
  (if (<= _base 0.0)
    (Variables|Default|SetDigCostBase (Variables|Default|GetDigCost)))
  (bind _workers (CallFunction|CountWorkers))
  (Variables|Default|SetWorkerCount _workers)
  (bind _workers_capped (Math|Integer|Min(Integer) _workers 3))
  (bind _mult (select (== _workers_capped 3) 0.25 (select (== _workers_capped 2) 0.5 (select (== _workers_capped 1) 0.75 1.0))))
  (Variables|Default|SetDigCost (* (Variables|Default|GetDigCostBase) _mult)))"""

WORKER_CAP_DSL = """(fn WorkerCapReached ()
  (bind _w (CallFunction|CountWorkers))
  (return (>= _w 3)))"""


def sid_open() -> str:
    try:
        sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
        call_tool(APP, "IsPIERunning", {}, sid)
    except Exception:
        sid = new_session()
    if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
        call_tool(APP, "StopPIE", {}, sid)
    return sid


def ensure_dig_cost_base(sid: str) -> None:
    vars_ = call_tool(
        BT, "list_variables", {"blueprint": {"refPath": GRID}}, sid
    ).get("returnValue", [])
    if "DigCostBase" not in vars_:
        call_tool(
            BT,
            "add_variable",
            {"blueprint": {"refPath": GRID}, "name": "DigCostBase", "type_name": "real"},
            sid,
        )
    if "WorkerCount" not in vars_:
        call_tool(
            BT,
            "add_variable",
            {"blueprint": {"refPath": GRID}, "name": "WorkerCount", "type_name": "int"},
            sid,
        )


def ensure_blueprint_fn(sid: str, name: str, code: str, return_type: str) -> None:
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


def ensure_refresh_fn(sid: str) -> None:
    funcs = call_tool(
        BT, "list_functions", {"blueprint": {"refPath": GRID}}, sid
    ).get("returnValue", [])
    names = [f["name"] if isinstance(f, dict) else f for f in funcs]
    if "RefreshDigCost" not in names:
        call_tool(
            BT,
            "add_function_graph",
            {"blueprint": {"refPath": GRID}, "graph_name": "RefreshDigCost"},
            sid,
        )
    ensure_blueprint_fn(sid, "RefreshDigCost", REFRESH_DSL, "real")


def call_fn_after_node(sid: str, graph_ref: str, after_node: str, y: int) -> None:
    """Insert Call RefreshDigCost on exec chain after after_node (pure exec prev)."""
    existing = call_tool(
        BT,
        "find_nodes",
        {"graph": {"refPath": graph_ref}, "title": "Refresh Dig Cost"},
        sid,
    ).get("returnValue", [])
    if existing:
        print("RefreshDigCost call already in", graph_ref)
        return
    prev = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": after_node}]}, sid)[
        "returnValue"
    ][0]
    if not prev["output_pins"][0].get("connected_pins"):
        raise RuntimeError(f"{after_node} has no exec out")
    conn = prev["output_pins"][0]["connected_pins"][0]
    nxt = conn["node"]["refPath"]
    input_pin = conn.get("pin_id") or {
        "direction": "EGPD_Input",
        "index_id": conn.get("index_id", 0),
        "node": {"refPath": nxt},
    }
    call_tool(
        BT,
        "break_pins",
        {
            "output_pin": prev["output_pins"][0]["pin_id"],
            "input_pin": input_pin,
        },
        sid,
    )
    call_node = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph_ref},
            "type_id": "CallFunction|RefreshDigCost",
            "pos": {"x": prev["position"]["x"] + 280, "y": y},
        },
        sid,
    )["returnValue"]["refPath"]
    cinfo = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": call_node}]}, sid)[
        "returnValue"
    ][0]
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": prev["output_pins"][0]["pin_id"],
            "input_pin": cinfo["input_pins"][0]["pin_id"],
        },
        sid,
    )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": cinfo["output_pins"][0]["pin_id"],
            "input_pin": {
                "direction": "EGPD_Input",
                "index_id": 0,
                "node": {"refPath": nxt},
            },
        },
        sid,
    )
    print("wired RefreshDigCost after", after_node)


def wire_init_base(sid: str) -> None:
    graph = f"{GRID}:InitGrid"
    if call_tool(
        BT,
        "find_nodes",
        {"graph": {"refPath": graph}, "title": "Refresh Dig Cost"},
        sid,
    ).get("returnValue"):
        print("InitGrid refresh already wired")
        return
    calib = call_tool(
        BT,
        "find_nodes",
        {"graph": {"refPath": graph}, "title": "Rebuild"},
        sid,
    )["returnValue"]
    if not calib:
        raise RuntimeError("InitGrid Rebuild node missing")
    calib = calib[-1]["refPath"]
    calib_i = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": calib}]}, sid)[
        "returnValue"
    ][0]
    set_base = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph},
            "type_id": "Variables|Default|SetDigCostBase",
            "pos": {"x": calib_i["position"]["x"] + 240, "y": calib_i["position"]["y"]},
        },
        sid,
    )["returnValue"]["refPath"]
    get_cost = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph},
            "type_id": "Variables|Default|GetDigCost",
            "pos": {"x": calib_i["position"]["x"] + 80, "y": calib_i["position"]["y"] + 80},
        },
        sid,
    )["returnValue"]["refPath"]
    sinfo = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": set_base}]}, sid)[
        "returnValue"
    ][0]
    ginfo = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": get_cost}]}, sid)[
        "returnValue"
    ][0]
    # Rebuild -> SetDigCostBase -> (downstream if any)
    outs = calib_i["output_pins"][0].get("connected_pins") or []

    def pin_ref(conn: dict) -> dict:
        return conn.get("pin_id") or {
            "direction": "EGPD_Input",
            "index_id": conn.get("index_id", 0),
            "node": {"refPath": conn["node"]["refPath"]},
        }

    if outs:
        call_tool(
            BT,
            "break_pins",
            {
                "output_pin": calib_i["output_pins"][0]["pin_id"],
                "input_pin": pin_ref(outs[0]),
            },
            sid,
        )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": calib_i["output_pins"][0]["pin_id"],
            "input_pin": sinfo["input_pins"][0]["pin_id"],
        },
        sid,
    )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": ginfo["output_pins"][0]["pin_id"],
            "input_pin": sinfo["input_pins"][1]["pin_id"],
        },
        sid,
    )
    if outs:
        call_tool(
            BT,
            "connect_pins",
            {
                "output_pin": sinfo["output_pins"][0]["pin_id"],
                "input_pin": pin_ref(outs[0]),
            },
            sid,
        )
    refresh = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph},
            "type_id": "CallFunction|RefreshDigCost",
            "pos": {"x": sinfo["position"]["x"] + 240, "y": sinfo["position"]["y"]},
        },
        sid,
    )["returnValue"]["refPath"]
    rinfo = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": refresh}]}, sid)[
        "returnValue"
    ][0]
    if outs:
        call_tool(
            BT,
            "break_pins",
            {
                "output_pin": sinfo["output_pins"][0]["pin_id"],
                "input_pin": pin_ref(outs[0]),
            },
            sid,
        )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": sinfo["output_pins"][0]["pin_id"],
            "input_pin": rinfo["input_pins"][0]["pin_id"],
        },
        sid,
    )
    if outs:
        call_tool(
            BT,
            "connect_pins",
            {
                "output_pin": rinfo["output_pins"][0]["pin_id"],
                "input_pin": pin_ref(outs[0]),
            },
            sid,
        )
    print("InitGrid: DigCostBase + RefreshDigCost wired")


def wire_spawn_refresh(sid: str) -> None:
    graph = f"{GRID}:SpawnMonster"
    hides = call_tool(
        BT,
        "find_nodes",
        {"graph": {"refPath": graph}, "title": "Hide Bone by Name"},
        sid,
    ).get("returnValue", [])
    # last exec node before function end — use last array Add MonsterIdx
    adds = call_tool(
        BT,
        "find_nodes",
        {"graph": {"refPath": graph}, "title": "ADD"},
        sid,
    )["returnValue"]
    # pick MonsterIdx add: highest X
    target = None
    best_x = -1
    for node in adds:
        info = call_tool(BT, "get_node_infos", {"nodes": [node]}, sid)["returnValue"][
            0
        ]
        if info["position"]["x"] > best_x and info["output_pins"][0].get(
            "connected_pins"
        ):
            best_x = info["position"]["x"]
            target = node["refPath"]
    if not target and hides:
        target = hides[-1]["refPath"]
    if not target:
        raise RuntimeError("SpawnMonster: no splice point")
    call_fn_after_node(sid, graph, target, 400)


def wire_buy_monster_refresh(sid: str) -> None:
    graph = f"{GRID}:BuyMonster"
    calls = call_tool(
        BT,
        "find_nodes",
        {"graph": {"refPath": graph}, "title": "Refresh Dig Cost"},
        sid,
    ).get("returnValue", [])
    if len(calls) >= 2:
        print("BuyMonster RefreshDigCost already at entry")
        return
    entry = call_tool(
        BT, "find_nodes", {"graph": {"refPath": graph}, "title": "BuyMonster"}, sid
    )["returnValue"][0]["refPath"]
    call_fn_after_node(sid, graph, entry, 80)


def wire_buy_monster_cap(sid: str) -> None:
    graph = f"{GRID}:BuyMonster"
    if len(
        call_tool(
            BT,
            "find_nodes",
            {"graph": {"refPath": graph}, "title": "WorkerCapReached"},
            sid,
        ).get("returnValue", [])
    ) > 1:
        print("BuyMonster worker cap already wired")
        return
    branches = call_tool(
        BT, "find_nodes", {"graph": {"refPath": graph}, "title": "Branch"}, sid
    )["returnValue"]
    # outer branch checks boss — lowest Y often first
    infos = [
        (
            call_tool(BT, "get_node_infos", {"nodes": [b]}, sid)["returnValue"][0],
            b["refPath"],
        )
        for b in branches
    ]
    boss_branch = None
    for info, ref in infos:
        cond = info["input_pins"][1].get("connected_pins", [])
        for c in cond:
            cnode = call_tool(
                BT, "get_node_infos", {"nodes": [{"refPath": c["node"]["refPath"]}]}, sid
            )["returnValue"][0]
            if any("BossSpawned" in str(p) for p in cnode.get("input_pins", []) + cnode.get("output_pins", [])):
                boss_branch = ref
                break
            t = cnode.get("title", "")
            if "Boss" in t:
                boss_branch = ref
                break
    if not boss_branch:
        boss_branch = sorted(infos, key=lambda x: x[0]["position"]["y"])[0][1]

    binfo = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": boss_branch}]}, sid)[
        "returnValue"
    ][0]
    else_pin = binfo["output_pins"][1]
    if not else_pin.get("connected_pins"):
        raise RuntimeError("boss branch missing else")
    else_exec = else_pin["connected_pins"][0]["node"]["refPath"]

    # Tier==0 (reuse cost-tier equal) AND WorkerCount>=3
    tier_eq0 = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid:BuyMonster.K2Node_PromotableOperator_0"
    cap_call = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph},
            "type_id": "CallFunction|WorkerCapReached",
            "pos": {"x": binfo["position"]["x"] - 80, "y": binfo["position"]["y"] + 140},
        },
        sid,
    )["returnValue"]["refPath"]
    and_node = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph},
            "type_id": "Math|Boolean|ANDBoolean",
            "pos": {"x": binfo["position"]["x"] + 80, "y": binfo["position"]["y"] + 100},
        },
        sid,
    )["returnValue"]["refPath"]
    cap_branch = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph},
            "type_id": "Utilities|FlowControl|Branch",
            "pos": {"x": binfo["position"]["x"] + 200, "y": binfo["position"]["y"] + 40},
        },
        sid,
    )["returnValue"]["refPath"]
    alert = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph},
            "type_id": "Class|WBPDungeonHUD|ShowAlert",
            "pos": {"x": binfo["position"]["x"] + 480, "y": binfo["position"]["y"] + 20},
        },
        sid,
    )["returnValue"]["refPath"]

    eq_i = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": tier_eq0}]}, sid)[
        "returnValue"
    ][0]
    cap_i = call_tool(
        BT, "get_node_infos", {"nodes": [{"refPath": cap_call}]}, sid
    )["returnValue"][0]
    and_i = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": and_node}]}, sid)[
        "returnValue"
    ][0]
    call_tool(
        BT,
        "connect_pins",
        {"output_pin": eq_i["output_pins"][0]["pin_id"], "input_pin": and_i["input_pins"][0]["pin_id"]},
        sid,
    )
    cap_bool = next(p for p in cap_i["output_pins"] if p["name"] == "ReturnValue")
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": cap_bool["pin_id"],
            "input_pin": and_i["input_pins"][1]["pin_id"],
        },
        sid,
    )
    cb_i = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": cap_branch}]}, sid)[
        "returnValue"
    ][0]
    call_tool(
        BT,
        "connect_pins",
        {"output_pin": and_i["output_pins"][0]["pin_id"], "input_pin": cb_i["input_pins"][1]["pin_id"]},
        sid,
    )

    # Break boss else -> mana branch; insert cap_branch on else chain
    else_conn = else_pin["connected_pins"][0]
    else_in = else_conn.get("pin_id") or {
        "direction": "EGPD_Input",
        "index_id": else_conn.get("index_id", 0),
        "node": {"refPath": else_conn["node"]["refPath"]},
    }
    call_tool(
        BT,
        "break_pins",
        {
            "output_pin": else_pin["pin_id"],
            "input_pin": else_in,
        },
        sid,
    )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": else_pin["pin_id"],
            "input_pin": cb_i["input_pins"][0]["pin_id"],
        },
        sid,
    )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": cb_i["output_pins"][1]["pin_id"],
            "input_pin": {
                "direction": "EGPD_Input",
                "index_id": 0,
                "node": {"refPath": else_exec},
            },
        },
        sid,
    )
    a_i = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": alert}]}, sid)[
        "returnValue"
    ][0]
    call_tool(
        BT,
        "set_pin_value",
        {
            "pin": next(p for p in a_i["input_pins"] if p["name"] == "Msg")["pin_id"],
            "value": "ONLY THREE WORKERS ALLOWED  -  you already have 3 workers",
        },
        sid,
    )
    hud_get = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": graph},
            "type_id": "Variables|Default|GetHUD",
            "pos": {"x": binfo["position"]["x"] + 320, "y": binfo["position"]["y"] - 40},
        },
        sid,
    )["returnValue"]["refPath"]
    hg = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": hud_get}]}, sid)[
        "returnValue"
    ][0]
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": hg["output_pins"][0]["pin_id"],
            "input_pin": a_i["input_pins"][0]["pin_id"],
        },
        sid,
    )
    exec_in = next(p for p in a_i["input_pins"] if p["name"] == "execute")
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": cb_i["output_pins"][0]["pin_id"],
            "input_pin": exec_in["pin_id"],
        },
        sid,
    )
  # mark: set pin default on alert node title for find — use set_pin_value only
    print("BuyMonster worker cap wired")


def main() -> None:
    sid = sid_open()
    ensure_dig_cost_base(sid)
    # CountWorkers / GetEffectiveDigCost may already exist from partial apply
    ensure_refresh_fn(sid)
    ensure_blueprint_fn(sid, "WorkerCapReached", WORKER_CAP_DSL, "bool")
    wire_init_base(sid)
    wire_spawn_refresh(sid)
    wire_buy_monster_refresh(sid)
    wire_buy_monster_cap(sid)
    call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
    call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
    print("surgery-worker-dig-rules OK")


if __name__ == "__main__":
    main()