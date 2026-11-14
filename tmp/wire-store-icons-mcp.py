"""Wire store icons + tooltips into WBP_DungeonHUD via Unreal MCP HTTP."""
from __future__ import annotations

import json
import urllib.request
from pathlib import Path

SID_PATH = Path(r"e:\Dungeon Core\tmp\mcp-sid.txt")
MCP_URL = "http://127.0.0.1:8000/mcp"
HUD_BP = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
ICON_DIR = Path(r"E:\Dungeon Core\DungeonCore\Content\UI\Icons")

STORE_ITEMS = [
    {
        "btn": "Btn_0",
        "txt": "BtnTxt_0",
        "img": "Icon_0",
        "file": "monster_generator.png",
        "asset": "/Game/UI/Icons/monster_generator",
        "tooltip": "Spawns defenders for your family. Place on an empty room square (not the core).",
    },
    {
        "btn": "Btn_1",
        "txt": "BtnTxt_1",
        "img": "Icon_1",
        "file": "mana_generator.png",
        "asset": "/Game/UI/Icons/mana_generator",
        "tooltip": "Passive +1 mana/sec while placed in a powered room.",
    },
    {
        "btn": "Btn_2",
        "txt": "BtnTxt_2",
        "img": "Icon_2",
        "file": "prism_beam_trap.png",
        "asset": "/Game/UI/Icons/prism_beam_trap",
        "tooltip": "Gem pillar that refracts a sapphire kill beam off mirrors.",
    },
    {
        "btn": "Btn_3",
        "txt": "BtnTxt_3",
        "img": "Icon_3",
        "file": "resonance_plate_trap.png",
        "asset": "/Game/UI/Icons/resonance_plate_trap",
        "tooltip": "Quartz floor tile; harmonic shock staggers and shatters mundane armor.",
    },
    {
        "btn": "Btn_4",
        "txt": "BtnTxt_4",
        "img": "Icon_4",
        "file": "gravity_well_trap.png",
        "asset": "/Game/UI/Icons/gravity_well_trap",
        "tooltip": "Reverses local gravity—pull intruders up, then drop them.",
    },
    {
        "btn": "Btn_5",
        "txt": "BtnTxt_5",
        "img": "Icon_5",
        "file": "runaway_cart_trap.png",
        "asset": "/Game/UI/Icons/runaway_cart_trap",
        "tooltip": "Releases a mine cart down a built rail line to crush intruders.",
    },
    {
        "btn": "Btn_6",
        "txt": "BtnTxt_6",
        "img": "Icon_6",
        "file": "loot_bait.png",
        "asset": "/Game/UI/Icons/loot_bait",
        "tooltip": "Open treasure bait; lures greedier heroes deeper into the maze.",
    },
    {
        "btn": "Btn_7",
        "txt": "BtnTxt_7",
        "img": "Icon_7",
        "file": "unshackled_golem.png",
        "asset": "/Game/UI/Icons/unshackled_golem",
        "tooltip": "Floating obsidian defender anchored by a ley-energy core.",
    },
    {
        "btn": "Btn_8",
        "txt": "BtnTxt_8",
        "img": "Icon_8",
        "file": "shard_sentinel.png",
        "asset": "/Game/UI/Icons/shard_sentinel",
        "tooltip": "Darting prism scout with a narrow searchlight cone.",
    },
]


def sid() -> str:
    return SID_PATH.read_text(encoding="utf-8").strip()


def mcp_call(method: str, params: dict) -> str:
    body = json.dumps({"jsonrpc": "2.0", "id": 1, "method": method, "params": params}).encode()
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
    return raw["result"]["content"][0]["text"]


def call_tool(toolset: str, tool: str, arguments: dict) -> str:
    return mcp_call(
        "tools/call",
        {
            "name": "call_tool",
            "arguments": {
                "toolset_name": toolset,
                "tool_name": tool,
                "arguments": arguments,
            },
        },
    )


def call_tool_json(toolset: str, tool: str, arguments: dict) -> dict:
    text = call_tool(toolset, tool, arguments)
    if text.startswith("Function") or text.startswith("Unknown"):
        raise RuntimeError(text)
    return json.loads(text)


def build_programmatic_script() -> str:
  # Script runs inside UE programmatic sandbox
    items_literal = json.dumps(STORE_ITEMS)
    return f'''
import json

HUD_BP = {json.dumps(HUD_BP)}
ITEMS = json.loads({json.dumps(items_literal)})

def ref(name):
    return {{"refPath": f"{{HUD_BP}}:WidgetTree.{{name}}"}}

def cls(path):
    return {{"refPath": path}}

def run():
    results = {{"imported": [], "buttons": []}}
    asset_tools = api("editor_toolset.toolsets.asset.AssetTools")
    umg = api("UMGToolSet.UMGToolSet")
    obj = api("editor_toolset.toolsets.object.ObjectTools")

    asset_tools.create_folder("/Game/UI/Icons")
    import_settings = {{
        "factory_name": "TextureFactory",
        "files": [],
        "destination_path": "/Game/UI",
        "destination_name": "unused",
    }}
    for item in ITEMS:
        disk_path = r"E:/Dungeon Core/DungeonCore/Content/UI/Icons/" + item["file"]
        dest = "/Game/UI/Icons/" + item["file"].replace(".png", "")
        import_settings["files"].append({{"source_path": disk_path, "destination_path": dest}})
    # Fallback: import via unreal python if asset missing
    for item in ITEMS:
        dest = item["asset"]
        if not asset_tools.exists(dest):
            try:
                import unreal
                task = unreal.AssetImportTask()
                task.filename = r"E:/Dungeon Core/DungeonCore/Content/UI/Icons/" + item["file"]
                task.destination_path = "/Game/UI/Icons"
                task.destination_name = item["file"].replace(".png", "")
                task.replace_existing = True
                task.automated = True
                task.save = True
                unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
                results["imported"].append(dest)
            except Exception as exc:
                results.setdefault("import_errors", []).append({{ "asset": dest, "error": str(exc) }})

    bp = {{"refPath": HUD_BP}}
    image_cls = cls("/Script/UMG.Image")

    for item in ITEMS:
        btn_ref = ref(item["btn"])
        txt_ref = ref(item["txt"])
        try:
            umg.RemoveWidget(bp, txt_ref)
        except Exception:
            pass

        add = umg.AddWidget(
            bp,
            btn_ref,
            image_cls,
            item["img"],
        )
        img_info = add["returnValue"] if isinstance(add, dict) and "returnValue" in add else add
        img_ref = img_info["widget"] if isinstance(img_info, dict) else img_info[0]["widget"]

        tex = cls(item["asset"] + "." + item["asset"].split("/")[-1])
        props = obj.list_properties(img_ref)
        prop_names = [p["name"] for p in props["returnValue"]] if isinstance(props, dict) and "returnValue" in props else []
        brush_props = {{}}
        if "Brush" in prop_names:
            brush_props["Brush"] = {{
                "ImageType": 0,
                "ImageSize": {{"X": 56.0, "Y": 56.0}},
                "ResourceObject": tex,
            }}
        if "bIsEnabled" in prop_names:
            brush_props["bIsEnabled"] = True
        obj.set_properties(img_ref, brush_props)

        btn_props = obj.list_properties(btn_ref)
        btn_names = [p["name"] for p in btn_props["returnValue"]] if isinstance(btn_props, dict) and "returnValue" in btn_props else []
        btn_values = {{}}
        if "ToolTipText" in btn_names:
            btn_values["ToolTipText"] = item["tooltip"]
        if "BackgroundColor" in btn_names:
            btn_values["BackgroundColor"] = {{"R": 0.02, "G": 0.03, "B": 0.06, "A": 0.85}}
        obj.set_properties(btn_ref, btn_values)

        slot = img_info.get("slot") if isinstance(img_info, dict) else None
        if slot:
            slot_props = obj.list_properties(slot)
            slot_names = [p["name"] for p in slot_props["returnValue"]] if isinstance(slot_props, dict) and "returnValue" in slot_props else []
            slot_values = {{}}
            if "Padding" in slot_names:
                slot_values["Padding"] = {{"Left": 6, "Top": 6, "Right": 6, "Bottom": 6}}
            if "HorizontalAlignment" in slot_names:
                slot_values["HorizontalAlignment"] = "HAlign_Center"
            if "VerticalAlignment" in slot_names:
                slot_values["VerticalAlignment"] = "VAlign_Center"
            obj.set_properties(slot, slot_values)

        results["buttons"].append(item["btn"])

    umg.CompileWidgetBlueprint(bp)
    asset_tools.save_assets([HUD_BP])
    return results
'''


def main() -> None:
    env = call_tool_json(
        "editor_toolset.toolsets.programmatic.ProgrammaticToolset",
        "get_execution_environment",
        {},
    )
    print("env language:", env["returnValue"]["language"])

    script = build_programmatic_script()
    result_text = call_tool(
        "editor_toolset.toolsets.programmatic.ProgrammaticToolset",
        "execute_tool_script",
        {"script": script},
    )
    print(result_text)


if __name__ == "__main__":
    main()
