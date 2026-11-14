import json
import urllib.request

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
body = json.dumps(
    {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "tools/call",
        "params": {
            "name": "call_tool",
            "arguments": {
                "toolset_name": BT,
                "tool_name": "read_graph_dsl",
                "arguments": {
                    "graph": {"refPath": "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD:EventGraph"}
                },
            },
        },
    }
).encode()
text = json.loads(
    urllib.request.urlopen(
        urllib.request.Request(
            "http://127.0.0.1:8000/mcp",
            data=body,
            headers={
                "Content-Type": "application/json",
                "Accept": "application/json, text/event-stream",
                "Mcp-Session-Id": sid,
            },
        ),
        timeout=120,
    ).read()
)["result"]["content"][0]["text"]
print(json.loads(text)["returnValue"])
