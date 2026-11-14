"""Grid PendingPress: keep store index while LMB held (fixes instant cancel)."""
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
  (bind _press_raw (Class|WBPDungeonHUD|GetPressedItem _hud))
  (bind _pc (Game|GetPlayerController 0))
  (bind _pending (Variables|Default|GetPendingPress))
  (bind _store_next (select (>= _press_raw 0) _press_raw _pending))
  (Variables|Default|SetPendingPress _store_next)
  (bind _drag (select (and (>= _store_next 0) (Game|Player|IsInputKeyDown _pc "LeftMouseButton")) _store_next _press_raw))
  (bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))
  (Variables|Default|SetHoveredItem _index_1)
  (Variables|Default|SetMsgTimer (- (Variables|Default|GetMsgTimer) (Utilities|Time|GetWorldDeltaSeconds)))
  (Class|WBPDungeonHUD|Highlight _hud (Variables|Default|GetHoveredItem) (Variables|Default|GetSelectedItem))
  (Class|WBPDungeonHUD|SetStatus _hud (select (> (Variables|Default|GetMsgTimer) 0.0) (Variables|Default|GetStatusMsg) (select (>= (Variables|Default|GetSelectedItem) 0) (Utilities|String|Append "Placing: " (Utilities|String|Append (Utilities|Array|Get(acopy) (Variables|Default|GetItemNames) (Math|Integer|Max(Integer) (Variables|Default|GetSelectedItem))) " - drop it on a room square")) "Drag an item onto a room square")))
  (CallFunction|UpdateDrag _drag))
""".strip()


def call(ts: str, tool: str, args: dict):
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
    if any(x in text for x in ("RuntimeError", "AssertionError", "does not exist", "Top-level form", "cannot rebind")):
        raise RuntimeError(text[:2500])
    return json.loads(text) if text.strip().startswith("{") else text


def main() -> None:
    vars_out = call(BT, "list_variables", {"blueprint": {"refPath": GRID}})
    if "PendingPress" not in vars_out.get("returnValue", []):
        call(BT, "add_variable", {"blueprint": {"refPath": GRID}, "name": "PendingPress", "type_name": "int"})

    w = call(BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": TICK_STORE_DSL})
    print("write:", str(w)[:400])
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}})["returnValue"]
    if "PendingPress" not in verify or "CallFunction|UpdateDrag _drag" not in verify:
        raise RuntimeError(verify)
    call(AT, "save_assets", {"asset_paths": ["/Game/Core/BP_DungeonGrid"]})
    print("TickStore PendingPress OK — STOP PIE then Play")


if __name__ == "__main__":
    main()