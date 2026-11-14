"""Hold-drag-release via TickStore only (no UpdateDrag rewrite)."""
from __future__ import annotations

import json
import subprocess
import sys
import urllib.request

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool, new_session

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
UMG = "UMGToolSet.UMGToolSet"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"

TICK_STORE = r"""
(fn TickStore ()
  (bind _hud (Variables|Default|GetHUD))
  (bind _press_raw (Class|WBPDungeonHUD|GetPressedItem _hud))
  (bind _pc (Game|GetPlayerController 0))
  (bind _pending (Variables|Default|GetPendingPress))
  (bind _store_next (select (>= _press_raw 0) _press_raw _pending))
  (Variables|Default|SetPendingPress _store_next)
  (bind _sel (Variables|Default|GetSelectedItem))
  (bind _lmb (Game|Player|IsInputKeyDown _pc "LeftMouseButton"))
  (bind _lmb_up (Game|Player|WasInputKeyJustReleased _pc "LeftMouseButton"))
  (bind _index (select (and (>= _sel 0) _lmb) _sel (select (and (>= _store_next 0) _lmb) _store_next _press_raw)))
  (bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))
  (Variables|Default|SetHoveredItem _index_1)
  (Variables|Default|SetMsgTimer (- (Variables|Default|GetMsgTimer) (Utilities|Time|GetWorldDeltaSeconds)))
  (Class|WBPDungeonHUD|Highlight _hud (Variables|Default|GetHoveredItem) (Variables|Default|GetSelectedItem))
  (Class|WBPDungeonHUD|SetStatus _hud (select (> (Variables|Default|GetMsgTimer) 0.0) (Variables|Default|GetStatusMsg) (select (>= (Variables|Default|GetSelectedItem) 0) (Utilities|String|Append "Placing: " (Utilities|String|Append (Utilities|Array|Get(acopy) (Variables|Default|GetItemNames) (Math|Integer|Max(Integer) (Variables|Default|GetSelectedItem))) " - release on a room square")) "Hold a store item, drag to a room, release")))
  (if (and _lmb_up (>= _sel 0))
    (Transformation|SetRelativeLocation (Variables|Default|GetGhost) (Math|Vector|MakeVector 0.0 0.0 -5000.0))
    (CallFunction|UpdateCursorCell)
    (if (Variables|Default|GetCursorOk)
      (CallFunction|TryPlace (Variables|Default|GetCursorX) (Variables|Default|GetCursorY))
      (Variables|Default|SetSelectedItem -1)
      (Variables|Default|SetPendingPress -1)
      (else
        (Variables|Default|SetSelectedItem -1)
        (Variables|Default|SetPendingPress -1)))
    (else
      (CallFunction|UpdateDrag _index)))
""".strip()


def main() -> None:
    sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
    try:
        pie = call_tool(APP, "IsPIERunning", {}, sid)
    except Exception:
        sid = new_session()
        pie = call_tool(APP, "IsPIERunning", {}, sid)
    if pie.get("returnValue"):
        call_tool(APP, "StopPIE", {}, sid)

    names = call_tool(BT, "list_variables", {"blueprint": {"refPath": GRID}}, sid).get(
        "returnValue", []
    )
    if "PendingPress" not in names:
        call_tool(
            BT,
            "add_variable",
            {"blueprint": {"refPath": GRID}, "name": "PendingPress", "type_name": "int"},
            sid,
        )

    w = call_tool(
        BT,
        "write_graph_dsl",
        {"graph": {"refPath": f"{GRID}:TickStore"}, "code": TICK_STORE},
        sid,
    )
    print("write:", w)
    verify = call_tool(
        BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}, sid
    )["returnValue"]
    if "PendingPress" not in verify or "WasInputKeyJustReleased" not in verify:
        raise RuntimeError(verify)

    subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-hud-widget-bindings.py"])
    subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-get-pressed-item-latch.py"])

    call_tool(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}}, sid)
    call_tool(AT, "save_assets", {"asset_paths": [GRID, HUD]}, sid)
    print("OK — Play: hold store row, drag, release on room.")


if __name__ == "__main__":
    main()