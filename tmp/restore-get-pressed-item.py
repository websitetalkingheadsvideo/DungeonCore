"""Restore clean GetPressedItem + working DragLatch."""
from __future__ import annotations

import json
import re
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
URL = "http://127.0.0.1:8000/mcp"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"


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
                URL,
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
    if any(
        s in text
        for s in ("RuntimeError", "AssertionError", "Unexpected", "Unclosed", "does not exist")
    ):
        raise RuntimeError(text[:2000])
    return json.loads(text) if text.strip().startswith("{") else text


def main() -> None:
    hover = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:GetHoveredItem"}})["returnValue"]
    chain = hover.split("(return ", 1)[1].strip()[:-1]
    pressed_chain = chain.replace("Widget|IsHovered", "Button|IsPressed")
    code = f"""(fn GetPressedItem ()
  (bind _pc (Game|GetPlayerController 0))
  (bind _pressed {pressed_chain})
  (bind _latched (Variables|Default|GetDragLatch))
  (bind _updated (select (>= _pressed 0) _pressed (select (Game|Player|WasInputKeyJustReleased _pc "LeftMouseButton") -1 _latched)))
  (Variables|Default|SetDragLatch _updated)
  (return _updated))"""
    if code.count("(") != code.count(")"):
        raise RuntimeError(f"paren mismatch: {code.count('(')} {code.count(')')}")
    call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:GetPressedItem"}, "code": code})
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:GetPressedItem"}})["returnValue"]
    if "SetDragLatch" not in verify or "Button|IsPressed" not in verify:
        raise RuntimeError(verify[:2000])
    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call(AT, "save_assets", {"asset_paths": ["/Game/UI/WBP_DungeonHUD"]})
    print("GetPressedItem restored OK")


if __name__ == "__main__":
    main()
