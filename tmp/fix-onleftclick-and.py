"""OnLeftClick: branch on LMB+not-store only (fix dead AND after CursorOk removal)."""
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
GRAPH = {"refPath": f"{GRID}:OnLeftClick"}

OUTER = f"{GRID}:OnLeftClick.K2Node_CommutativeAssociativeBinaryOperator_10"
BRANCH = f"{GRID}:OnLeftClick.K2Node_IfThenElse_11"

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

oinfo = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": OUTER}]}, sid)["returnValue"][0]
inner_and_out = oinfo["input_pins"][0]["connected_pins"][0]
inner_and = inner_and_out["node"]["refPath"]

# Branch condition <- inner AND (skip outer AND that required CursorOk)
call_tool(
    BT,
    "break_pins",
    {
        "output_pin": oinfo["output_pins"][0]["pin_id"],
        "input_pin": {
            "direction": "EGPD_Input",
            "index_id": 1,
            "node": {"refPath": BRANCH},
        },
    },
    sid,
)
iinfo = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": inner_and}]}, sid)[
    "returnValue"
][0]
call_tool(
    BT,
    "connect_pins",
    {
        "output_pin": iinfo["output_pins"][0]["pin_id"],
        "input_pin": {
            "direction": "EGPD_Input",
            "index_id": 1,
            "node": {"refPath": BRANCH},
        },
    },
    sid,
)

call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
print("OnLeftClick: dig on LMB when not hovering store")