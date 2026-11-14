import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

code = open(r"e:\Dungeon Core\tmp\tickstore-drag.dsl", encoding="utf-8").read().strip()
sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
pie = call_tool(APP, "IsPIERunning", {}, sid)
if pie.get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)
names = call_tool(BT, "list_variables", {"blueprint": {"refPath": GRID}}, sid).get(
    "returnValue", []
)
for vn in ("PendingPress", "DragFeed"):
    if vn not in names:
        call_tool(
            BT,
            "add_variable",
            {"blueprint": {"refPath": GRID}, "name": vn, "type_name": "int"},
            sid,
        )
w = call_tool(
    BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": code}, sid
)
print("write", w)
v = call_tool(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}, sid)[
    "returnValue"
]
print("verify GetDragFeed" in v, len(v))
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)