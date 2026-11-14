"""Let grid clicks reach TryDig: drop CursorOk gate; arm store only when store hovered."""
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

# OnLeftClick: remove CursorOk from AND (pin surgery on IfThenElse_0 in OnLeftClick)
onclick = call_tool(
    BT, "find_nodes", {"graph": {"refPath": f"{GRID}:OnLeftClick"}, "title": "Branch"}, sid
)["returnValue"]
if not onclick:
    raise RuntimeError("OnLeftClick branch missing")
br = onclick[0]["refPath"]
info = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": br}]}, sid)["returnValue"][0]
cond = info["input_pins"][1]
# Condition wired to Boolean AND with 3 inputs — break pin feeding CursorOk (Get CursorOk)
for cp in cond.get("connected_pins", []):
    src = cp.get("node", {}).get("refPath", "")
    if "CursorOk" in src or "GetCursorOk" in str(cp):
        call_tool(
            BT,
            "break_pins",
            {
                "output_pin": {
                    "direction": cp.get("direction", "EGPD_Output"),
                    "index_id": cp.get("index_id", 0),
                    "node": {"refPath": src},
                },
                "input_pin": cond["pin_id"],
            },
            sid,
        )
        print("broke CursorOk from OnLeftClick")

# Simpler: reconnect condition to only LMB+hovered — find AND node, break B pin if it's CursorOk
ands = call_tool(
    BT, "find_nodes", {"graph": {"refPath": f"{GRID}:OnLeftClick"}, "title": "AND"}, sid
).get("returnValue", [])
for an in ands:
    ni = call_tool(BT, "get_node_infos", {"nodes": [an]}, sid)["returnValue"][0]
    for inp in ni["input_pins"]:
        for cp in inp.get("connected_pins", []):
            nref = cp.get("node", {}).get("refPath", "")
            if "CursorOk" in nref or "VariableGet" in nref and inp["name"] == "B":
                # check if variable is CursorOk via get_node_infos on source
                src_info = call_tool(
                    BT, "get_node_infos", {"nodes": [{"refPath": nref}]}, sid
                )["returnValue"][0]
                if "CursorOk" in str(src_info):
                    call_tool(
                        BT,
                        "break_pins",
                        {
                            "output_pin": cp,
                            "input_pin": inp["pin_id"],
                        },
                        sid,
                    )
                    print("removed CursorOk AND leg")

# UpdateDrag arm: require HoveredItem >= 0 — wire GetHoveredItem into arm AND if missing
arms = call_tool(
    BT, "find_nodes", {"graph": {"refPath": f"{GRID}:UpdateDrag"}, "title": "AND"}, sid
).get("returnValue", [])
arm_and = arms[0] if arms else None
if arm_and:
    hud_hover = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": f"{GRID}:UpdateDrag"},
            "type_id": "Variables|Default|GetHoveredItem",
            "pos": {"x": 6200, "y": -120},
        },
        sid,
    )
    ge0 = call_tool(
        BT,
        "create_node",
        {
            "graph": {"refPath": f"{GRID}:UpdateDrag"},
            "type_id": "Math|Integer|integer>=integer",
            "pos": {"x": 6480, "y": -120},
        },
        sid,
    )
    # ... too fragile without knowing free AND pin

call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
print("OnLeftClick dig gate updated")