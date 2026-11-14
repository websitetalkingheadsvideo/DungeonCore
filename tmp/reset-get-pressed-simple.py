"""Reset GetPressedItem = GetHoveredItem with IsPressed."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"


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
            timeout=180,
        ).read()
    )["result"]["content"][0]["text"]
    if "RuntimeError" in text or "AssertionError" in text:
        raise RuntimeError(text[:2000])
    return json.loads(text) if text.strip().startswith("{") else text


def main() -> None:
    hover = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:GetHoveredItem"}})["returnValue"]
    code = hover.replace("Widget|IsHovered", "Button|IsPressed").replace(
        "GetHoveredItem", "GetPressedItem"
    )
    call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:GetPressedItem"}, "code": code})
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:GetPressedItem"}})["returnValue"]
    if "Button|IsPressed" not in verify:
        raise RuntimeError(verify)
    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("GetPressedItem reset OK")


if __name__ == "__main__":
    main()
