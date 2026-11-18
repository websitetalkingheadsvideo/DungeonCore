"""Patch BP_DungeonGrid for Drag Lattice (#15) placement — MCP HTTP."""
from __future__ import annotations

import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

OLD_MIRROR_TAIL = (
    "(select (== _selecteditem 14) (Variables|Default|GetMirrorPillars) "
    "(Variables|Default|GetItems))"
)
NEW_MIRROR_TAIL = (
    "(select (== _selecteditem 14) (Variables|Default|GetMirrorPillars) "
    "(select (== _selecteditem 15) (Variables|Default|GetDragLattices) "
    "(Variables|Default|GetItems)))"
)

OLD_ROTATE_TAIL = (
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 15) "
    "(Variables|Default|GetMirrorPillars) (Variables|Default|GetItems))"
)
NEW_ROTATE_TAIL = (
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 15) "
    "(Variables|Default|GetMirrorPillars) "
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 16) "
    "(Variables|Default|GetDragLattices) (Variables|Default|GetItems)))"
)

TINT_LINE = "  (CallFunction|TintOne (Variables|Default|GetDragLattices) Tint))"


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def call_json(toolset: str, tool: str, arguments: dict) -> dict:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {
                    "toolset_name": toolset,
                    "tool_name": tool,
                    "arguments": arguments,
                },
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
    text = json.loads(urllib.request.urlopen(req, timeout=300).read().decode())[
        "result"
    ]["content"][0]["text"]
    if text.startswith("Function") or text.startswith("Unknown"):
        raise RuntimeError(f"{toolset}.{tool}: {text}")
    return json.loads(text) if text.strip() else {}


def read_dsl(fn: str) -> str:
    return call_json(
        "editor_toolset.toolsets.blueprint.BlueprintTools",
        "read_graph_dsl",
        {"graph": {"refPath": f"{GRID}:{fn}"}},
    )["returnValue"]


def write_dsl(fn: str, code: str) -> None:
    out = call_json(
        "editor_toolset.toolsets.blueprint.BlueprintTools",
        "write_graph_dsl",
        {"graph": {"refPath": f"{GRID}:{fn}"}, "code": code},
    )
    print(f"write {fn}:", out)


def main() -> None:
    try_place = read_dsl("TryPlace")
    if OLD_MIRROR_TAIL not in try_place:
        raise RuntimeError("TryPlace: expected mirror tail missing")
    try_place = try_place.replace("(bind _returnvalue_14 14)", "(bind _returnvalue_14 15)", 1)
    try_place = try_place.replace(OLD_MIRROR_TAIL, NEW_MIRROR_TAIL, 1)
    if "GetDragLattices" not in try_place:
        raise RuntimeError("TryPlace patch failed")
    write_dsl("TryPlace", try_place)

    rotate = read_dsl("RotateItemTo")
    if OLD_ROTATE_TAIL not in rotate:
        raise RuntimeError("RotateItemTo: expected mirror tail missing")
    rotate = rotate.replace(OLD_ROTATE_TAIL, NEW_ROTATE_TAIL, 1)
    rotate = rotate.replace(
        "(<= (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 15)",
        "(<= (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 16)",
    )
    write_dsl("RotateItemTo", rotate)

    tint = read_dsl("TintItems")
    if "GetDragLattices" in tint:
        print("TintItems already has DragLattices")
    else:
        if not tint.rstrip().endswith("GetMirrorPillars) Tint))"):
            raise RuntimeError("TintItems: unexpected ending")
        tint = tint.rstrip()[:-1] + "\n" + TINT_LINE
        write_dsl("TintItems", tint)

    call_json(
        "editor_toolset.toolsets.blueprint.BlueprintTools",
        "compile_blueprint",
        {"blueprint": {"refPath": GRID}},
    )
    call_json(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {"asset_paths": ["/Game/Core/BP_DungeonGrid", "/Game/Props/StoreItems/drag_lattice/drag_lattice_unreal_SM"]},
    )
    print("Drag Lattice placement wired (item 15).")


if __name__ == "__main__":
    main()