"""Store buttons: DownAndUp press, row visible for reliable clicks."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
URL = "http://127.0.0.1:8000/mcp"
OB = "editor_toolset.toolsets.object.ObjectTools"
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
                URL,
                data=body,
                headers={
                    "Content-Type": "application/json",
                    "Accept": "application/json, text/event-stream",
                    "Mcp-Session-Id": SID,
                },
            ),
            timeout=120,
        ).read()
    )["result"]["content"][0]["text"]
    if text.startswith("Function") or text.startswith("Unknown"):
        raise RuntimeError(text[:400])


def wref(name: str) -> dict:
    return {"refPath": f"{HUD}:WidgetTree.{name}"}


def main() -> None:
    vals = {
        "PressMethod": "DownAndUp",
        "ClickMethod": "DownAndUp",
        "Visibility": "Visible",
        "bIsEnabled": True,
    }
    for i in range(9):
        call(
            OB,
            "set_properties",
            {"instance": wref(f"Btn_{i}"), "values": json.dumps(vals)},
        )
        call(
            OB,
            "set_properties",
            {
                "instance": wref(f"Row_{i}"),
                "values": json.dumps({"Visibility": "Visible"}),
            },
        )
        call(
            OB,
            "set_properties",
            {
                "instance": wref(f"Icon_{i}"),
                "values": json.dumps({"Visibility": "SelfHitTestInvisible"}),
            },
        )
        call(
            OB,
            "set_properties",
            {
                "instance": wref(f"Cost_{i}"),
                "values": json.dumps({"Visibility": "SelfHitTestInvisible"}),
            },
        )
    call(
        "UMGToolSet.UMGToolSet",
        "CompileWidgetBlueprint",
        {"widgetBlueprint": {"refPath": HUD}},
    )
    call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("button input reset")


if __name__ == "__main__":
    main()
