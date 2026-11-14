"""Wire UMG store drag-and-drop: grid BP funcs, TickStore, HUD parent + overlay."""
from __future__ import annotations

import re
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
UMG = "UMGToolSet.UMGToolSet"
OB = "editor_toolset.toolsets.object.ObjectTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
APP = "EditorToolset.EditorAppToolset"

HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
CPP_PARENT = "/Script/DungeonCore.DungeonLegacyStoreHudWidget"
DROP_ZONE = "/Script/DungeonCore.DungeonStoreGridDropZone"

GRID_DSL = open(r"e:\Dungeon Core\tmp\store-umg-dnd-grid.dsl", encoding="utf-8").read()
TICK = open(r"e:\Dungeon Core\tmp\tickstore-umg-dnd.dsl", encoding="utf-8").read().strip()


def split_fns(text: str) -> dict[str, str]:
    parts = re.split(r"(?=\(fn )", text)
    out: dict[str, str] = {}
    for part in parts:
        part = part.strip()
        if not part.startswith("(fn "):
            continue
        name = part.split()[1].rstrip("(")
        if name.startswith(";"):
            continue
        out[name] = part
    return out


def reparent_hud() -> None:
    out = call_tool(
        BT,
        "set_parent",
        {"blueprint": {"refPath": HUD}, "parent_class": {"refPath": CPP_PARENT}},
        SID,
    )
    print("set_parent", out)


def ensure_overlay() -> None:
    widgets = call_tool(UMG, "GetWidgets", {"widgetBlueprint": {"refPath": HUD}}, SID)["returnValue"][
        "widgets"
    ]
    if any(w["widgetName"] == "GridDropOverlay" for w in widgets):
        print("GridDropOverlay already present")
        return

    call_tool(
        UMG,
        "AddWidget",
        {
            "widgetBlueprint": {"refPath": HUD},
            "parentWidget": {"refPath": f"{HUD}:WidgetTree.Root"},
            "widgetClass": {"refPath": DROP_ZONE},
            "widgetDisplayName": "GridDropOverlay",
        },
        SID,
    )
    wref = {"refPath": f"{HUD}:WidgetTree.GridDropOverlay"}
    call_tool(
        UMG,
        "ToggleWidgetAsVariable",
        {"widgetBlueprint": {"refPath": HUD}, "widget": wref, "bIsVariable": True},
        SID,
    )

    desc = call_tool(UMG, "GetWidgetDescription", {"widgetBlueprint": {"refPath": HUD}}, SID)
    slot_ref = None
    for w in desc["returnValue"]["widgets"]:
        if w.get("widgetName") == "GridDropOverlay":
            slot_ref = w.get("slot")
            break
    if slot_ref:
        props = call_tool(OB, "list_properties", {"instance": slot_ref}, SID)
        print("overlay slot props sample", str(props)[:200])


def main() -> None:
    if call_tool(APP, "IsPIERunning", {}, SID).get("returnValue"):
        call_tool(APP, "StopPIE", {}, SID)

    for name, code in split_fns(GRID_DSL).items():
        gref = {"refPath": f"{GRID}:{name}"}
        call_tool(BT, "add_function_graph", {"blueprint": {"refPath": GRID}, "graph_name": name}, SID)
        def add_param(param_name: str, param_type: str, input_param: bool) -> None:
            try:
                call_tool(
                    BT,
                    "add_function_param",
                    {
                        "graph": gref,
                        "param_name": param_name,
                        "param_type": param_type,
                        "input_param": input_param,
                    },
                    SID,
                )
            except RuntimeError as exc:
                if "already exists" not in str(exc):
                    raise

        if name == "ArmStoreDrag":
            add_param("ItemIndex", "int", True)
            add_param("ReturnValue", "bool", False)
        if name == "CommitStoreDrag":
            add_param("ReturnValue", "bool", False)
        w = call_tool(BT, "write_graph_dsl", {"graph": gref, "code": code.strip()}, SID)
        print(name, w)

    call_tool(
        BT,
        "write_graph_dsl",
        {"graph": {"refPath": f"{GRID}:TickStore"}, "code": TICK},
        SID,
    )

    reparent_hud()
    ensure_overlay()

    call_tool(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}}, SID)
    call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, SID)
    call_tool(AT, "save_assets", {"asset_paths": [HUD, GRID]}, SID)
    print("Grid + HUD base wiring done.")
    print("If rows do not drag yet: each Btn_N needs OnDragDetected -> CreateStoreDragOperation(N).")


if __name__ == "__main__":
    main()