"""Fix dead store drag: LMB release + key/hud index in TickStore only."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

RELEASE_OLD = "(elif (and (< Pressed 0) (>= _selecteditem 0))"
RELEASE_NEW = """(bind _pc_ud (Game|GetPlayerController 0))
  (elif (and (Game|Player|WasInputKeyJustReleased _pc_ud "LeftMouseButton") (>= _selecteditem 0))"""

TICK_HEAD = """(fn TickStore ()
  (bind _hud (Variables|Default|GetHUD))
  (bind _pc_ts (Game|GetPlayerController 0))
  (bind _keypick -1)
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "One") (bind _keypick 0))
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "Two") (bind _keypick 1))
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "Three") (bind _keypick 2))
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "Four") (bind _keypick 3))
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "Five") (bind _keypick 4))
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "Six") (bind _keypick 5))
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "Seven") (bind _keypick 6))
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "Eight") (bind _keypick 7))
  (if (Game|Player|IsInputKeyJustPressed _pc_ts "Nine") (bind _keypick 8))
  (bind _index (Class|WBPDungeonHUD|GetPressedItem _hud))
  (bind _index (select (>= _keypick 0) _keypick _index))"""

TICK_HEAD_OLD = """(fn TickStore ()
  (bind _hud (Variables|Default|GetHUD))
  (bind _index (Class|WBPDungeonHUD|GetPressedItem _hud))"""


def call(tool: str, args: dict) -> dict | str:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": AT if tool in ("save_assets",) else BT, "tool_name": tool, "arguments": args},
            },
        }
    ).encode()
    text = json.loads(
        urllib.request.urlopen(
            urllib.request.Request(
                "http://127.0.0.1:8000/mcp",
                data=body,
                headers={
                    "Content-Type": "application/json",
                    "Accept": "application/json, text/event-stream",
                    "Mcp-Session-Id": SID,
                },
            ),
            timeout=300,
        ).read()
    )["result"]["content"][0]["text"]
    if text.startswith("RuntimeError") or text.startswith("AssertionError"):
        raise RuntimeError(text[:2000])
    return json.loads(text) if text.strip().startswith("{") else text


def main() -> None:
    ud = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}})["returnValue"]
    if "WasInputKeyJustReleased" in ud and "UpdateDrag" in ud:
        print("UpdateDrag: LMB release already set")
    else:
        if RELEASE_OLD not in ud:
            raise RuntimeError(f"missing release pattern:\n{ud}")
        ud2 = ud.replace(RELEASE_OLD, RELEASE_NEW, 1)
        call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}, "code": ud2})

    tick = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}})["returnValue"]
    if "_keypick" in tick:
        print("TickStore: keys already patched")
    else:
        if TICK_HEAD_OLD not in tick:
            raise RuntimeError(tick)
        tick2 = tick.replace(TICK_HEAD_OLD, TICK_HEAD, 1)
        # Fix wrong GetHoveredItem on grid variable
        tick2 = tick2.replace(
            "(bind _index_1 (Variables|Default|GetHoveredItem _hud))",
            "(bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))",
            1,
        )
        call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": tick2})

    call("save_assets", {"asset_paths": ["/Game/Core/BP_DungeonGrid"]})
    print("OK: Press 1-9 OR hold store row, drag with LMB, release LMB to place.")


if __name__ == "__main__":
    main()