"""Rebuild Btn_0..8 under StoreBox so BindWidget metadata regenerates."""
from __future__ import annotations

import json
import time
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
OB = "editor_toolset.toolsets.object.ObjectTools"
UMG = "UMGToolSet.UMGToolSet"
AT = "editor_toolset.toolsets.asset.AssetTools"

COSTS = [40, 30, 35, 20, 45, 30, 15, 50, 25]
SLUGS = [
    "monster_generator",
    "mana_generator",
    "prism_beam_trap",
    "resonance_plate_trap",
    "gravity_well_trap",
    "runaway_cart_trap",
    "loot_bait",
    "unshackled_golem",
    "shard_sentinel",
]
TOOLTIPS = [
    "Spawns defenders for your family. Place on an empty room square (not the core).",
    "Passive +1 mana/sec while placed in a powered room.",
    "Gem pillar that refracts a sapphire kill beam off mirrors.",
    "Quartz floor tile; harmonic shock staggers and shatters mundane armor.",
    "Reverses local gravity—pull intruders up, then drop them.",
    "Releases a mine cart down a built rail line to crush intruders.",
    "Open treasure bait; lures greedier heroes deeper into the maze.",
    "Floating obsidian defender anchored by a ley-energy core.",
    "Darting prism scout with a narrow searchlight cone.",
]


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
    text = json.loads(
        urllib.request.urlopen(
            urllib.request.Request(
                MCP_URL,
                data=body,
                headers={
                    "Content-Type": "application/json",
                    "Accept": "application/json, text/event-stream",
                    "Mcp-Session-Id": sid(),
                },
            ),
            timeout=300,
        ).read()
    )["result"]["content"][0]["text"]
    if text.startswith("Function") or text.startswith("Unknown"):
        raise RuntimeError(f"{ts}.{tool}: {text[:400]}")
    if not text.strip():
        return {}
    return json.loads(text)


def bp() -> dict:
    return {"refPath": HUD}


def wref(name: str) -> dict:
    return {"refPath": f"{HUD}:WidgetTree.{name}"}


def texture_ref(slug: str) -> str:
    found = call(
        AT,
        "find_assets",
        {"folder_path": "/Game/UI/Icons", "name": slug, "recursive": False},
    )["returnValue"]
    package = found[0]
    return f"{package}.{slug}"


def ensure_variable(widget_name: str) -> None:
    desc = call(UMG, "GetWidgetDescription", {"widgetBlueprint": bp()})
    widgets = desc["returnValue"]["widgets"]
    entry = next(w for w in widgets if w["widgetName"] == widget_name)
    ref = entry["widget"]
    if not entry.get("bIsVariable"):
        call(
            UMG,
            "ToggleWidgetAsVariable",
            {"widgetBlueprint": bp(), "widget": ref, "bIsVariable": True},
        )
    else:
        call(
            UMG,
            "ToggleWidgetAsVariable",
            {"widgetBlueprint": bp(), "widget": ref, "bIsVariable": False},
        )
        call(
            UMG,
            "ToggleWidgetAsVariable",
            {"widgetBlueprint": bp(), "widget": ref, "bIsVariable": True},
        )


def rebuild_button(index: int) -> None:
    btn = f"Btn_{index}"
    row = f"Row_{index}"
    icon = f"Icon_{index}"
    cost = f"Cost_{index}"

    for name in (icon, cost, row, btn):
        try:
            call(UMG, "RemoveWidget", {"widgetBlueprint": bp(), "widget": wref(name)})
        except RuntimeError:
            pass
        time.sleep(0.05)

    call(
        UMG,
        "AddWidget",
        {
            "widgetBlueprint": bp(),
            "parentWidget": wref("StoreBox"),
            "widgetClass": {"refPath": "/Script/UMG.Button"},
            "widgetDisplayName": btn,
        },
    )
    ensure_variable(btn)

    call(
        UMG,
        "AddWidget",
        {
            "widgetBlueprint": bp(),
            "parentWidget": wref(btn),
            "widgetClass": {"refPath": "/Script/UMG.HorizontalBox"},
            "widgetDisplayName": row,
        },
    )
    call(
        UMG,
        "AddWidget",
        {
            "widgetBlueprint": bp(),
            "parentWidget": wref(row),
            "widgetClass": {"refPath": "/Script/UMG.Image"},
            "widgetDisplayName": icon,
        },
    )
    call(
        UMG,
        "AddWidget",
        {
            "widgetBlueprint": bp(),
            "parentWidget": wref(row),
            "widgetClass": {"refPath": "/Script/UMG.TextBlock"},
            "widgetDisplayName": cost,
        },
    )

    tex = texture_ref(SLUGS[index])
    brush = {
        "brush": {
            "drawAs": "Image",
            "imageType": "FullColor",
            "imageSize": {"x": 48, "y": 48},
            "resourceObject": {"refPath": tex},
            "tintColor": {
                "specifiedColor": {"r": 1, "g": 1, "b": 1, "a": 1},
                "colorUseRule": "UseColor_Specified",
            },
        }
    }
    call(OB, "set_properties", {"instance": wref(icon), "values": json.dumps(brush)})

    cost_vals = {
        "text": f"{COSTS[index]} mana",
        "font": {"size": 13},
        "colorAndOpacity": {
            "specifiedColor": {"r": 0.72, "g": 0.85, "b": 1.0, "a": 1.0},
            "colorUseRule": "UseColor_Specified",
        },
        "visibility": "SelfHitTestInvisible",
    }
    call(OB, "set_properties", {"instance": wref(cost), "values": json.dumps(cost_vals)})

    btn_vals = {
        "pressMethod": "DownAndUp",
        "clickMethod": "DownAndUp",
        "visibility": "Visible",
        "bIsEnabled": True,
        "isFocusable": False,
        "toolTipText": TOOLTIPS[index],
        "backgroundColor": {"r": 0.05, "g": 0.08, "b": 0.16, "a": 1.0},
    }
    call(OB, "set_properties", {"instance": wref(btn), "values": json.dumps(btn_vals)})
    call(
        OB,
        "set_properties",
        {
            "instance": wref(row),
            "values": json.dumps({"visibility": "SelfHitTestInvisible"}),
        },
    )


def main() -> None:
    for i in range(9):
        print(f"rebuild Btn_{i}")
        rebuild_button(i)

    for name in ["Bar_Mana", "Txt_Mana", "Txt_Status"]:
        ensure_variable(name)

    call(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": bp()})
    call(AT, "save_assets", {"asset_paths": []})
    print("Buttons rebuilt — STOP PIE, then Play")


if __name__ == "__main__":
    main()
