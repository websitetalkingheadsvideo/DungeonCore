import json
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:TryDig"}
sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()

branches = call_tool(BT, "find_nodes", {"graph": GRAPH, "title": "Branch"}, sid)[
    "returnValue"
]
print("branches", len(branches))
infos = call_tool(BT, "get_node_infos", {"nodes": branches}, sid)["returnValue"]
open(r"e:\Dungeon Core\tmp\trydig-branches.json", "w").write(json.dumps(infos, indent=2))

for b in infos:
    cond = b["input_pins"][1] if len(b["input_pins"]) > 1 else {}
    conn = cond.get("connected_pins", [])
    print(b["node"]["refPath"].split("_")[-1], "else connected", len(b["output_pins"][1].get("connected_pins", [])))