"""Bind Btn_0..8 OnPressed -> set DragLatch + grid can read via GetPressedItem."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"

# GetPressedItem reads DragLatch while LMB held; OnPressed sets latch.
GET_PRESSED = """
(fn GetPressedItem ()
  (bind _pc (Game|GetPlayerController 0))
  (bind _latched (Variables|Default|GetDragLatch))
  (if (Game|Player|WasInputKeyJustReleased _pc "LeftMouseButton")
    (Variables|Default|SetDragLatch -1)
    (return -1))
  (return _latched))
"""

ON_PRESSED_FN = """
(fn OnStorePressed (Index)
  (Variables|Default|SetDragLatch Index))
"""


def call(ts, tool, args):
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
    if any(x in text for x in ("RuntimeError", "AssertionError", "Unexpected", "does not exist")):
        raise RuntimeError(text[:2500])
    return json.loads(text) if text.strip().startswith("{") else text


def main() -> None:
    vars_out = call(BT, "list_variables", {"blueprint": {"refPath": HUD}})
    if "DragLatch" not in vars_out.get("returnValue", []):
        call(BT, "add_variable", {"blueprint": {"refPath": HUD}, "name": "DragLatch", "type_name": "int"})
    call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:OnStorePressed"}, "code": ON_PRESSED_FN.strip()})
    call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:GetPressedItem"}, "code": GET_PRESSED.strip()})

    events = []
    for i in range(9):
        events.append(
            f"(event UserInterface|ComponentBoundEvent|Btn_{i}|OnPressed\n"
            f"  (CallFunction|OnStorePressed {i}))"
        )
    event_graph = "(event UserInterface|EventConstruct)\n" + "\n".join(events)
    call(
        BT,
        "write_graph_dsl",
        {"graph": {"refPath": f"{HUD}:EventGraph"}, "code": event_graph},
    )

    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("OnPressed -> DragLatch wired")


if __name__ == "__main__":
    main()
