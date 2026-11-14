"""Import store icons and convert WBP_DungeonHUD buttons to icon + tooltip rows."""
from __future__ import annotations

import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
HUD = "/Game/UI/WBP_DungeonHUD"
HUD_REF = f"{HUD}.WBP_DungeonHUD"
ICON_SRC = r"E:/Dungeon Core/Design/Reference Images/icons"

ITEMS = [
    ("Btn_0", "BtnTxt_0", "Icon_0", "monster_generator", "Spawns defenders for your family. Place on an empty room square (not the core)."),
    ("Btn_1", "BtnTxt_1", "Icon_1", "mana_generator", "Passive +1 mana/sec while placed in a powered room."),
    ("Btn_2", "BtnTxt_2", "Icon_2", "prism_beam_trap", "Gem pillar that refracts a sapphire kill beam off mirrors."),
    ("Btn_3", "BtnTxt_3", "Icon_3", "resonance_plate_trap", "Quartz floor tile; harmonic shock staggers and shatters mundane armor."),
    ("Btn_4", "BtnTxt_4", "Icon_4", "gravity_well_trap", "Reverses local gravity—pull intruders up, then drop them."),
    ("Btn_5", "BtnTxt_5", "Icon_5", "runaway_cart_trap", "Releases a mine cart down a built rail line to crush intruders."),
    ("Btn_6", "BtnTxt_6", "Icon_6", "loot_bait", "Open treasure bait; lures greedier heroes deeper into the maze."),
    ("Btn_7", "BtnTxt_7", "Icon_7", "unshackled_golem", "Floating obsidian defender anchored by a ley-energy core."),
    ("Btn_8", "BtnTxt_8", "Icon_8", "shard_sentinel", "Darting prism scout with a narrow searchlight cone."),
]


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def call_tool(toolset: str, tool: str, arguments: dict) -> str:
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
    raw = json.loads(urllib.request.urlopen(req, timeout=300).read().decode())
    text = raw["result"]["content"][0]["text"]
    if text.startswith("Function") or text.startswith("Unknown"):
        raise RuntimeError(f"{toolset}.{tool}: {text}")
    return text


def call_json(toolset: str, tool: str, arguments: dict) -> dict:
    text = call_tool(toolset, tool, arguments)
    if not text.strip():
        raise RuntimeError(f"{toolset}.{tool} returned empty response")
    return json.loads(text)


def widget_ref(name: str) -> dict:
    return {"refPath": f"{HUD_REF}:WidgetTree.{name}"}


def texture_object_ref(package_path: str) -> str:
    asset_name = package_path.rsplit("/", 1)[-1]
    return f"{package_path}.{asset_name}"


def texture_ref(slug: str) -> str:
    matches = call_json(
        "editor_toolset.toolsets.asset.AssetTools",
        "find_assets",
        {"folder_path": "/Game/UI/Icons", "name": slug, "recursive": False},
    ).get("returnValue", [])
    if matches:
        return matches[0]
    return f"/Game/UI/Icons/{slug}"


def import_texture(slug: str) -> str:
    ref = texture_ref(slug)
    if call_json("editor_toolset.toolsets.asset.AssetTools", "exists", {"path": ref}).get("returnValue"):
        return texture_object_ref(ref)
    text = call_tool(
        "editor_toolset.toolsets.texture.TextureTools",
        "import_file",
        {
            "folder_path": "/Game/UI/Icons",
            "asset_name": slug,
            "source_file": f"{ICON_SRC}/{slug}.png",
        },
    )
    if not text.strip():
        return ref
    try:
        result = json.loads(text)
    except json.JSONDecodeError:
        return ref
    return texture_object_ref(result["returnValue"][0]["refPath"])


def widget_exists(name: str) -> bool:
    tree = call_json(
        "UMGToolSet.UMGToolSet",
        "GetWidgetDescription",
        {"widgetBlueprint": {"refPath": HUD_REF}},
    )
    for entry in tree["returnValue"]["widgets"]:
        if entry.get("widgetName") == name:
            return True
    return False


def ensure_icon_widget(btn: str, txt: str, icon_name: str) -> dict:
    bp = {"refPath": HUD_REF}
    if widget_exists(txt):
        call_json(
            "UMGToolSet.UMGToolSet",
            "RemoveWidget",
            {"widgetBlueprint": bp, "widget": widget_ref(txt)},
        )
    if widget_exists(icon_name):
        return widget_ref(icon_name)
    add = call_json(
        "UMGToolSet.UMGToolSet",
        "AddWidget",
        {
            "widgetBlueprint": bp,
            "parentWidget": widget_ref(btn),
            "widgetClass": {"refPath": "/Script/UMG.Image"},
            "widgetDisplayName": icon_name,
        },
    )
    return add["returnValue"]["widget"]


def main() -> None:
    call_tool("editor_toolset.toolsets.asset.AssetTools", "create_folder", {"path": "/Game/UI/Icons"})

    for btn, txt, icon_name, slug, tooltip in ITEMS:
        print(f"-- {slug}")
        tex_ref = import_texture(slug)
        icon_ref = ensure_icon_widget(btn, txt, icon_name)
        call_json(
            "editor_toolset.toolsets.object.ObjectTools",
            "set_properties",
            {
                "instance": icon_ref,
                "values": json.dumps(
                    {
                        "brush": {
                            "drawAs": "Image",
                            "imageType": "FullColor",
                            "imageSize": {"x": 56, "y": 56},
                            "resourceObject": {"refPath": tex_ref},
                            "tintColor": {
                                "specifiedColor": {"r": 1, "g": 1, "b": 1, "a": 1},
                                "colorUseRule": "UseColor_Specified",
                            },
                        }
                    }
                ),
            },
        )
        call_json(
            "editor_toolset.toolsets.object.ObjectTools",
            "set_properties",
            {
                "instance": widget_ref(btn),
                "values": json.dumps({"toolTipText": tooltip}),
            },
        )
        print(f"OK {btn} -> {slug}")

    call_json("UMGToolSet.UMGToolSet", "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD_REF}})
    saved = call_json("editor_toolset.toolsets.asset.AssetTools", "save_assets", {"asset_paths": [HUD]})
    print("saved", saved)


if __name__ == "__main__":
    main()
