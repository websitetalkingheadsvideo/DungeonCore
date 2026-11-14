import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
APP = "EditorToolset.EditorAppToolset"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

tick = call_tool(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}, sid)[
    "returnValue"
]
old = "(bind _index_1 (Variables|Default|GetHoveredItem _hud))"
new = "(bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))"
if old not in tick:
    print("already patched or unexpected graph")
    print(tick[:500])
else:
    pat = tick.replace(old, new, 1)
    w = call_tool(
        BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": pat}, sid
    )
    print("write", w)
v = call_tool(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}, sid)[
    "returnValue"
]
print("hover ok", "Class|WBPDungeonHUD|GetHoveredItem" in v)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)