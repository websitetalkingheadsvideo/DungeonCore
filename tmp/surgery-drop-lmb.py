"""Replace Pressed<0 with LMB WasReleased on IfThenElse_7 via pin surgery."""
import json
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:UpdateDrag"}

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

# Disconnect Pressed < 0 from drop-branch AND (operator_21 -> and_13 input A)
SKIP_BREAK = True
break_out = {
    "output_pin": {
        "direction": "EGPD_Output",
        "index_id": 0,
        "node": {"refPath": f"{GRID}:UpdateDrag.K2Node_PromotableOperator_21"},
    },
    "input_pin": {
        "direction": "EGPD_Input",
        "index_id": 0,
        "node": {"refPath": f"{GRID}:UpdateDrag.K2Node_CommutativeAssociativeBinaryOperator_13"},
    },
}
if not SKIP_BREAK:
    print("break", call_tool(BT, "break_pins", break_out, sid))

# Create nodes near AND
pc = call_tool(
    BT,
    "create_node",
    {
        "graph": GRAPH,
        "type_id": "Game|GetPlayerController",
        "pos": {"x": 7200, "y": 480},
    },
    sid,
)
print("pc", pc)
pc_node = pc["returnValue"]["refPath"]

lmb = call_tool(
    BT,
    "create_node",
    {
        "graph": GRAPH,
        "type_id": 'Input|GetInputKey("LeftMouseButton")',
        "pos": {"x": 7440, "y": 560},
    },
    sid,
)
print("lmb key", lmb)

rel = call_tool(
    BT,
    "create_node",
    {
        "graph": GRAPH,
        "type_id": "Game|Player|WasInputKeyJustReleased",
        "pos": {"x": 7680, "y": 480},
    },
    sid,
)
print("rel", rel)
rel_node = rel["returnValue"]["refPath"]

# Connect PlayerController -> WasReleased target (input 0)
call_tool(
    BT,
    "connect_pins",
    {
        "output_pin": {
            "direction": "EGPD_Output",
            "index_id": 0,
            "node": {"refPath": pc_node},
        },
        "input_pin": {
            "direction": "EGPD_Input",
            "index_id": 0,
            "node": {"refPath": rel_node},
        },
    },
    sid,
)

# Key pin - get pin infos for rel node
infos = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": rel_node}]}, sid)
rel_info = infos["returnValue"][0]
print("rel pins", [p["name"] for p in rel_info["input_pins"]])
key_node = lmb.get("returnValue", {}).get("refPath", "")
if key_node:
    for pin in rel_info["input_pins"]:
        if "Key" in pin["name"]:
            call_tool(
                BT,
                "connect_pins",
                {
                    "output_pin": {
                        "direction": "EGPD_Output",
                        "index_id": 0,
                        "node": {"refPath": key_node},
                    },
                    "input_pin": pin["pin_id"],
                },
                sid,
            )
            break

# WasReleased -> AND A
call_tool(
    BT,
    "connect_pins",
    {
        "output_pin": {
            "direction": "EGPD_Output",
            "index_id": 0,
            "node": {"refPath": rel_node},
        },
        "input_pin": {
            "direction": "EGPD_Input",
            "index_id": 0,
            "node": {
                "refPath": f"{GRID}:UpdateDrag.K2Node_CommutativeAssociativeBinaryOperator_13"
            },
        },
    },
    sid,
)

call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
print("DONE drop branch uses LMB release")