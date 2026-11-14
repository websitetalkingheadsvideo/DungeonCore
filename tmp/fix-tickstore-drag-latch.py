"""Latch store drag index while LMB held (fixes IsPressed dropping off button)."""
from __future__ import annotations

import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

TICK_STORE_DSL = """
(fn TickStore ()
  (bind _hud (Variables|Default|GetHUD))
  (bind _index (Class|WBPDungeonHUD|GetPressedItem _hud))
  (bind _pc (Game|GetPlayerController 0))
  (bind _sel (Variables|Default|GetSelectedItem))
  (bind _drag_index (select (and (>= _sel 0) (Game|Player|IsInputKeyDown _pc "LeftMouseButton")) _sel _index))
  (bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))
  (Variables|Default|SetHoveredItem _index_1)
  (Variables|Default|SetMsgTimer (- (Variables|Default|GetMsgTimer) (Utilities|Time|GetWorldDeltaSeconds)))
  (Class|WBPDungeonHUD|Highlight _hud (Variables|Default|GetHoveredItem) (Variables|Default|GetSelectedItem))
  (Class|WBPDungeonHUD|SetStatus _hud (select (> (Variables|Default|GetMsgTimer) 0.0) (Variables|Default|GetStatusMsg) (select (>= (Variables|Default|GetSelectedItem) 0) (Utilities|String|Append "Placing: " (Utilities|String|Append (Utilities|Array|Get(acopy) (Variables|Default|GetItemNames) (Math|Integer|Max(Integer) (Variables|Default|GetSelectedItem))) " - drop it on a room square")) "Drag an item onto a room square")))
  (CallFunction|UpdateDrag _drag_index))
"""


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def call(ts: str, tool: str, args: dict) -> dict | str:
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
    req = urllib.request.Request(
        MCP_URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Mcp-Session-Id": sid(),
        },
    )
    text = json.loads(urllib.request.urlopen(req, timeout=180).read())["result"]["content"][0]["text"]
    if text.startswith("Function") or text.startswith("Unknown") or "received" in text:
        raise RuntimeError(text[:1200])
    if text.strip().startswith("{"):
        return json.loads(text)
    return text


def main() -> None:
    call(BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": TICK_STORE_DSL.strip()})
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}})["returnValue"]
    if "_drag_index" not in verify:
        raise RuntimeError(f"patch missing:\n{verify}")
    call(AT, "save_assets", {"asset_paths": ["/Game/Core/BP_DungeonGrid"]})
    print("TickStore patched — stop PIE, play again")


if __name__ == "__main__":
    main()
