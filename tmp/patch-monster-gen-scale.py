"""Patch TryPlace + RotateItemTo: Monster Generator (item 0 / placed 1) at 2x scale."""
from __future__ import annotations

import json
import urllib.request
from pathlib import Path

SID = Path(__file__).resolve().parent.joinpath("mcp-sid.txt").read_text(encoding="utf-8").strip()
MCP = "http://127.0.0.1:8000/mcp"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

TRY_SCALE_OLD = (
    "(select _returnvalue_25 (Math|Vector|MakeVector 1.0 1.0 1.0) "
    "(Math|Vector|MakeVector 0.7 0.7 0.4)))))"
)
TRY_SCALE_NEW = (
    "(select _returnvalue_25 (select (== _selecteditem 0) "
    "(Math|Vector|MakeVector 2.0 2.0 2.0) (Math|Vector|MakeVector 1.0 1.0 1.0)) "
    "(Math|Vector|MakeVector 0.7 0.7 0.4)))))"
)

ROT_HAS_MODEL = (
    "(select (or (or (or (and (>= (Utilities|Array|Get(acopy) "
    "(Variables|Default|GetPlaced) Idx) 1) (<= (Utilities|Array|Get(acopy) "
    "(Variables|Default|GetPlaced) Idx) 5)) (or (== (Utilities|Array|Get(acopy) "
    "(Variables|Default|GetPlaced) Idx) 7) (== (Utilities|Array|Get(acopy) "
    "(Variables|Default|GetPlaced) Idx) 10))) (== (Utilities|Array|Get(acopy) "
    "(Variables|Default|GetPlaced) Idx) 11)) (and (>= (Utilities|Array|Get(acopy) "
    "(Variables|Default|GetPlaced) Idx) 12) (<= (Utilities|Array|Get(acopy) "
    "(Variables|Default|GetPlaced) Idx) 16))) (Math|Vector|MakeVector 1.0 1.0 1.0) "
    "(Math|Vector|MakeVector 0.7 0.7 0.4))) false true)"
)
ROT_OR_INNER = ROT_HAS_MODEL[len("(select ") : -len(" false true)")]
ROT_SCALE_NEW = (
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 1) "
    "(Math|Vector|MakeVector 2.0 2.0 2.0) "
    f"(select {ROT_OR_INNER})) false true)"
)


def call_tool(tool: str, args: dict) -> dict:
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
    raw = json.loads(urllib.request.urlopen(req, timeout=180).read().decode())
    text = raw["result"]["content"][0]["text"]
    if raw["result"].get("isError"):
        raise RuntimeError(text)
    return json.loads(text)


def balance_ok(code: str) -> bool:
    depth = 0
    for ch in code:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if depth < 0:
            return False
    return depth == 0


def main() -> None:
    for fn, patch in (
        ("TryPlace", lambda c: c.replace(TRY_SCALE_OLD, TRY_SCALE_NEW, 1)),
        ("RotateItemTo", lambda c: c.replace(ROT_HAS_MODEL, ROT_SCALE_NEW, 1)),
    ):
        read = call_tool("read_graph_dsl", {"graph": {"refPath": f"{GRID}:{fn}"}})
        code = read["returnValue"]
        if fn == "TryPlace" and TRY_SCALE_OLD not in code:
            raise RuntimeError("TryPlace: expected scale snippet missing")
        if fn == "RotateItemTo" and ROT_HAS_MODEL not in code:
            raise RuntimeError("RotateItemTo: expected scale snippet missing")
        patched = patch(code)
        if not balance_ok(patched):
            raise RuntimeError(f"{fn}: unbalanced parens")
        call_tool(
            "write_graph_dsl",
            {"graph": {"refPath": f"{GRID}:{fn}"}, "code": patched},
        )
        print(fn, "patched OK")


if __name__ == "__main__":
    main()