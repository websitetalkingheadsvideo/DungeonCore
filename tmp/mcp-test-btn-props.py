import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
URL = "http://127.0.0.1:8000/mcp"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"


def call(ts, tool, args):
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {
                    "toolset_name": ts,
                    "tool_name": tool,
                    "arguments": args,
                },
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
    return urllib.request.urlopen(req, timeout=120).read().decode()


inst = {"refPath": f"{HUD}:WidgetTree.Btn_0"}
print("list", call("editor_toolset.toolsets.object.ObjectTools", "list_properties", {"instance": inst})[:2500])
print(
    "set",
    call(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {
            "instance": inst,
            "values": json.dumps({"backgroundColor": {"r": 0.05, "g": 0.08, "b": 0.16, "a": 1.0}}),
        },
    ),
)
