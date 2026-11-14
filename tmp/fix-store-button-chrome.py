"""Compact store row chrome: left-aligned auto-width buttons."""
from __future__ import annotations

import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
OB = "editor_toolset.toolsets.object.ObjectTools"


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def call_json(ts: str, tool: str, arguments: dict) -> dict:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": ts, "tool_name": tool, "arguments": arguments},
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
    text = json.loads(urllib.request.urlopen(req, timeout=300).read())["result"]["content"][0]["text"]
    if not text.strip():
        return {}
    outer = json.loads(text)
    rv = outer.get("returnValue", outer)
    if isinstance(rv, str):
        try:
            return json.loads(rv)
        except json.JSONDecodeError:
            return {"raw": rv}
    return rv


def wref(name: str) -> dict:
    return {"refPath": f"{HUD}:WidgetTree.{name}"}


def bp() -> dict:
    return {"refPath": HUD}


def set_props(instance: dict, values: dict) -> None:
    call_json(OB, "set_properties", {"instance": instance, "values": json.dumps(values)})


def slot_ref(widget_name: str) -> str:
    props = call_json(OB, "get_properties", {"instance": wref(widget_name), "properties": ["slot"]})
    return props["slot"]["refPath"]


def main() -> None:
    pad = {"left": 6, "top": 4, "right": 8, "bottom": 4}
    style_pad = {"left": 4, "top": 4, "right": 4, "bottom": 4}
    cost_color = {
        "specifiedColor": {"R": 0.72, "G": 0.85, "B": 1.0, "A": 1.0},
        "colorUseRule": "UseColor_Specified",
    }

    for index in range(9):
        btn = f"Btn_{index}"
        row = f"Row_{index}"
        icon = f"Icon_{index}"
        cost = f"Cost_{index}"

        set_props(
            {"refPath": slot_ref(btn)},
            {
                "Padding": pad,
                "Size": {"SizeRule": "Automatic", "Value": 1.0},
                "HorizontalAlignment": "HAlign_Left",
                "VerticalAlignment": "VAlign_Center",
            },
        )
        set_props(
            wref(btn),
            {
                "WidgetStyle": {
                    "NormalPadding": style_pad,
                    "PressedPadding": style_pad,
                },
                "PressMethod": "ButtonPress",
            },
        )
        set_props(wref(row), {"Visibility": "SelfHitTestInvisible"})
        set_props(
            {"refPath": slot_ref(icon)},
            {
                "Padding": {"left": 0, "top": 0, "right": 6, "bottom": 0},
                "Size": {"SizeRule": "Automatic", "Value": 1.0},
                "HorizontalAlignment": "HAlign_Left",
                "VerticalAlignment": "VAlign_Center",
            },
        )
        set_props(
            {"refPath": slot_ref(cost)},
            {
                "Padding": {"left": 0, "top": 0, "right": 0, "bottom": 0},
                "Size": {"SizeRule": "Automatic", "Value": 1.0},
                "HorizontalAlignment": "HAlign_Left",
                "VerticalAlignment": "VAlign_Center",
            },
        )
        set_props(
            wref(cost),
            {"ColorAndOpacity": cost_color, "Visibility": "SelfHitTestInvisible"},
        )
        print(f"chrome {index}")

    call_json("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": bp()})
    call_json("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("saved")


if __name__ == "__main__":
    main()
