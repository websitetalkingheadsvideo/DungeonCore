"""Route store/mana UI through StoreBox/ManaBox children (fixes null Btn_* at PIE)."""
from __future__ import annotations

import json
import re
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"

EXTRA_VARS = ["StoreBox", "ManaBox"]


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


def btn_child(idx: int) -> str:
    # StoreBox children: 0 title, 1 status, 2..10 store buttons
    return f"(UserInterface|CastButton (Widget|GetChildAt (Variables|WBP_DungeonHUD|GetStoreBox) {idx + 2}))"


def pressed_select_tail() -> str:
    inner = "-1"
    for i in reversed(range(9)):
        inner = f"(select (Button|IsPressed {btn_child(i)}) {i} {inner})"
    return inner


def hovered_select_tail() -> str:
    inner = "-1"
    for i in reversed(range(9)):
        inner = f"(select (Widget|IsHovered {btn_child(i)}) {i} {inner})"
    return inner


def get_pressed_item_dsl() -> str:
    pressed = pressed_select_tail()
    return f"""(fn GetPressedItem ()
  (bind _pc (Game|GetPlayerController 0))
  (bind _pressed {pressed})
  (bind _latched (Variables|Default|GetDragLatch))
  (bind _updated (select (>= _pressed 0) _pressed (select (Game|Player|WasInputKeyJustReleased _pc "LeftMouseButton") -1 _latched)))
  (Variables|Default|SetDragLatch _updated)
  (return _updated))"""


def get_hovered_item_dsl() -> str:
    return f"""(fn GetHoveredItem ()
  (return {hovered_select_tail()}))"""


def highlight_dsl() -> str:
    lines = [
        "(fn Highlight (Hovered Selected)",
        "  (bind _linearcolor (Utilities|Struct|MakeLinearColor 0.05 0.08 0.16 1.0))",
        "  (bind _linearcolor_1 (Utilities|Struct|MakeLinearColor 0.2 0.34 0.65 1.0))",
        "  (bind _linearcolor_2 (Utilities|Struct|MakeLinearColor 0.85 0.55 0.1 1.0))",
    ]
    for i in range(9):
        b = btn_child(i)
        lines.append(
            f"  (Button|Appearance|SetBackgroundColor {b} "
            f"(select (== Selected {i}) _linearcolor_2 (select (== Hovered {i}) _linearcolor_1 _linearcolor)))"
        )
    lines.append(")")
    return "\n".join(lines)


def update_mana_dsl() -> str:
    return """(fn UpdateMana (Mana MaxMana)
  (Progress|SetPercent (UserInterface|CastProgressBar (Widget|GetChildAt (Variables|WBP_DungeonHUD|GetManaBox) 1)) (/ Mana MaxMana))
  (Widget|SetText(Text) (UserInterface|CastTextBlock (Widget|GetChildAt (Variables|WBP_DungeonHUD|GetManaBox) 0)) (Utilities|Text|ToText(String) (Utilities|String|Append "Mana  " (Utilities|String|Append (Utilities|String|ToString(Integer) (Math|Float|Floor Mana)) (Utilities|String|Append " / " (Utilities|String|ToString(Integer) (Math|Float|Floor MaxMana))))))))"""


def set_status_dsl() -> str:
    return """(fn SetStatus (Msg)
  (Widget|SetText(Text) (UserInterface|CastTextBlock (Widget|GetChildAt (Variables|WBP_DungeonHUD|GetStoreBox) 1)) (Utilities|Text|ToText(String) Msg)))"""


def toggle_var(widget_name: str) -> None:
    bp = {"refPath": HUD}
    raw = call("UMGToolSet.UMGToolSet", "GetWidgetDescription", {"widgetBlueprint": bp})
    desc = raw if isinstance(raw, dict) else json.loads(raw)
    rv = desc.get("returnValue", desc)
    if isinstance(rv, str):
        rv = json.loads(rv)
    widgets = rv["widgets"] if isinstance(rv, dict) else rv
    by_name = {w["widgetName"]: w for w in widgets}
    if widget_name not in by_name:
        raise RuntimeError(f"missing widget {widget_name}")
    w = by_name[widget_name]
    ref = w["widget"]
    if not w.get("bIsVariable"):
        call(
            "UMGToolSet.UMGToolSet",
            "ToggleWidgetAsVariable",
            {"widgetBlueprint": bp, "widget": ref, "bIsVariable": True},
        )
    print(f"variable {widget_name} OK")


def write_graph(name: str, code: str) -> None:
    out = call(BT, "write_graph_dsl", {"graph": {"refPath": f"{HUD}:{name}"}, "code": code})
    if isinstance(out, dict) and out.get("returnValue") is None:
        pass
    verify = call(BT, "read_graph_dsl", {"graph": {"refPath": f"{HUD}:{name}"}})["returnValue"]
    if "GetStoreBox" not in verify and name in {"GetPressedItem", "GetHoveredItem", "Highlight", "SetStatus"}:
        raise RuntimeError(f"{name} patch failed:\n{verify}")
    if "GetManaBox" not in verify and name == "UpdateMana":
        raise RuntimeError(f"{name} patch failed:\n{verify}")
    print(f"wrote {name}")


def main() -> None:
    vars_out = call(BT, "list_variables", {"blueprint": {"refPath": HUD}})
    names = vars_out.get("returnValue", [])
    if "DragLatch" not in names:
        call(BT, "add_variable", {"blueprint": {"refPath": HUD}, "name": "DragLatch", "type_name": "int"})

    for v in EXTRA_VARS:
        toggle_var(v)

    write_graph("GetPressedItem", get_pressed_item_dsl())
    write_graph("GetHoveredItem", get_hovered_item_dsl())
    write_graph("Highlight", highlight_dsl())
    write_graph("UpdateMana", update_mana_dsl())
    write_graph("SetStatus", set_status_dsl())

    call("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}})
    call("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("StoreBox/ManaBox refs OK — STOP PIE, then Play. Hold store row, drag, release LMB.")


if __name__ == "__main__":
    main()
