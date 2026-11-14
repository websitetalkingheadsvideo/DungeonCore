"""TickStore: feed SelectedItem into UpdateDrag; LMB release passes -1 to place."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

TICK_STORE_DSL = """
(fn TickStore ()
  (bind _hud (Variables|Default|GetHUD))
  (bind _pc_ts (Game|GetPlayerController 0))
  (bind _index (Class|WBPDungeonHUD|GetPressedItem _hud))
  (bind _sel (Variables|Default|GetSelectedItem))
  (bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))
  (Variables|Default|SetHoveredItem _index_1)
  (Variables|Default|SetMsgTimer (- (Variables|Default|GetMsgTimer) (Utilities|Time|GetWorldDeltaSeconds)))
  (Class|WBPDungeonHUD|Highlight _hud (Variables|Default|GetHoveredItem) (Variables|Default|GetSelectedItem))
  (Class|WBPDungeonHUD|SetStatus _hud (select (> (Variables|Default|GetMsgTimer) 0.0) (Variables|Default|GetStatusMsg) (select (>= (Variables|Default|GetSelectedItem) 0) (Utilities|String|Append "Placing: " (Utilities|String|Append (Utilities|Array|Get(acopy) (Variables|Default|GetItemNames) (Math|Integer|Max(Integer) (Variables|Default|GetSelectedItem))) " - drop it on a room square")) "Drag an item onto a room square")))
  (CallFunction|UpdateDrag (select (>= _sel 0) _sel _index)))
""".strip()


def call(tool: str, args: dict) -> str:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {
                    "toolset_name": AT if tool == "save_assets" else BT,
                    "tool_name": tool,
                    "arguments": args,
                },
            },
        }
    ).encode()
    return json.loads(
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


def main() -> None:
    w = call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": TICK_STORE_DSL})
    print("WRITE:", w[:1200])
    v = json.loads(call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}))["returnValue"]
    ok = "select (>= _sel 0) _sel _index" in v and "Class|WBPDungeonHUD|GetHoveredItem" in v
    print("VERIFY:", ok)
    if not ok:
        raise SystemExit(1)
    call("save_assets", {"asset_paths": ["/Game/Core/BP_DungeonGrid"]})
    print("saved — STOP PIE then Play")


if __name__ == "__main__":
    main()