import json
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()
for title in [
    "Get Player Controller",
    "Was Input Key Just Released",
    "Left Mouse Button",
]:
    out = call_tool(
        BT, "find_nodes", {"graph": {"refPath": f"{GRID}:UpdateDrag"}, "title": title}, sid
    )
    print(title, json.dumps(out.get("returnValue", []), indent=2))