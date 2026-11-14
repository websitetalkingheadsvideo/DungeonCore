"""Only arm store when cursor is over a store row (HoveredItem >= 0)."""
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

# Arm branch = IfThenElse_5 condition — AND with Pressed>=0 and selected<0; add HoveredItem>=0
ands = call_tool(BT, "find_nodes", {"graph": GRAPH, "title": "AND"}, sid)["returnValue"]
# first AND at x~7280 is arm (from prior read)
arm_and = None
for an in ands:
    pos = call_tool(BT, "get_node_infos", {"nodes": [an]}, sid)["returnValue"][0]["position"]["x"]
    if 7000 <= pos <= 7500:
        arm_and = an["refPath"]
        break
if not arm_and:
    raise RuntimeError("arm AND not found")

ainfos = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": arm_and}]}, sid)[
    "returnValue"
][0]
# need 3-input AND or extra AND — check if already has hovered
for inp in ainfos["input_pins"]:
    for c in inp.get("connected_pins", []):
        src = c["node"]["refPath"]
        if "HoveredItem" in str(
            call_tool(BT, "get_node_infos", {"nodes": [{"refPath": src}]}, sid)
        ):
            print("already has hovered")
            break
else:
    hover_get = call_tool(
        BT,
        "create_node",
        {
            "graph": GRAPH,
            "type_id": "Variables|Default|GetHoveredItem",
            "pos": {"x": 7000, "y": -80},
        },
        sid,
    )["returnValue"]["refPath"]
    cmp_node = call_tool(
        BT,
        "create_node",
        {
            "graph": GRAPH,
            "type_id": "Math|Integer|integer>=integer",
            "pos": {"x": 7200, "y": -80},
        },
        sid,
    )["returnValue"]["refPath"]
    cmp_i = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": cmp_node}]}, sid)[
        "returnValue"
    ][0]
    call_tool(
        BT,
        "set_pin_value",
        {"pin": cmp_i["input_pins"][1]["pin_id"], "value": "0"},
        sid,
    )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": call_tool(
                BT, "get_node_infos", {"nodes": [{"refPath": hover_get}]}, sid
            )["returnValue"][0]["output_pins"][0]["pin_id"],
            "input_pin": cmp_i["input_pins"][0]["pin_id"],
        },
        sid,
    )
    # Nest: new AND( old_arm_and, hovered>=0 ) -> IfThenElse_5
    new_and = call_tool(
        BT,
        "create_node",
        {
            "graph": GRAPH,
            "type_id": "Math|Boolean|ANDBoolean",
            "pos": {"x": 7360, "y": -40},
        },
        sid,
    )["returnValue"]["refPath"]
    ninfo = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": new_and}]}, sid)[
        "returnValue"
    ][0]
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": ainfos["output_pins"][0]["pin_id"],
            "input_pin": ninfo["input_pins"][0]["pin_id"],
        },
        sid,
    )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": cmp_i["output_pins"][0]["pin_id"],
            "input_pin": ninfo["input_pins"][1]["pin_id"],
        },
        sid,
    )
    call_tool(
        BT,
        "break_pins",
        {
            "output_pin": ainfos["output_pins"][0]["pin_id"],
            "input_pin": {
                "direction": "EGPD_Input",
                "index_id": 1,
                "node": {"refPath": f"{GRID}:UpdateDrag.K2Node_IfThenElse_5"},
            },
        },
        sid,
    )
    call_tool(
        BT,
        "connect_pins",
        {
            "output_pin": ninfo["output_pins"][0]["pin_id"],
            "input_pin": {
                "direction": "EGPD_Input",
                "index_id": 1,
                "node": {"refPath": f"{GRID}:UpdateDrag.K2Node_IfThenElse_5"},
            },
        },
        sid,
    )
    print("arm store only when HoveredItem >= 0")

call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)