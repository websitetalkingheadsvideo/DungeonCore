"""Wire TryDig mana-fail else branch to StatusMsg + MsgTimer."""
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:TryDig"}
BRANCH = f"{GRID}:TryDig.K2Node_IfThenElse_7"

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

# StatusMsg
msg = call_tool(
    BT,
    "create_node",
    {
        "graph": GRAPH,
        "type_id": "Variables|Default|SetStatusMsg",
        "pos": {"x": 14560, "y": 200},
    },
    sid,
)
msg_node = msg["returnValue"]["refPath"]
msg_infos = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": msg_node}]}, sid)[
    "returnValue"
][0]
status_pin = next(p for p in msg_infos["input_pins"] if p["name"] == "StatusMsg")
call_tool(
    BT,
    "set_pin_value",
    {
        "pin": status_pin["pin_id"],
        "value": "Not enough mana to dig",
    },
    sid,
)

# MsgTimer = 3
timer = call_tool(
    BT,
    "create_node",
    {
        "graph": GRAPH,
        "type_id": "Variables|Default|SetMsgTimer",
        "pos": {"x": 14880, "y": 200},
    },
    sid,
)
timer_node = timer["returnValue"]["refPath"]
timer_infos = call_tool(
    BT, "get_node_infos", {"nodes": [{"refPath": timer_node}]}, sid
)["returnValue"][0]
timer_pin = next(p for p in timer_infos["input_pins"] if p["name"] == "MsgTimer")
call_tool(BT, "set_pin_value", {"pin": timer_pin["pin_id"], "value": "3.0"}, sid)

# Else -> SetStatusMsg -> SetMsgTimer
call_tool(
    BT,
    "connect_pins",
    {
        "output_pin": {
            "direction": "EGPD_Output",
            "index_id": 1,
            "node": {"refPath": BRANCH},
        },
        "input_pin": {
            "direction": "EGPD_Input",
            "index_id": 0,
            "node": {"refPath": msg_node},
        },
    },
    sid,
)
call_tool(
    BT,
    "connect_pins",
    {
        "output_pin": {
            "direction": "EGPD_Output",
            "index_id": 0,
            "node": {"refPath": msg_node},
        },
        "input_pin": {
            "direction": "EGPD_Input",
            "index_id": 0,
            "node": {"refPath": timer_node},
        },
    },
    sid,
)

call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
print("TryDig: mana fail shows status for 3s")