import json, urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()
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
            timeout=300,
        ).read()
    )["result"]["content"][0]["text"]


GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"
tick = json.loads(call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}))["returnValue"]
old = "(CallFunction|UpdateDrag _index))"
new = "(CallFunction|UpdateDrag _index_TESTMARKER))"
pat = tick.replace(old, new, 1)
print("replacing tail", old in tick)
w = call("write_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}, "code": pat})
print("WRITE:", w)
tick2 = json.loads(call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:TickStore"}}))["returnValue"]
print("VERIFY:", "_index_TESTMARKER" in tick2)