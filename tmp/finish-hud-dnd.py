"""Reparent HUD, add full-screen GridDropOverlay behind store."""
from __future__ import annotations

import json
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
CPP_PARENT = "/Script/DungeonCore.DungeonLegacyStoreHudWidget"
DROP_ZONE = "/Script/DungeonCore.DungeonStoreGridDropZone"


def reparent() -> None:
    out = call_tool(
        BT,
        "set_parent",
        {"blueprint": {"refPath": HUD}, "parent_class": {"refPath": CPP_PARENT}},
        SID,
    )
    print("set_parent", out)
    parent = call_tool(BT, "get_parent", {"blueprint": {"refPath": HUD}}, SID)
    print("parent", parent)


def ensure_overlay() -> None:
    widgets = call_tool(UMG, "GetWidgets", {"widgetBlueprint": {"refPath": HUD}}, SID)["returnValue"][
        "widgets"
    ]
    has = any(w["widgetName"] == "GridDropOverlay" for w in widgets)
    if not has:
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
        call_tool(
            UMG,
            "ToggleWidgetAsVariable",
            {
                "widgetBlueprint": {"refPath": HUD},
                "widget": {"refPath": f"{HUD}:WidgetTree.GridDropOverlay"},
                "bIsVariable": True,
            },
            SID,
        )

    widgets = call_tool(UMG, "GetWidgets", {"widgetBlueprint": {"refPath": HUD}}, SID)["returnValue"][
        "widgets"
    ]
    slot_ref = None
    for w in widgets:
        if w.get("widgetName") == "GridDropOverlay":
            slot_ref = w.get("slot")
            break
    if slot_ref is None:
        raise RuntimeError("GridDropOverlay slot missing")

    layout = {
        "LayoutData": {
            "Offsets": {"Left": 0.0, "Top": 0.0, "Right": 0.0, "Bottom": 0.0},
            "Anchors": {
                "Minimum": {"X": 0.0, "Y": 0.0},
                "Maximum": {"X": 1.0, "Y": 1.0},
            },
            "Alignment": {"X": 0.0, "Y": 0.0},
        },
        "ZOrder": 0,
    }
    call_tool(
        OB,
        "set_properties",
        {"instance": slot_ref, "values": json.dumps(layout)},
        SID,
    )
    try:
        call_tool(
            UMG,
            "MoveWidget",
            {
                "widgetBlueprint": {"refPath": HUD},
                "widget": {"refPath": f"{HUD}:WidgetTree.GridDropOverlay"},
                "newParent": {"refPath": f"{HUD}:WidgetTree.Root"},
                "insertIndex": 0,
            },
            SID,
        )
    except RuntimeError as exc:
        print("MoveWidget", exc)

    call_tool(
        OB,
        "set_properties",
        {
            "instance": {"refPath": f"{HUD}:WidgetTree.GridDropOverlay"},
            "values": json.dumps(
                {
                    "Visibility": "Visible",
                    "RenderOpacity": 0.01,
                    "Clipping": "Inherit",
                }
            ),
        },
        SID,
    )
    print("overlay ready")


def main() -> None:
    if call_tool(APP, "IsPIERunning", {}, SID).get("returnValue"):
        call_tool(APP, "StopPIE", {}, SID)
    reparent()
    ensure_overlay()
    call_tool(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}}, SID)
    call_tool(AT, "save_assets", {"asset_paths": [HUD]}, SID)
    print("HUD DnD ready — re-patch Live Coding if you changed C++ again, then PIE.")


if __name__ == "__main__":
    main()