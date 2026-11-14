"""Apply balanced TickStore for store hold-drag-release."""
from __future__ import annotations

import subprocess
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
UMG = "UMGToolSet.UMGToolSet"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
TICK_STORE = open(r"e:\Dungeon Core\tmp\tickstore-drag-fixed.dsl", encoding="utf-8").read().strip()


def main() -> None:
    if call_tool(APP, "IsPIERunning", {}, SID).get("returnValue"):
        call_tool(APP, "StopPIE", {}, SID)

    assert TICK_STORE.count("(") == TICK_STORE.count(")")
    w = call_tool(
        BT,
        "write_graph_dsl",
        {"graph": {"refPath": f"{GRID}:TickStore"}, "code": TICK_STORE},
        SID,
    )
    print("TickStore write:", w)

    verify = call_tool(
        BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}, SID
    )["returnValue"]
    for needle in ("PendingPress", "GetHoveredItem _hud", ":Pressed _pressed"):
        if needle not in verify.replace("\n", " "):
            raise RuntimeError(f"missing {needle}:\n{verify}")

    subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-hud-widget-bindings.py"])
    subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-get-pressed-item-latch.py"])

    call_tool(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}}, SID)
    call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, SID)
    call_tool(AT, "save_assets", {"asset_paths": [HUD, GRID]}, SID)
    print("OK — hold store row, drag to room, release LMB.")


if __name__ == "__main__":
    main()