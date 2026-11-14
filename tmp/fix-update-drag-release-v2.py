"""Drop on LMB release, not when store button unpresses."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

OLD = "(elif (and (< Pressed 0) (>= _selecteditem 0))"
NEW = '(elif (and (>= _selecteditem 0) (Game|Player|WasInputKeyJustReleased (Game|GetPlayerController 0) "LeftMouseButton"))'


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
            timeout=300,
        ).read()
    )["result"]["content"][0]["text"]
    if not text.strip().startswith("{"):
        raise RuntimeError(text[:1500])
    return json.loads(text)


def main() -> None:
    dsl = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}})["returnValue"]
    if OLD not in dsl:
        if NEW in dsl:
            print("UpdateDrag release already patched")
            return
        raise RuntimeError(dsl)
    patched = dsl.replace(OLD, NEW, 1)
    out = call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}, "code": patched})
    if isinstance(out, dict) and out.get("returnValue") is None:
        verify = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}})["returnValue"]
        if "WasInputKeyJustReleased" not in verify:
            raise RuntimeError(verify)
    call(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {"asset_paths": ["/Game/Core/BP_DungeonGrid"]},
    )
    print("UpdateDrag: drop on LMB release")


if __name__ == "__main__":
    main()