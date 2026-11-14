"""Fix store drag: end placement on LMB release, not when Btn IsPressed drops."""
from __future__ import annotations

import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

OLD = "(elif (and (< Pressed 0) (>= _selecteditem 0))"
NEW = '(elif (and (>= _selecteditem 0) (Game|Player|WasInputKeyJustReleased (Game|GetPlayerController 0) "LeftMouseButton"))'


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def call(toolset: str, tool: str, arguments: dict) -> str:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": toolset, "tool_name": tool, "arguments": arguments},
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
    return json.loads(urllib.request.urlopen(req, timeout=300).read())["result"]["content"][0]["text"]


def main() -> None:
    raw = call(
        BT,
        "read_graph_dsl",
        {"graph": {"refPath": f"{GRID}:UpdateDrag"}},
    )
    dsl = json.loads(raw)["returnValue"]
    if OLD not in dsl:
        raise RuntimeError(f"pattern not found; got:\n{dsl}")
    patched = dsl.replace(OLD, NEW, 1)
    write_raw = call(BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}, "code": patched})
    if write_raw.strip() and not write_raw.strip().startswith("{"):
        if "Unclosed" in write_raw or "Error" in write_raw:
            raise RuntimeError(write_raw)
    verify = json.loads(call(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateDrag"}}))[
        "returnValue"
    ]
    print("patched line present:", NEW in verify)
    call(AT, "save_assets", {"asset_paths": ["/Game/Core/BP_DungeonGrid"]})
    print("saved")


if __name__ == "__main__":
    main()
