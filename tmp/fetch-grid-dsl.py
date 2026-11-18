"""Fetch TryPlace and RotateItemTo DSL from Unreal MCP HTTP."""
from __future__ import annotations

import json
import urllib.request
from pathlib import Path

SID = Path(__file__).resolve().parents[0].joinpath("mcp-sid.txt").read_text(encoding="utf-8").strip()
MCP = "http://127.0.0.1:8000/mcp"
OUT = Path(__file__).resolve().parents[0]


def call(tool: str, args: dict) -> dict:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {
                    "toolset_name": "editor_toolset.toolsets.blueprint.BlueprintTools",
                    "tool_name": tool,
                    "arguments": args,
                },
            },
        }
    ).encode()
    req = urllib.request.Request(
        MCP,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Mcp-Session-Id": SID,
        },
    )
    raw = json.loads(urllib.request.urlopen(req, timeout=120).read().decode())
    text = raw["result"]["content"][0]["text"]
    return json.loads(text)


def main() -> None:
    for fn in ("TryPlace", "RotateItemTo"):
        resp = call(
            "read_graph_dsl",
            {"graph": {"refPath": f"/Game/Core/BP_DungeonGrid.BP_DungeonGrid:{fn}"}},
        )
        slug = "tryplace" if fn == "TryPlace" else "rotate"
        path = OUT / f"{slug}-original.dsl"
        path.write_text(resp["returnValue"], encoding="utf-8")
        print(path, len(resp["returnValue"]))


if __name__ == "__main__":
    main()