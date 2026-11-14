"""Room placement: CursorOk = same rules as TryPlace (RoomId>0, empty, not core)."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

OLD_BLOCK = """  (Variables|Default|SetCursorOk (Game|Player|GetHitResultUnderCursorByChannel _returnvalue "TraceTypeQuery1" false))
  (Variables|Default|SetCursorX (Math|Float|Round (+ (/ (.x _returnvalue_10) _cellsize) _returnvalue_7)))
  (Variables|Default|SetCursorY (Math|Float|Round (+ (/ (.y _returnvalue_10) _cellsize) _returnvalue_7)))"""
NEW_BLOCK = """  (Variables|Default|SetCursorX (Math|Float|Round (+ (/ (.x _returnvalue_10) _cellsize) _returnvalue_7)))
  (Variables|Default|SetCursorY (Math|Float|Round (+ (/ (.y _returnvalue_10) _cellsize) _returnvalue_7)))
  (bind _cx (Variables|Default|GetCursorX))
  (bind _cy (Variables|Default|GetCursorY))
  (bind _gs (Variables|Default|GetGridSize))
  (bind _ci (+ (* _cy _gs) _cx))
  (bind _core (/ (- (* _gs _gs) 1) 2))
  (bind _hit (Game|Player|GetHitResultUnderCursorByChannel _returnvalue "TraceTypeQuery1" false))
  (Variables|Default|SetCursorOk (and _hit (and (and (> (Utilities|Array|Get(acopy) (Variables|Default|GetRoomId) _ci) 0) (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) _ci) 0)) (!= _ci _core))))"""


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
        raise RuntimeError(text[:1500])
    return json.loads(text)


def main() -> None:
    cell = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateCursorCell"}})["returnValue"]
    if OLD_BLOCK not in cell:
        raise RuntimeError(f"pattern missing in UpdateCursorCell:\n{cell}")
    patched = cell.replace(OLD_BLOCK, NEW_BLOCK, 1)
    call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateCursorCell"}, "code": patched})
    verify = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:UpdateCursorCell"}})["returnValue"]
    if "GetRoomId" not in verify or "GetPlaced" not in verify:
        raise RuntimeError(verify)
    call(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {"asset_paths": ["/Game/Core/BP_DungeonGrid"]},
    )
    print("CursorOk now matches TryPlace room rules")


if __name__ == "__main__":
    main()