import json
import urllib.request

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
body = json.dumps(
    {
        "jsonrpc": "2.0",
        "id": 3,
        "method": "tools/call",
        "params": {"name": "describe_toolset", "arguments": {"toolset_name": "editor_toolset.toolsets.asset.AssetTools"}},
    }
).encode()
req = urllib.request.Request(
    "http://127.0.0.1:8000/mcp",
    data=body,
    headers={
        "Content-Type": "application/json",
        "Accept": "application/json, text/event-stream",
        "Mcp-Session-Id": sid,
    },
)
raw = json.loads(urllib.request.urlopen(req, timeout=60).read().decode())
data = json.loads(raw["result"]["content"][0]["text"])
for tool in data["tools"]:
    short = tool["name"].split(".")[-1]
    print(short, "-", (tool.get("description") or "").split("\n")[0][:140])
