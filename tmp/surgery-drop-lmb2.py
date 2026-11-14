import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:UpdateDrag"}
AND = f"{GRID}:UpdateDrag.K2Node_CommutativeAssociativeBinaryOperator_13"

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

# Use latest GetPlayerController node
pcs = call_tool(BT, "find_nodes", {"graph": GRAPH, "title": "Get Player Controller"}, sid)[
    "returnValue"
]
pc_node = pcs[-1]["refPath"] if pcs else None
if not pc_node:
    pc = call_tool(
        BT,
        "create_node",
        {"graph": GRAPH, "type_id": "Game|GetPlayerController", "pos": {"x": 7200, "y": 480}},
        sid,
    )
    pc_node = pc["returnValue"]["refPath"]

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
rel_node = rel["returnValue"]["refPath"]

infos = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": rel_node}]}, sid)
rel_info = infos["returnValue"][0]
key_pin = next(p for p in rel_info["input_pins"] if p["name"] == "Key")
call_tool(BT, "set_pin_value", {"pin": key_pin["pin_id"], "value": "LeftMouseButton"}, sid)

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
            "node": {"refPath": AND},
        },
    },
    sid,
)

call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
print("OK: drop on LMB release wired")