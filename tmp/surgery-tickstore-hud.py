import json
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
UMG = "UMGToolSet.UMGToolSet"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:TickStore"}

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

# Fix wrong GetHoveredItem on grid self -> HUD GetHoveredItem
hover_calls = call_tool(
    BT, "find_nodes", {"graph": GRAPH, "title": "Get Hovered Item"}, sid
).get("returnValue", [])
print("hover nodes", len(hover_calls))
for n in hover_calls:
    infos = call_tool(BT, "get_node_infos", {"nodes": [n]}, sid)["returnValue"][0]
    print(json.dumps(infos.get("type_id"), infos.get("input_pins", [])[:2]), infos["node"]["refPath"])

# PendingPress + DragFeed vars for future latch (add if missing)
for vn in ("PendingPress",):
    names = call_tool(BT, "list_variables", {"blueprint": {"refPath": GRID}}, sid).get(
        "returnValue", []
    )
    if vn not in names:
        call_tool(
            BT,
            "add_variable",
            {"blueprint": {"refPath": GRID}, "name": vn, "type_name": "int"},
            sid,
        )

import subprocess

subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-hud-widget-bindings.py"])
try:
    subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-get-pressed-item-latch.py"])
except Exception as exc:
    print("latch", exc)

call_tool(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}}, sid)
call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID, HUD]}, sid)
print("HUD bindings + grid compile saved")