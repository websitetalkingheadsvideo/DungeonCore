import json
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()
nodes = [
    {"refPath": f"{GRID}:UpdateDrag.K2Node_CommutativeAssociativeBinaryOperator_13"},
    {"refPath": f"{GRID}:UpdateDrag.K2Node_PromotableOperator_19"},
    {"refPath": f"{GRID}:UpdateDrag.K2Node_PromotableOperator_21"},
    {"refPath": f"{GRID}:UpdateDrag.K2Node_PromotableOperator_23"},
]
out = call_tool(BT, "get_node_infos", {"nodes": nodes}, sid)
open(r"e:\Dungeon Core\tmp\updatedrag-ops.json", "w").write(json.dumps(out, indent=2))
print("written")