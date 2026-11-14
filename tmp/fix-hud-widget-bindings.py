"""Re-link WBP_DungeonHUD named widget variables after MCP tree edits."""
from __future__ import annotations

import json
import sys
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
NAMES = [f"Btn_{i}" for i in range(9)] + ["Bar_Mana", "Txt_Mana", "Txt_Status"]


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
    raw = json.loads(urllib.request.urlopen(req, timeout=300).read())["result"]["content"][0]["text"]
    if raw.startswith("Function") or raw.startswith("Unknown"):
        raise RuntimeError(f"{ts}.{tool}: {raw}")
    if not raw.strip():
        return {}
    outer = json.loads(raw)
    rv = outer.get("returnValue", outer)
    return json.loads(rv) if isinstance(rv, str) else rv


def bp() -> dict:
    return {"refPath": HUD}


def main() -> None:
    desc = call_json("UMGToolSet.UMGToolSet", "GetWidgetDescription", {"widgetBlueprint": bp()})
    by_name = {w["widgetName"]: w for w in desc["widgets"]}

    missing = [n for n in NAMES if n not in by_name]
    if missing:
        print("missing widgets:", missing, file=sys.stderr)
        sys.exit(1)

    for name in NAMES:
        w = by_name[name]
        widget_ref = w["widget"]
        if not w.get("bIsVariable"):
            call_json(
                "UMGToolSet.UMGToolSet",
                "ToggleWidgetAsVariable",
                {"widgetBlueprint": bp(), "widget": widget_ref, "bIsVariable": True},
            )
            print(f"enabled variable {name}")
        else:
            call_json(
                "UMGToolSet.UMGToolSet",
                "ToggleWidgetAsVariable",
                {"widgetBlueprint": bp(), "widget": widget_ref, "bIsVariable": False},
            )
            call_json(
                "UMGToolSet.UMGToolSet",
                "ToggleWidgetAsVariable",
                {"widgetBlueprint": bp(), "widget": widget_ref, "bIsVariable": True},
            )
            print(f"relinked variable {name}")

    call_json("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": bp()})
    call_json("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("compiled and saved — stop PIE and play again")


if __name__ == "__main__":
    main()
