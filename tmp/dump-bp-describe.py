import json
import urllib.request

sid = open(r"e:\Dungeon Core\tmp\mcp-sid.txt").read().strip()
body = json.dumps(
    {
        "jsonrpc": "2.0",
        "id": 1,
        "method": "tools/call",
        "params": {
            "name": "describe_toolset",
            "arguments": {"toolset_name": "editor_toolset.toolsets.blueprint.BlueprintTools"},
        },
    }
).encode()
raw = urllib.request.urlopen(
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
).read().decode()
open(r"e:\Dungeon Core\tmp\bp-describe-raw.json", "w", encoding="utf-8").write(raw)
print("wrote", len(raw), "bytes")
print(raw[:1500])