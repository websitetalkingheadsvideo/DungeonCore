"""Reparent WBP_DungeonHUD to DungeonBlueprintStoreHud and strip overriding BP functions."""
from __future__ import annotations

import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
CPP_PARENT = "/Script/DungeonCore.DungeonBlueprintStoreHud"
FUNCS = ["GetPressedItem", "GetHoveredItem", "Highlight", "UpdateMana", "SetStatus"]


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
    if text.startswith("Unknown tool"):
        return text
    if text.strip().startswith("{"):
        return json.loads(text)
    return text


def main() -> None:
    tools_to_try = [
        ("editor_toolset.toolsets.blueprint.BlueprintTools", "set_parent_class"),
        ("UMGToolSet.UMGToolSet", "SetWidgetBlueprintParentClass"),
        ("editor_toolset.toolsets.blueprint.BlueprintTools", "reparent"),
    ]
    for ts, tool in tools_to_try:
        out = call(
            ts,
            tool,
            {"blueprint": {"refPath": HUD}, "parent_class": {"refPath": CPP_PARENT}, "widgetBlueprint": {"refPath": HUD}},
        )
        print(ts, tool, "=>", out)

    for fn in FUNCS:
        for tool in ("remove_function", "delete_function", "remove_graph"):
            out = call(
                "editor_toolset.toolsets.blueprint.BlueprintTools",
                tool,
                {"blueprint": {"refPath": HUD}, "graph": {"refPath": f"{HUD}:{fn}"}, "name": fn},
            )
            if isinstance(out, str) and out.startswith("Unknown"):
                continue
            print("removed", fn, "via", tool, out)
            break

    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    graphs = call("editor_toolset.toolsets.blueprint.BlueprintTools", "list_graphs", {"blueprint": {"refPath": HUD}})
    print("graphs:", graphs)


if __name__ == "__main__":
    main()
