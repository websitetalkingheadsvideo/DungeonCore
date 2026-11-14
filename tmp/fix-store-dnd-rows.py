"""Fix store drag (pass-through children) and icon + cost rows in WBP_DungeonHUD."""
from __future__ import annotations

import json
import time
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"

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
COSTS = [40, 30, 35, 20, 45, 30, 15, 50, 25]
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
    if not text.strip():
        return {}
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return {"raw": text}


def wref(name: str) -> dict:
    return {"refPath": f"{HUD}:WidgetTree.{name}"}


def bp() -> dict:
    return {"refPath": HUD}


def widget_names() -> set[str]:
    desc = call_json(
        "UMGToolSet.UMGToolSet",
        "GetWidgetDescription",
        {"widgetBlueprint": bp()},
    )
    return {entry["widgetName"] for entry in desc["returnValue"]["widgets"]}


def texture_ref(slug: str) -> str:
    package = call_json(
        "editor_toolset.toolsets.asset.AssetTools",
        "find_assets",
        {"folder_path": "/Game/UI/Icons", "name": slug, "recursive": False},
    )["returnValue"][0]
    return f"{package}.{slug}"


def ensure_row(index: int) -> None:
    btn = f"Btn_{index}"
    row = f"Row_{index}"
    icon = f"Icon_{index}"
    cost = f"Cost_{index}"
    names = widget_names()

    if icon in names and row not in names:
        call_json(
            "UMGToolSet.UMGToolSet",
            "RemoveWidget",
            {"widgetBlueprint": bp(), "widget": wref(icon)},
        )
        names.discard(icon)

    if row not in widget_names():
        call_json(
            "UMGToolSet.UMGToolSet",
            "AddWidget",
            {
                "widgetBlueprint": bp(),
                "parentWidget": wref(btn),
                "widgetClass": {"refPath": "/Script/UMG.HorizontalBox"},
                "widgetDisplayName": row,
            },
        )

    names = widget_names()
    if icon not in names:
        call_json(
            "UMGToolSet.UMGToolSet",
            "AddWidget",
            {
                "widgetBlueprint": bp(),
                "parentWidget": wref(row),
                "widgetClass": {"refPath": "/Script/UMG.Image"},
                "widgetDisplayName": icon,
            },
        )
    if cost not in widget_names():
        call_json(
            "UMGToolSet.UMGToolSet",
            "AddWidget",
            {
                "widgetBlueprint": bp(),
                "parentWidget": wref(row),
                "widgetClass": {"refPath": "/Script/UMG.TextBlock"},
                "widgetDisplayName": cost,
            },
        )

    tex = texture_ref(SLUGS[index])
    call_json(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {
            "instance": wref(icon),
            "values": json.dumps(
                {
                    "Brush": {
                        "DrawAs": "Image",
                        "ImageType": "FullColor",
                        "ImageSize": {"X": 48, "Y": 48},
                        "ResourceObject": {"refPath": tex},
                    },
                    "Visibility": "SelfHitTestInvisible",
                }
            ),
        },
    )
    call_json(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {
            "instance": wref(cost),
            "values": json.dumps(
                {
                    "Text": f"{COSTS[index]} mana",
                    "Visibility": "SelfHitTestInvisible",
                    "Font": {"Size": 13},
                }
            ),
        },
    )
    call_json(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {
            "instance": wref(row),
            "values": json.dumps({"Visibility": "SelfHitTestInvisible"}),
        },
    )
    call_json(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {
            "instance": wref(btn),
            "values": json.dumps(
                {
                    "ToolTipText": TOOLTIPS[index],
                    "BackgroundColor": {"R": 0.05, "G": 0.08, "B": 0.16, "A": 1.0},
                    "PressMethod": "ButtonPress",
                }
            ),
        },
    )


def main() -> None:
    import sys

    start = int(sys.argv[1]) if len(sys.argv) > 1 else 0
    for index in range(start, 9):
        print(f"row {index}")
        ensure_row(index)
        time.sleep(0.12)
    call_json("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": bp()})
    call_json("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": []})
    print("saved")


if __name__ == "__main__":
    main()
