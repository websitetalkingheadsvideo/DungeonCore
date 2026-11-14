"""Pin surgery: UpdateDrag drop on LMB release, not Pressed < 0."""
import json
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

graph = {"refPath": f"{GRID}:UpdateDrag"}
branches = call_tool(BT, "find_nodes", {"graph": graph, "title": "Branch"}, sid)
nodes = branches.get("returnValue", branches)
print("branches", len(nodes))
open(r"e:\Dungeon Core\tmp\updatedrag-branches.json", "w").write(json.dumps(nodes, indent=2))

infos = call_tool(
    BT,
    "get_node_infos",
    {"nodes": nodes[:15] if isinstance(nodes, list) else nodes},
    sid,
)
open(r"e:\Dungeon Core\tmp\updatedrag-node-infos.json", "w").write(
    json.dumps(infos, indent=2)[:80000]
)
print("wrote node infos")
call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)