"""Find graphs that reference RoomId on BP_DungeonGrid."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"


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
            timeout=120,
        ).read()
    )["result"]["content"][0]["text"]
    return json.loads(text)


def main() -> None:
    graphs = call("list_graphs", {"blueprint": {"refPath": GRID}})["returnValue"]
    for g in graphs:
        name = g["refPath"].split(":")[-1]
        dsl = call("read_graph_dsl", {"graph": g})["returnValue"]
        if "RoomId" in dsl or "room" in dsl.lower():
            print(f"\n{'=' * 60}\n{name}\n{'=' * 60}")
            print(dsl[:4000])


if __name__ == "__main__":
    main()