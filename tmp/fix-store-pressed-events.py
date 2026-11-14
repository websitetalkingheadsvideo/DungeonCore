"""Strip null Btn_* reads: latch-only GetPressedItem + no-op highlight/mana/status."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"


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
    if any(x in text for x in ("RuntimeError", "AssertionError", "does not exist", "Top-level form")):
        raise RuntimeError(text[:2500])
    if text.strip().startswith("{"):
        return json.loads(text)
    return text


def main() -> None:
    vars_out = call(BT, "list_variables", {"blueprint": {"refPath": HUD}})
    if "DragLatch" not in vars_out.get("returnValue", []):
        call(BT, "add_variable", {"blueprint": {"refPath": HUD}, "name": "DragLatch", "type_name": "int"})

    graphs = {
        "GetPressedItem": """(fn GetPressedItem ()
  (bind _pc (Game|GetPlayerController 0))
  (bind _latched (Variables|Default|GetDragLatch))
  (if (Game|Player|WasInputKeyJustReleased _pc "LeftMouseButton")
    (Variables|Default|SetDragLatch -1)
    (return -1))
  (return _latched))""",
        "GetHoveredItem": "(fn GetHoveredItem () (return (Variables|Default|GetDragLatch)))",
        "Highlight": "(fn Highlight (Hovered Selected))",
        "UpdateMana": "(fn UpdateMana (Mana MaxMana))",
        "SetStatus": "(fn SetStatus (Msg))",
    }
    for name, code in graphs.items():
        call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:{name}"}, "code": code})

    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("HUD store graphs stripped of null Btn_* refs.")


if __name__ == "__main__":
    main()
