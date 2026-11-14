"""Fix WBP_DungeonHUD compiler errors (Highlight missing Btn targets + restore store fns)."""
from __future__ import annotations

import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"


def call(ts: str, tool: str, args: dict) -> dict:
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
    raw = json.loads(
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
    if raw.startswith("AssertionError") or raw.startswith("RuntimeError"):
        raise RuntimeError(raw[:1500])
    if not raw.strip().startswith("{"):
        raise RuntimeError(raw[:500])
    return json.loads(raw)


def item_fn(name: str, check: str) -> str:
    inner = "-1"
    for i in reversed(range(9)):
        inner = f"(select ({check} (Variables|WBP_DungeonHUD|GetBtn_{i})) {i} {inner})"
    return f"(fn {name} ()\n  (return {inner}))"


def highlight_fn() -> str:
    lines = [
        "(fn Highlight (Hovered Selected)",
        "  (bind _linearcolor (Utilities|Struct|MakeLinearColor 0.05 0.08 0.16 1.0))",
        "  (bind _linearcolor_1 (Utilities|Struct|MakeLinearColor 0.2 0.34 0.65 1.0))",
        "  (bind _linearcolor_2 (Utilities|Struct|MakeLinearColor 0.85 0.55 0.1 1.0))",
    ]
    for i in range(9):
        lines.append(
            f"  (Button|Appearance|SetBackgroundColor (Variables|WBP_DungeonHUD|GetBtn_{i}) "
            f"(select (== Selected {i}) _linearcolor_2 (select (== Hovered {i}) _linearcolor_1 _linearcolor)))"
        )
    lines.append(")")
    return "\n".join(lines)


def main() -> None:
    call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:Highlight"}, "code": "(fn Highlight (Hovered Selected))"})
    for name, check in [("GetHoveredItem", "Widget|IsHovered"), ("GetPressedItem", "Button|IsPressed")]:
        call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:{name}"}, "code": item_fn(name, check)})
    call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:Highlight"}, "code": highlight_fn()})
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:Highlight"}})["returnValue"]
    if "GetBtn_0" not in verify or "(select (== Selected 0)" not in verify:
        raise RuntimeError(f"Highlight verify failed:\n{verify}")
    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("WBP_DungeonHUD compile OK")


if __name__ == "__main__":
    main()
