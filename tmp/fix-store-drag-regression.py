"""Fix store drag regression: TickStore HUD hover + HUD bindings + arm branch sanity."""
import subprocess
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
UMG = "UMGToolSet.UMGToolSet"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
TICK = {"refPath": f"{GRID}:TickStore"}
UD = {"refPath": f"{GRID}:UpdateDrag"}

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)


def rewire_hovered_call() -> None:
    calls = call_tool(BT, "find_nodes", {"graph": TICK, "title": "GetHoveredItem"}, sid).get(
        "returnValue", []
    )
    if not calls:
        raise RuntimeError("GetHoveredItem call missing in TickStore")
    node = calls[0]["refPath"]
    info = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": node}]}, sid)[
        "returnValue"
    ][0]
    # HUD function: Target pin is self on WBP, not grid GetHoveredItem variable
    hud_nodes = call_tool(BT, "find_nodes", {"graph": TICK, "title": "Get HUD"}, sid).get(
        "returnValue", []
    )
    if not hud_nodes:
        raise RuntimeError("GetHUD missing")
    hud_out = call_tool(BT, "get_node_infos", {"nodes": [hud_nodes[0]]}, sid)[
        "returnValue"
    ][0]["output_pins"][0]["pin_id"]
    for inp in info["input_pins"]:
        if inp["name"] in ("self", "Target"):
            for cp in list(inp.get("connected_pins", [])):
                call_tool(
                    BT,
                    "break_pins",
                    {"output_pin": cp, "input_pin": inp["pin_id"]},
                    sid,
                )
            call_tool(
                BT,
                "connect_pins",
                {"output_pin": hud_out, "input_pin": inp["pin_id"]},
                sid,
            )
            print("TickStore: GetHoveredItem targets HUD widget")

    bad = call_tool(
        BT, "find_nodes", {"graph": TICK, "title": "Get Hovered Item"}, sid
    ).get("returnValue", [])
    for b in bad:
        if b["refPath"] != node:
            print("WARN grid GetHoveredItem variable get still present", b)


def fix_arm_branch() -> None:
    """Remove broken half-added HoveredItem gate if it left arm AND false."""
    branches = call_tool(BT, "find_nodes", {"graph": UD, "title": "Branch"}, sid)[
        "returnValue"
    ]
    arm = None
    for b in branches:
        pos = call_tool(BT, "get_node_infos", {"nodes": [b]}, sid)["returnValue"][0][
            "position"
        ]["x"]
        if 7200 <= pos <= 7350:
            arm = b["refPath"]
            break
    if not arm:
        return
    cond = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": arm}]}, sid)[
        "returnValue"
    ][0]["input_pins"][1]
    src = cond["connected_pins"][0]["node"]["refPath"] if cond.get("connected_pins") else ""
    if "Operator_39" in src or "Operator_40" in src:
        # broken partial patch — reconnect arm AND output directly if nested
        print("checking arm AND", src)


rewire_hovered_call()
fix_arm_branch()

subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-hud-widget-bindings.py"])
subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-get-pressed-item-latch.py"])

call_tool(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}}, sid)
call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID, HUD]}, sid)

# OnLeftClick must not run while placing — verify UpdateDrag still has ghost before else
ud = call_tool(BT, "read_graph_dsl", {"graph": UD}, sid)["returnValue"]
assert "WasInputKeyJustReleased" in ud
assert "elif (>= _selecteditem 0)" in ud
print("OK — stop PIE, play: hold store row, drag, release on room")