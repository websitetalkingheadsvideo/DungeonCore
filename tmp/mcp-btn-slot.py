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
    return json.loads(text)


ob = "editor_toolset.toolsets.object.ObjectTools"
btn = f"{HUD}:WidgetTree.Btn_0"
raw = call(ob, "get_properties", {"instance": {"refPath": btn}, "properties": ["slot"]})["returnValue"]
slot = json.loads(raw) if isinstance(raw, str) else raw
print("btn slot", json.dumps(slot, indent=2)[:800])
slot_ref = slot["slot"]["refPath"]
print("slot_ref", slot_ref)
print("slot props schema keys", list(call(ob, "list_properties", {"instance": {"refPath": slot_ref}}).get("returnValue", {}).keys())[:20])
