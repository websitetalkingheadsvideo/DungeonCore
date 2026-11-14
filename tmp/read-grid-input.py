import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
URL = "http://127.0.0.1:8000/mcp"
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
    req = urllib.request.Request(
        URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Mcp-Session-Id": SID,
        },
    )
    text = json.loads(urllib.request.urlopen(req, timeout=120).read())["result"]["content"][0]["text"]
    return json.loads(text).get("returnValue")


for g in ["EventGraph", "BeginPlay", "ReceiveTick", "OnLeftClick", "SetupInput"]:
    try:
        dsl = call("read_graph_dsl", {"graph": {"refPath": f"{GRID}:{g}"}})
        print(f"\n=== {g} ===\n{dsl}\n")
    except Exception as exc:
        print(g, "missing", exc)
