"""Store drag without HUD Btn_*: keys 1-9 + LMB latch on BP_DungeonGrid."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

KEYS = [
    ("One", 0),
    ("Two", 1),
    ("Three", 2),
    ("Four", 3),
    ("Five", 4),
    ("Six", 5),
    ("Seven", 6),
    ("Eight", 7),
    ("Nine", 8),
]

STORE_LATCH = """  (bind _pc_store (Game|GetPlayerController 0))
  (bind _latched (Variables|Default|GetStoreDragIndex))
  (bind _pick -1)"""

for key, idx in KEYS:
    STORE_LATCH += f"""
  (if (Game|Player|IsInputKeyJustPressed _pc_store "{key}") (bind _pick {idx}))"""

STORE_LATCH += """
  (bind _next (select (>= _pick 0) _pick _latched))
  (bind _holding (and (>= _next 0) (Game|Player|IsInputKeyDown _pc_store "LeftMouseButton")))
  (bind _out (select _holding _next (select (Game|Player|WasInputKeyJustReleased _pc_store "LeftMouseButton") -1 _latched)))
  (Variables|Default|SetStoreDragIndex _out)"""

TICK_OLD = "(bind _index (Class|WBPDungeonHUD|GetPressedItem _hud))"
TICK_NEW = "(bind _index (Variables|Default|GetStoreDragIndex))"


def call(tool: str, args: dict) -> dict | str:
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
    if text.startswith("RuntimeError") or text.startswith("AssertionError"):
        raise RuntimeError(text[:2000])
    if text.strip().startswith("{"):
        return json.loads(text)
    return text


def main() -> None:
    vars_out = call("list_variables", {"blueprint": {"refPath": GRID}})
    names = vars_out.get("returnValue", [])
    if "StoreDragIndex" not in names:
        call(
            "add_variable",
            {"blueprint": {"refPath": GRID}, "name": "StoreDragIndex", "type_name": "int"},
        )

    eg = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:EventGraph"}})["returnValue"]
    if "SetStoreDragIndex" in eg:
        print("EventGraph latch already present")
    else:
        anchor = "  (CallFunction|TickStore))"
        if anchor not in eg:
            raise RuntimeError(eg)
        patched_eg = eg.replace(anchor, STORE_LATCH + "\n" + anchor, 1)
        call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:EventGraph"}, "code": patched_eg})

    tick = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}})["returnValue"]
    if TICK_NEW in tick:
        print("TickStore already uses StoreDragIndex")
    elif TICK_OLD in tick:
        call(
            "write_graph_dsl",
            {
                "graph": {"refPath": f"{GRID}:TickStore"},
                "code": tick.replace(TICK_OLD, TICK_NEW, 1),
            },
        )
    else:
        raise RuntimeError(tick)

    verify = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:EventGraph"}})["returnValue"]
    if "StoreDragIndex" not in verify:
        raise RuntimeError("EventGraph patch missing")

    call(AT, "save_assets", {"asset_paths": ["/Game/Core/BP_DungeonGrid"]})
    print("GRID: hold 1-9 then LMB drag; release LMB to place. HUD buttons optional.")


if __name__ == "__main__":
    main()