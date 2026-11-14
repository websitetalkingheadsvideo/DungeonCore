"""Bind HUD widget variables in EventConstruct via WidgetTree lookup (fixes None at PIE)."""
from __future__ import annotations

import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"

BTN = [f"Btn_{i}" for i in range(9)]
OTHER = ["Bar_Mana", "Txt_Mana", "Txt_Status"]


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
    if text.startswith("RuntimeError") or text.startswith("Function") or text.startswith("Unknown"):
        raise RuntimeError(text[:2000])
    if text.strip().startswith("{"):
        return json.loads(text)
    return text


def bind_line(member: str, widget_name: str) -> str:
    # Cast to concrete type so variable setter pins match.
    if member.startswith("Btn_"):
        cast = "UserInterface|CastButton"
    elif member == "Bar_Mana":
        cast = "UserInterface|CastProgressBar"
    else:
        cast = "UserInterface|CastTextBlock"
    lookup = (
        f'(UserInterface|WidgetTree|GetWidgetFromName '
        f'(UserInterface|WidgetTree|Get _w) "{widget_name}")'
    )
    return f"  (Variables|WBP_DungeonHUD|Set{member} ({cast} {lookup}))"


def main() -> None:
    lines = [bind_line(n, n) for n in BTN + OTHER]
    # Ensure DragLatch exists for placement latch (default -1 set in editor if missing).
    vars_out = call(BT, "list_variables", {"blueprint": {"refPath": HUD}})
    names = vars_out.get("returnValue", [])
    if "DragLatch" not in names:
        call(BT, "add_variable", {"blueprint": {"refPath": HUD}, "name": "DragLatch", "type_name": "int"})
    lines.append("  (Variables|Default|SetDragLatch -1)")

    code = (
        "(event UserInterface|EventPreConstruct (IsDesignTime))\n\n"
        "(event UserInterface|EventConstruct\n"
        "  (bind _w self)\n"
        + "\n".join(lines)
        + ")\n\n"
        "(event UserInterface|EventTick (MyGeometry InDeltaTime))\n"
    )
    write = call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:EventGraph"}, "code": code})
    print("write:", write)
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:EventGraph"}})
    v = verify["returnValue"] if isinstance(verify, dict) else verify
    if "GetWidgetFromName" not in v:
        raise RuntimeError(f"Construct bind missing:\n{v}")
    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("HUD Construct bindings OK — stop PIE, then Play")


if __name__ == "__main__":
    main()
