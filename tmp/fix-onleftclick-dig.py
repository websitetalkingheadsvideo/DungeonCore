import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

CODE = """(fn OnLeftClick ()
  (bind _pc (Game|GetPlayerController 0))
  (if (and (Game|Player|WasInputKeyJustPressed _pc "LeftMouseButton") (< (Variables|Default|GetHoveredItem) 0))
    (CallFunction|TryDig (Variables|Default|GetCursorX) (Variables|Default|GetCursorY))))"""

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

print(call_tool(BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:OnLeftClick"}, "code": CODE}, sid))
v = call_tool(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:OnLeftClick"}}, sid)["returnValue"]
print(v)
call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)