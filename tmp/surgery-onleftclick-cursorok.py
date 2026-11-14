import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:OnLeftClick"}

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

gets = call_tool(BT, "find_nodes", {"graph": GRAPH, "title": "Get CursorOk"}, sid).get(
    "returnValue", []
)
for g in gets:
    gi = call_tool(BT, "get_node_infos", {"nodes": [g]}, sid)["returnValue"][0]
    for op in gi["output_pins"]:
        for cp in op.get("connected_pins", []):
            call_tool(
                BT,
                "break_pins",
                {"output_pin": op["pin_id"], "input_pin": cp},
                sid,
            )
            print("broke CursorOk output")

# If top branch is 3-input AND, connect LMB AND Hovered directly — break CursorOk leg only done above
# Ensure TryDig still connected — find TryDig call
trys = call_tool(BT, "find_nodes", {"graph": GRAPH, "title": "TryDig"}, sid).get(
    "returnValue", []
)
print("TryDig nodes", trys)

call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
print("OnLeftClick: CursorOk gate removed")