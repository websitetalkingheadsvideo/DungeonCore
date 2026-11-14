"""One-gesture store drag: ghost while placing; drop only on LMB release (not Pressed < 0)."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

# Full replacement — same structure as asset, fixed branch order.
UPDATE_DRAG = """
(fn UpdateDrag (Pressed)
  (bind _returnvalue (Math|Integer|Max(Integer) Pressed))
  (bind _itemcosts (Variables|Default|GetItemCosts))
  (bind _output (Utilities|Array|Get(acopy) _itemcosts _returnvalue))
  (bind _selecteditem (Variables|Default|GetSelectedItem))
  (bind _pc (Game|GetPlayerController 0))
  (bind _lmb_up (Game|Player|WasInputKeyJustReleased _pc "LeftMouseButton"))
  (CallFunction|UpdateCursorCell)
  (if (and (>= Pressed 0) (< _selecteditem 0))
    (if (>= (Variables|Default|GetMana) _output)
      (Variables|Default|SetSelectedItem Pressed)
      (Variables|Default|SetMsgTimer 0.0)
      (else
        (Variables|Default|SetStatusMsg (Utilities|String|Append "Not enough mana for " (Utilities|String|Append (Utilities|Array|Get(acopy) (Variables|Default|GetItemNames) (Math|Integer|Max(Integer) Pressed)) (Utilities|String|Append " - needs " (Utilities|String|ToString(Integer) (Math|Float|Round _output))))))
        (Variables|Default|SetMsgTimer 3.0)))
    (elif (and _lmb_up (>= _selecteditem 0))
      (Transformation|SetRelativeLocation (Variables|Default|GetGhost) (Math|Vector|MakeVector 0.0 0.0 -5000.0))
      (if (Variables|Default|GetCursorOk)
        (CallFunction|TryPlace (Variables|Default|GetCursorX) (Variables|Default|GetCursorY))
        (Variables|Default|SetSelectedItem -1)
        (else
          (Variables|Default|SetSelectedItem -1)))
    (elif (>= _selecteditem 0)
      (Transformation|SetRelativeLocation (Variables|Default|GetGhost) (Math|Vector|MakeVector (* (- (Math|Conversions|ToFloat(Integer) (Variables|Default|GetCursorX)) (* (Math|Conversions|ToFloat(Integer) (- (Variables|Default|GetGridSize) 1)) 0.5)) (Variables|Default|GetCellSize)) (* (- (Math|Conversions|ToFloat(Integer) (Variables|Default|GetCursorY)) (* (Math|Conversions|ToFloat(Integer) (- (Variables|Default|GetGridSize) 1)) 0.5)) (Variables|Default|GetCellSize)) (select (Variables|Default|GetCursorOk) 3.0 -5000.0)))
    (else
      (CallFunction|OnLeftClick))))))
""".strip()


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
    if not text.strip().startswith("{"):
        raise RuntimeError(text[:2000])
    return json.loads(text)


def main() -> None:
    w = call(BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}, "code": UPDATE_DRAG})
    print("write:", w)
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}})["returnValue"]
    if "WasInputKeyJustReleased" not in verify or "(< Pressed 0)" in verify:
        raise RuntimeError(verify[:2500])
    call(AT, "save_assets", {"asset_paths": ["/Game/Core/BP_DungeonGrid"]})
    print("UpdateDrag: one LMB gesture (ghost while placing, drop on LMB up)")


if __name__ == "__main__":
    main()