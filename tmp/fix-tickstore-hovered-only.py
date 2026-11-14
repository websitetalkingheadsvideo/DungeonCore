import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"


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
    return json.loads(
        urllib.request.urlopen(
            urllib.request.Request(
                "http://127.0.0.1:8000/mcp",
                data=body,
                headers={
                    "Content-Type": "application/json",
                    "Accept": "application/json, text/event-stream",
                    "Mcp-Session-Id": SID,
                },
            ),
            timeout=120,
        ).read()
    )["result"]["content"][0]["text"]


tick = json.loads(call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}))["returnValue"]
pat = tick.replace(
    "(bind _index_1 (Variables|Default|GetHoveredItem _hud))",
    "(bind _index_1 (Class|WBPDungeonHUD|GetHoveredItem _hud))",
    1,
)
w = call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": pat})
print(w)
v = json.loads(call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}))["returnValue"]
print("fixed", "Class|WBPDungeonHUD|GetHoveredItem" in v)