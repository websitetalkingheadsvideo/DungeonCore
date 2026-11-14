"""TryDig failure feedback: dig only from reachable revealed edge."""
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:TryDig"}

BRANCH_NODES = [
    f"{GRID}:TryDig.K2Node_IfThenElse_4",  # bounds
    f"{GRID}:TryDig.K2Node_IfThenElse_5",  # revealed + undug
    f"{GRID}:TryDig.K2Node_IfThenElse_6",  # adjacent
]


def wire_msg(sid: str, branch_ref: str, text: str, y: int) -> None:
    infos = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": branch_ref}]}, sid)[
        "returnValue"
    ][0]
    if infos["output_pins"][1].get("connected_pins"):
        return
    msg = call_tool(
        BT,
        "create_node",
        {
            "graph": GRAPH,
            "type_id": "Variables|Default|SetStatusMsg",
            "pos": {"x": 15200, "y": y},
        },
        sid,
    )["returnValue"]["refPath"]
    tmr = call_tool(
        BT,
        "create_node",
        {
            "graph": GRAPH,
            "type_id": "Variables|Default|SetMsgTimer",
            "pos": {"x": 15520, "y": y},
        },
        sid,
    )["returnValue"]["refPath"]
    for node, pin_name, val in [
        (msg, "StatusMsg", text),
        (tmr, "MsgTimer", "3.0"),
    ]:
        pi = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": node}]}, sid)[
            "returnValue"
        ][0]
        p = next(p for p in pi["input_pins"] if p["name"] == pin_name)
        call_tool(BT, "set_pin_value", {"pin": p["pin_id"], "value": val}, sid)
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": {"direction": "EGPD_Output", "index_id": 1, "node": {"refPath": branch_ref}},
            "input_pin": {"direction": "EGPD_Input", "index_id": 0, "node": {"refPath": msg}},
        },
        sid,
    )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": {"direction": "EGPD_Output", "index_id": 0, "node": {"refPath": msg}},
            "input_pin": {"direction": "EGPD_Input", "index_id": 0, "node": {"refPath": tmr}},
        },
        sid,
    )
    print("wired", text[:40])


def main() -> None:
    sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
    if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
        call_tool(APP, "StopPIE", {}, sid)

    branches = call_tool(BT, "find_nodes", {"graph": GRAPH, "title": "Branch"}, sid)[
        "returnValue"
    ]
    if len(branches) < 4:
        raise RuntimeError(branches)
  # outermost..inner: 4=grid bounds, 5=revealed/undug, 6=adjacent, 7=mana
    by_y = sorted(
        branches,
        key=lambda b: call_tool(BT, "get_node_infos", {"nodes": [b]}, sid)["returnValue"][
            0
        ]["position"]["y"],
    )
    wire_msg(sid, by_y[0]["refPath"], "That tile is outside the dig grid", 400)
    wire_msg(sid, by_y[1]["refPath"], "Dig revealed floor only — expand from the edge", 520)
    wire_msg(sid, by_y[2]["refPath"], "Dig from a tile next to open floor", 640)

    call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
    call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
    print("TryDig feedback OK")


if __name__ == "__main__":
    main()