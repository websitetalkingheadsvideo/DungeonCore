"""TryDig: clearer low-mana text (status line is pushed via TickStore SetStatus)."""
from __future__ import annotations

import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

MANA_MSG = """(Utilities|String|Append "Not enough mana to dig (needs " (Utilities|String|Append (Utilities|String|ToString(Integer) (Variables|Default|GetDigCost)) " mana)"))"""

dsl = call_tool(BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TryDig"}}, SID)["returnValue"]
patched = dsl.replace(
    '(Variables|Default|SetStatusMsg "Not enough mana to dig")',
    f"(Variables|Default|SetStatusMsg {MANA_MSG})",
    1,
)
if patched != dsl:
    print(call_tool(BT, "write_graph_dsl", {"graph": {"refPath": f"{GRID}:TryDig"}, "code": patched}, SID))
call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, SID)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, SID)
print("OK")