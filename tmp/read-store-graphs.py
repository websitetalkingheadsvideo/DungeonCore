import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
URL = "http://127.0.0.1:8000/mcp"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"


def call(tool, args):
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": BT, "tool_name": tool, "arguments": args},
            },
        }
    ).encode()
    req = urllib.request.Request(
        URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Mcp-Session-Id": SID,
        },
    )
    text = json.loads(urllib.request.urlopen(req, timeout=300).read())["result"]["content"][0]["text"]
    outer = json.loads(text)
    return outer.get("returnValue", outer)


HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

for bp, graphs in [
    (HUD, ["GetPressedItem", "GetHoveredItem"]),
    (GRID, ["TickStore", "UpdateDrag", "TryPlace"]),
]:
    for g in graphs:
        dsl = call("read_graph_dsl", {"graph": {"refPath": f"{bp}:{g}"}})
        print(f"\n{'='*60}\n{bp} :: {g}\n{'='*60}\n")
        print(dsl if isinstance(dsl, str) else json.dumps(dsl, indent=2)[:12000])
