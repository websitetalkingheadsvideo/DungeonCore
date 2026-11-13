import json
import urllib.request

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
body = json.dumps(
    {
        "jsonrpc": "2.0",
        "id": 4,
        "method": "tools/call",
        "params": {"name": "list_toolsets", "arguments": {}},
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
text = raw["result"]["content"][0]["text"]
open(r"e:\Dungeon Core\tmp\toolsets.txt", "w", encoding="utf-8").write(text)
for line in text.splitlines():
    if line.startswith("- "):
        print(line.split(":")[0])
