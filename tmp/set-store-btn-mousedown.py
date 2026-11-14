import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

OB = "editor_toolset.toolsets.object.ObjectTools"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
UMG = "UMGToolSet.UMGToolSet"
sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()

for i in range(9):
    inst = {"refPath": f"{HUD}:WidgetTree.Btn_{i}"}
    try:
        call_tool(
            OB,
            "set_property",
            {
                "instance": inst,
                "property_name": "ClickMethod",
                "value": "DownAndUp",
            },
            sid,
        )
        print(f"Btn_{i} ClickMethod DownAndUp")
    except Exception as exc:
        print(f"Btn_{i}", exc)

call_tool(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}}, sid)
print("done")