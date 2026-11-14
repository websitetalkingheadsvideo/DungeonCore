import json
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:TickStore"}
sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()

for title in [
    "Get Pressed Item",
    "Update Drag",
    "Get Hovered Item",
    "Get HUD",
    "Is Input Key Down",
]:
    nodes = call_tool(BT, "find_nodes", {"graph": GRAPH, "title": title}, sid).get(
        "returnValue", []
    )
    print(title, len(nodes), [n["refPath"][-20:] for n in nodes[:5]])