"""TryDig: show status when blocked by insufficient mana."""
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool, new_session

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

OLD = """        (if (>= (Variables|Default|GetMana) (Variables|Default|GetDigCost))
          (Variables|Default|SetMana (- (Variables|Default|GetMana) (Variables|Default|GetDigCost)))
          (Utilities|Array|SetArrayElem (Variables|Default|GetCells) _returnvalue_1 1)
          (CallFunction|RevealAround X Y)
          (Calibration|Rebuild))))))"""

NEW = """        (if (>= (Variables|Default|GetMana) (Variables|Default|GetDigCost))
          (Variables|Default|SetMana (- (Variables|Default|GetMana) (Variables|Default|GetDigCost)))
          (Utilities|Array|SetArrayElem (Variables|Default|GetCells) _returnvalue_1 1)
          (CallFunction|RevealAround X Y)
          (Calibration|Rebuild)
          (else
            (Variables|Default|SetStatusMsg (Utilities|String|Append "Not enough mana to dig - needs " (Utilities|String|ToString(Integer) (Variables|Default|GetDigCost))))
            (Variables|Default|SetMsgTimer 3.0)))))))"""

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
try:
    if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
        call_tool(APP, "StopPIE", {}, sid)
except Exception:
    sid = new_session()
    if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
        call_tool(APP, "StopPIE", {}, sid)

dsl = call_tool(
    BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TryDig"}}, sid
)["returnValue"]
if "Not enough mana to dig" in dsl:
    print("already patched")
else:
    if OLD not in dsl:
        raise RuntimeError(f"TryDig block not found:\n{dsl}")
    patched = dsl.replace(OLD, NEW, 1)
    if patched.count("(") != patched.count(")"):
        raise RuntimeError(
            f"parens {patched.count('(')} vs {patched.count(')')}"
        )
    out = call_tool(
        BT,
        "write_graph_dsl",
        {"graph": {"refPath": f"{GRID}:TryDig"}, "code": patched},
        sid,
    )
    print("write", out)

verify = call_tool(
    BT, "read_graph_dsl", {"graph": {"refPath": f"{GRID}:TryDig"}}, sid
)["returnValue"]
if "Not enough mana to dig" not in verify:
    raise RuntimeError(verify)

call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID]}, sid)
print("TryDig mana status OK")