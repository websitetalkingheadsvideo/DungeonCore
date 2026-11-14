import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"


def call(tool: str, arguments: dict) -> dict:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {
                    "toolset_name": "UMGToolSet.UMGToolSet",
                    "tool_name": tool,
                    "arguments": arguments,
                },
            },
        }
    ).encode()
    req = urllib.request.Request(
        MCP_URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Mcp-Session-Id": open(SID_PATH, encoding="utf-8").read().strip(),
        },
    )
    text = json.loads(urllib.request.urlopen(req, timeout=120).read())["result"][
        "content"
    ][0]["text"]
    return json.loads(text)


d = call("GetWidgetDescription", {"widgetBlueprint": {"refPath": HUD}})["returnValue"]
for w in d["widgets"]:
    n = w["widgetName"]
    if any(n.startswith(p) for p in ("Btn_", "Row_", "Store", "Icon_", "Cost_", "BtnTxt")):
        print(n, w.get("widgetClass", ""), "parent:", w.get("parentWidget", ""))
