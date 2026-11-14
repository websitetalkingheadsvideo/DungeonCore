"""HUD GetPressedItem: latch store index until LMB release."""
from __future__ import annotations

import json
import re
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


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
    req = urllib.request.Request(
        MCP_URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Mcp-Session-Id": sid(),
        },
    )
    text = json.loads(urllib.request.urlopen(req, timeout=180).read())["result"]["content"][0]["text"]
    if (
        "RuntimeError" in text
        or text.startswith("Function")
        or "Unexpected" in text
        or "Unclosed" in text
        or "AssertionError" in text
        or "does not exist" in text
    ):
        raise RuntimeError(text[:1500])
    if text.strip().startswith("{"):
        return json.loads(text)
    return text


def main() -> None:
    vars_out = call(BT, "list_variables", {"blueprint": {"refPath": HUD}})
    names = vars_out.get("returnValue", []) if isinstance(vars_out, dict) else []
    if "DragLatch" not in names:
        call(BT, "add_variable", {"blueprint": {"refPath": HUD}, "name": "DragLatch", "type_name": "int"})
    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    hover = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:GetHoveredItem"}})["returnValue"]
    m = re.search(r"-1(\)+)", hover)
    if not m:
        raise RuntimeError(f"select tail not found:\n{old}")
    rest = m.group(1)[:-1]
    pressed_line = f"  (bind _pressed (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_0)) 0 (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_1)) 1 (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_2)) 2 (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_3)) 3 (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_4)) 4 (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_5)) 5 (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_6)) 6 (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_7)) 7 (select (Button|IsPressed (Variables|WBP_DungeonHUD|GetBtn_8)) 8 -1{rest}"
    patched = f"""(fn GetPressedItem ()
  (bind _pc (Game|GetPlayerController 0))
{pressed_line}
  (bind _latched (Variables|Default|GetDragLatch))
  (bind _updated (select (>= _pressed 0) _pressed (select (Game|Player|WasInputKeyJustReleased _pc "LeftMouseButton") -1 _latched)))
  (Variables|Default|SetDragLatch _updated)
  (return _updated))"""
    if patched.count("(") != patched.count(")"):
        raise RuntimeError(f"bad parens: {patched.count('(')} vs {patched.count(')')}")
    open(r"e:\Dungeon Core\tmp\get-pressed-patched.dsl", "w", encoding="utf-8").write(patched)
    write_out = call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:GetPressedItem"}, "code": patched})
    print("write:", write_out)
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:GetPressedItem"}})["returnValue"]
    if "DragLatch" not in verify:
        raise RuntimeError(f"verify failed:\n{verify}")
    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call(AT, "save_assets", {"asset_paths": ["/Game/UI/WBP_DungeonHUD"]})
    print("GetPressedItem latched — stop PIE, play again")


if __name__ == "__main__":
    main()
