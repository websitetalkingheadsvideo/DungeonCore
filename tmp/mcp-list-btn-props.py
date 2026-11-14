import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"


def call(ts, tool, args):
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": ts, "tool_name": tool, "arguments": args},
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
    return json.loads(text) if text.strip() else {}


for name in ("Btn_0", "Row_0", "StoreBox", "Icon_0", "Cost_0"):
    inst = {"refPath": f"{HUD}:WidgetTree.{name}"}
    props = call("editor_toolset.toolsets.object.ObjectTools", "list_properties", {"instance": inst})
    rv = props.get("returnValue", props)
    if isinstance(rv, list):
        names = [p.get("name", p) for p in rv]
    else:
        names = rv
    print(name, names)
