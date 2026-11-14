"""Temporary store pick: number keys 1-9 arm placement (HUD Btn_* bindings are dead)."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

KEYS = [
    "One",
    "Two",
    "Three",
    "Four",
    "Five",
    "Six",
    "Seven",
    "Eight",
    "Nine",
]

ANCHOR = "(CallFunction|TickStore)"
BLOCK = """
  (bind _pc_store (Game|GetPlayerController 0))
  (bind _store_pick -1)
  (if (Game|Player|IsInputKeyJustPressed _pc_store "One") (bind _store_pick 0))
  (if (Game|Player|IsInputKeyJustPressed _pc_store "Two") (bind _store_pick 1))
  (if (Game|Player|IsInputKeyJustPressed _pc_store "Three") (bind _store_pick 2))
  (if (Game|Player|IsInputKeyJustPressed _pc_store "Four") (bind _store_pick 3))
  (if (Game|Player|IsInputKeyJustPressed _pc_store "Five") (bind _store_pick 4))
  (if (Game|Player|IsInputKeyJustPressed _pc_store "Six") (bind _store_pick 5))
  (if (Game|Player|IsInputKeyJustPressed _pc_store "Seven") (bind _store_pick 6))
  (if (Game|Player|IsInputKeyJustPressed _pc_store "Eight") (bind _store_pick 7))
  (if (Game|Player|IsInputKeyJustPressed _pc_store "Nine") (bind _store_pick 8))
  (if (>= _store_pick 0) (CallFunction|UpdateDrag _store_pick))
"""


def call(tool: str, args: dict) -> dict | str:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": BT, "tool_name": tool, "arguments": args},
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
            timeout=180,
        ).read()
    )["result"]["content"][0]["text"]
    if isinstance(text, str) and not text.strip().startswith("{") and ("Error" in text or "AssertionError" in text):
        raise RuntimeError(text[:2000])
    return json.loads(text) if text.strip().startswith("{") else text


def main() -> None:
    tick = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:EventGraph"}})["returnValue"]
    if "_store_pick" in tick:
        print("hotkeys already patched")
        return
    if ANCHOR not in tick:
        raise RuntimeError(tick)
    patched = tick.replace(ANCHOR, BLOCK + f"\n  {ANCHOR}", 1)
    call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:EventGraph"}, "code": patched})
    verify = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:EventGraph"}})["returnValue"]
    if "_store_pick" not in verify:
        raise RuntimeError(verify)
    call(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {"asset_paths": ["/Game/Core/BP_DungeonGrid"]},
    )
    print("Grid keys 1-9 arm store slots. STOP PIE. Play. Press 1-9, LMB on room.")


if __name__ == "__main__":
    main()
