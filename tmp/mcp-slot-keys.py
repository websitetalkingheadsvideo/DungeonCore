import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
OB = "editor_toolset.toolsets.object.ObjectTools"
btn = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD:WidgetTree.Btn_0"
slot_raw = json.loads(
    json.loads(
        urllib.request.urlopen(
            urllib.request.Request(
                "http://127.0.0.1:8000/mcp",
                data=json.dumps(
                    {
                        "jsonrpc": "2.0",
                        "id": 1,
                        "method": "tools/call",
                        "params": {
                            "name": "call_tool",
                            "arguments": {
                                "toolset_name": OB,
                                "tool_name": "get_properties",
                                "arguments": {"instance": {"refPath": btn}, "properties": ["slot"]},
                            },
                        },
                    }
                ).encode(),
                headers={
                    "Content-Type": "application/json",
                    "Accept": "application/json, text/event-stream",
                    "Mcp-Session-Id": SID,
                },
            ),
            timeout=120,
        ).read()
    )["result"]["content"][0]["text"]
)["returnValue"]
ref = json.loads(slot_raw)["slot"]["refPath"]
body = json.dumps(
    {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "tools/call",
        "params": {
            "name": "call_tool",
            "arguments": {
                "toolset_name": "editor_toolset.toolsets.object.ObjectTools",
                "tool_name": "get_properties",
                "arguments": {
                    "instance": {"refPath": ref},
                    "properties": ["padding", "size", "horizontalAlignment", "verticalAlignment"],
                },
            },
        },
    }
).encode()
req = urllib.request.Request(
    "http://127.0.0.1:8000/mcp",
    data=body,
    headers={
        "Content-Type": "application/json",
        "Accept": "application/json, text/event-stream",
        "Mcp-Session-Id": SID,
    },
)
text = json.loads(urllib.request.urlopen(req, timeout=120).read())["result"]["content"][0]["text"]
print(text)
