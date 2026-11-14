"""Fix TickStore GetHoveredItem call on HUD."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

OLD = "(bind _index_1 (Variables|Default|GetHoveredItem _hud))"
NEW = "(bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))"


def call(tool: str, args: dict) -> dict:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": BT, "tool_name": tool, "arguments": args},
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
    if not text.strip().startswith("{"):
        raise RuntimeError(text[:1200])
    return json.loads(text)


def main() -> None:
    tick = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}})["returnValue"]
    if NEW in tick:
        print("TickStore OK")
        return
    if OLD not in tick:
        raise RuntimeError(tick)
    call(
        "write_graph_dsl",
        {"graph": {"refPath": f"{GRID}:TickStore"}, "code": tick.replace(OLD, NEW, 1)},
    )
    call(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {"asset_paths": ["/Game/Core/BP_DungeonGrid"]},
    )
    print("TickStore GetHoveredItem fixed")


if __name__ == "__main__":
    main()