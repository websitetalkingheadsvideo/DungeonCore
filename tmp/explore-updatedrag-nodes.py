import json
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()

for title in ["Pressed", "Less", "Branch", "TryPlace", "UpdateDrag"]:
    args = {
        "graph": {"refPath": f"{GRID}:UpdateDrag"},
        "title_contains": title,
        "max_results": 20,
    }
    try:
        out = call_tool(BT, "find_nodes", args, sid)
        nodes = out.get("returnValue", out)
        print("=== title", title, "===")
        print(json.dumps(nodes, indent=2)[:3000])
    except Exception as exc:
        print(title, exc)