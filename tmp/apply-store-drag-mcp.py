"""Stop PIE if needed, apply hold-drag-release via MCP (no excuses)."""
from __future__ import annotations

import json
import subprocess
import sys
import urllib.request

MCP_URL = "http://127.0.0.1:8000/mcp"
SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
UMG = "UMGToolSet.UMGToolSet"
APP = "EditorToolset.EditorAppToolset"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

RELEASE_OLD = "(elif (and (< Pressed 0) (>= _selecteditem 0))"
RELEASE_NEW = '(elif (and (>= _selecteditem 0) (Game|Player|WasInputKeyJustReleased (Game|GetPlayerController 0) "LeftMouseButton"))'

TICK_HEAD_OLD = """(fn TickStore ()
  (bind _hud (Variables|Default|GetHUD))
  (bind _index (Class|WBPDungeonHUD|GetPressedItem _hud))
  (bind _index_1 (Variables|Default|GetHoveredItem _hud))"""

TICK_HEAD_NEW = """(fn TickStore ()
  (bind _hud (Variables|Default|GetHUD))
  (bind _press_raw (Class|WBPDungeonHUD|GetPressedItem _hud))
  (bind _pc (Game|GetPlayerController 0))
  (bind _pending (Variables|Default|GetPendingPress))
  (bind _store_next (select (>= _press_raw 0) _press_raw _pending))
  (Variables|Default|SetPendingPress _store_next)
  (bind _index (select (and (>= _store_next 0) (Game|Player|IsInputKeyDown _pc "LeftMouseButton")) _store_next _press_raw))
  (bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))"""


def ensure_session() -> str:
    try:
        return open(SID_PATH, encoding="utf-8").read().strip()
    except OSError:
        pass
    subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\mcp_session.py"])
    return open(SID_PATH, encoding="utf-8").read().strip()


def call(ts: str, tool: str, args: dict) -> dict | str:
    sid = ensure_session()
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": ts, "tool_name": tool, "arguments": args},
            },
        }
    ).encode()
    try:
        raw = urllib.request.urlopen(
            urllib.request.Request(
                MCP_URL,
                data=body,
                headers={
                    "Content-Type": "application/json",
                    "Accept": "application/json, text/event-stream",
                    "Mcp-Session-Id": sid,
                },
            ),
            timeout=300,
        ).read()
    except urllib.error.HTTPError as exc:
        if exc.code == 404:
            subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\mcp_session.py"])
            return call(ts, tool, args)
        raise
    text = json.loads(raw)["result"]["content"][0]["text"]
    if raw_json := text.strip().startswith("{"):
        try:
            return json.loads(text)
        except json.JSONDecodeError:
            pass
    if any(
        x in text
        for x in (
            "RuntimeError",
            "AssertionError",
            "does not exist",
            "Could not connect",
            "Unclosed",
        )
    ):
        raise RuntimeError(text[:3000])
    return text


def main() -> None:
    pie = call(APP, "IsPIERunning", {})
    if pie.get("returnValue"):
        print("Stopping PIE...")
        call(APP, "StopPIE", {})

    vars_out = call(BT, "list_variables", {"blueprint": {"refPath": GRID}})
    if "PendingPress" not in vars_out.get("returnValue", []):
        call(BT, "add_variable", {"blueprint": {"refPath": GRID}, "name": "PendingPress", "type_name": "int"})

    ud = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}})["returnValue"]
    if RELEASE_OLD in ud:
        call(BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}, "code": ud.replace(RELEASE_OLD, RELEASE_NEW, 1)})
    ud2 = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}})["returnValue"]
    if "WasInputKeyJustReleased" not in ud2:
        raise RuntimeError(f"UpdateDrag patch failed:\n{ud2}")

    tick = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}})["returnValue"]
    if "PendingPress" not in tick:
        if TICK_HEAD_OLD not in tick:
            raise RuntimeError(f"TickStore head mismatch:\n{tick[:800]}")
        tick2 = tick.replace(TICK_HEAD_OLD, TICK_HEAD_NEW, 1)
        tick2 = tick2.replace(
            '" - drop it on a room square")) "Drag an item onto a room square")))',
            '" - release on a room square")) "Hold a store item, drag to a room, release")))',
            1,
        )
        w = call(BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": tick2})
        if isinstance(w, dict):
            print("TickStore write ok")
        else:
            print("TickStore write:", str(w)[:400])

    tick3 = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}})["returnValue"]
    if "PendingPress" not in tick3 or "GetHoveredItem _hud" not in tick3:
        raise RuntimeError(f"TickStore verify failed:\n{tick3}")

    subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-hud-widget-bindings.py"])
    subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-get-pressed-item-latch.py"], stderr=subprocess.DEVNULL)

    call(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}})
    call(AT, "save_assets", {"asset_paths": [GRID, HUD]})
    print("DONE — Play: hold store row, drag, release LMB on room.")


if __name__ == "__main__":
    main()