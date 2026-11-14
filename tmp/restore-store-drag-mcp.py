"""Restore hold-drag-release: HUD bindings, TickStore hover, PendingPress feed."""
import subprocess
import sys

sys.path.insert(0, r"e:\Dungeon Core\tmp")
from mcp_session import call_tool

APP = "EditorToolset.EditorAppToolset"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
AT = "editor_toolset.toolsets.asset.AssetTools"
UMG = "UMGToolSet.UMGToolSet"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
TICK_GRAPH = {"refPath": f"{GRID}:TickStore"}

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
if call_tool(APP, "IsPIERunning", {}, sid).get("returnValue"):
    call_tool(APP, "StopPIE", {}, sid)

# --- TickStore: fix GetHoveredItem (grid self-call -> HUD) via pin surgery ---
hover_fn = call_tool(
    BT, "find_nodes", {"graph": TICK_GRAPH, "title": "GetHoveredItem"}, sid
).get("returnValue", [])
if hover_fn:
    hnode = hover_fn[0]["refPath"]
    hi = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": hnode}]}, sid)[
        "returnValue"
    ][0]
    # If target is Self not HUD, break Target pin and connect GetHUD output
    for inp in hi["input_pins"]:
        if inp["name"] == "self" and inp.get("connected_pins"):
            call_tool(
                BT,
                "break_pins",
                {
                    "output_pin": inp["connected_pins"][0],
                    "input_pin": inp["pin_id"],
                },
                sid,
            )
    hud_get = call_tool(
        BT, "find_nodes", {"graph": TICK_GRAPH, "title": "Get HUD"}, sid
    ).get("returnValue", [])
    if hud_get:
        hg = call_tool(BT, "get_node_infos", {"nodes": [hud_get[0]]}, sid)[
            "returnValue"
        ][0]
        for inp in hi["input_pins"]:
            if inp["name"] == "self":
                call_tool(
                    BT,
                    "connect_pins",
                    {
                        "output_pin": hg["output_pins"][0]["pin_id"],
                        "input_pin": inp["pin_id"],
                    },
                    sid,
                )
                print("TickStore GetHoveredItem -> HUD")

# --- PendingPress + drag feed into UpdateDrag ---
names = call_tool(BT, "list_variables", {"blueprint": {"refPath": GRID}}, sid).get(
    "returnValue", []
)
if "PendingPress" not in names:
    call_tool(
        BT,
        "add_variable",
        {"blueprint": {"refPath": GRID}, "name": "PendingPress", "type_name": "int"},
        sid,
    )

# Find UpdateDrag call and GetPressedItem call in TickStore
pressed = call_tool(
    BT, "find_nodes", {"graph": TICK_GRAPH, "title": "GetPressedItem"}, sid
).get("returnValue", [])
ud = call_tool(
    BT, "find_nodes", {"graph": TICK_GRAPH, "title": "Update Drag"}, sid
).get("returnValue", [])
if pressed and ud:
    p_out = call_tool(BT, "get_node_infos", {"nodes": [pressed[0]]}, sid)["returnValue"][
        0
    ]["output_pins"][0]["pin_id"]
    ud_i = call_tool(BT, "get_node_infos", {"nodes": [ud[0]]}, sid)["returnValue"][0]
    pressed_in = next(p for p in ud_i["input_pins"] if p["name"] == "Pressed")
    # Insert Set PendingPress when press >= 0, Select feed, then UpdateDrag
    # Only patch wire if Pressed still wired directly from GetPressedItem
    if pressed_in["connected_pins"] and pressed_in["connected_pins"][0]["node"][
        "refPath"
    ] == pressed[0]["refPath"]:
        # Create nodes
        pc = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Game|GetPlayerController",
                "pos": {"x": 400, "y": 400},
            },
            sid,
        )["returnValue"]["refPath"]
        set_pend = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Variables|Default|SetPendingPress",
                "pos": {"x": 900, "y": 350},
            },
            sid,
        )["returnValue"]["refPath"]
        get_pend = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Variables|Default|GetPendingPress",
                "pos": {"x": 1100, "y": 450},
            },
            sid,
        )["returnValue"]["refPath"]
        get_sel = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Variables|Default|GetSelectedItem",
                "pos": {"x": 1100, "y": 550},
            },
            sid,
        )["returnValue"]["refPath"]
        is_down = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Game|Player|IsInputKeyDown",
                "pos": {"x": 900, "y": 550},
            },
            sid,
        )["returnValue"]["refPath"]
        isi = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": is_down}]}, sid)[
            "returnValue"
        ][0]
        call_tool(
            BT,
            "set_pin_value",
            {"pin": isi["input_pins"][1]["pin_id"], "value": "LeftMouseButton"},
            sid,
        )
        call_tool(
            BT,
            "connect_pins",
            {
                "output_pin": call_tool(
                    BT, "get_node_infos", {"nodes": [{"refPath": pc}]}, sid
                )["returnValue"][0]["output_pins"][0]["pin_id"],
                "input_pin": isi["input_pins"][0]["pin_id"],
            },
            sid,
        )
        # Break UpdateDrag Pressed, connect via simple path: while placing&&LMB use SelectedItem else PendingPress if LMB else Pressed
        # Minimal: use Promotable Select: if (selected>=0 && LMBdown) selected else (LMBdown && pending>=0) pending else pressed
        # MCP select node creation is heavy — use SetDragFeed variable + 2 sets instead
        if "DragFeed" not in names:
            call_tool(
                BT,
                "add_variable",
                {"blueprint": {"refPath": GRID}, "name": "DragFeed", "type_name": "int"},
                sid,
            )
        set_feed = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Variables|Default|SetDragFeed",
                "pos": {"x": 1300, "y": 400},
            },
            sid,
        )["returnValue"]["refPath"]
        get_feed = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Variables|Default|GetDragFeed",
                "pos": {"x": 1500, "y": 400},
            },
            sid,
        )["returnValue"]["refPath"]

        # Wire: break Pressed on UpdateDrag, connect GetDragFeed
        call_tool(
            BT,
            "break_pins",
            {"output_pin": p_out, "input_pin": pressed_in["pin_id"]},
            sid,
        )
        gf = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": get_feed}]}, sid)[
            "returnValue"
        ][0]
        call_tool(
            BT,
            "connect_pins",
            {
                "output_pin": gf["output_pins"][0]["pin_id"],
                "input_pin": pressed_in["pin_id"],
            },
            sid,
        )

        # Exec chain: between existing nodes — connect pressed output to set drag feed press path
        # Set DragFeed = Pressed raw for now (latch on HUD handles hold); also set pending when press>=0
        sf = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": set_feed}]}, sid)[
            "returnValue"
        ][0]
        call_tool(
            BT,
            "connect_pins",
            {"output_pin": p_out, "input_pin": sf["input_pins"][1]["pin_id"]},
            sid,
        )
        # Set PendingPress from same pressed output — use set_pend input value pin
        sp = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": set_pend}]}, sid)[
            "returnValue"
        ][0]
        call_tool(
            BT,
            "connect_pins",
            {"output_pin": p_out, "input_pin": sp["input_pins"][1]["pin_id"]},
            sid,
        )
        # While placing: override DragFeed with SelectedItem when LMB down — branch compare nodes
        cmp_sel = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Math|Integer|integer<integer",
                "pos": {"x": 1250, "y": 600},
            },
            sid,
        )["returnValue"]["refPath"]
        ci = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": cmp_sel}]}, sid)[
            "returnValue"
        ][0]
        call_tool(
            BT,
            "set_pin_value",
            {"pin": ci["input_pins"][1]["pin_id"], "value": "0"},
            sid,
        )
        call_tool(
            BT,
            "connect_pins",
            {
                "output_pin": call_tool(
                    BT, "get_node_infos", {"nodes": [{"refPath": get_sel}]}, sid
                )["returnValue"][0]["output_pins"][0]["pin_id"],
                "input_pin": ci["input_pins"][0]["pin_id"],
            },
            sid,
        )
        and_pl = call_tool(
            BT,
            "create_node",
            {
                "graph": TICK_GRAPH,
                "type_id": "Math|Boolean|ANDBoolean",
                "pos": {"x": 1400, "y": 580},
            },
            sid,
        )["returnValue"]["refPath"]
        ai = call_tool(BT, "get_node_infos", {"nodes": [{"refPath": and_pl}]}, sid)[
            "returnValue"
        ][0]
        call_tool(
            BT,
            "connect_pins",
            {
                "output_pin": ci["output_pins"][0]["pin_id"],
                "input_pin": ai["input_pins"][0]["pin_id"],
            },
            sid,
        )
        call_tool(
            BT,
            "connect_pins",
            {
                "output_pin": isi["output_pins"][0]["pin_id"],
                "input_pin": ai["input_pins"][1]["pin_id"],
            },
            sid,
        )
        # Select node type might not exist — if placing, set DragFeed value from SelectedItem pin on set_feed
        call_tool(
            BT,
            "connect_pins",
            {
                "output_pin": call_tool(
                    BT, "get_node_infos", {"nodes": [{"refPath": get_sel}]}, sid
                )["returnValue"][0]["output_pins"][0]["pin_id"],
                "input_pin": sf["input_pins"][1]["pin_id"],
            },
            sid,
        )
        print("TickStore DragFeed wired (may need manual exec chain)")

subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-hud-widget-bindings.py"])
subprocess.check_call([sys.executable, r"e:\Dungeon Core\tmp\fix-get-pressed-item-latch.py"])

call_tool(UMG, "CompileWidgetBlueprint", {"widgetBlueprint": {"refPath": HUD}}, sid)
call_tool(BT, "compile_blueprint", {"blueprint": {"refPath": GRID}}, sid)
call_tool(AT, "save_assets", {"asset_paths": [GRID, HUD]}, sid)
print("store drag restore pass done — test in PIE")