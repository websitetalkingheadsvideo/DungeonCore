import json
import re
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
req = urllib.request.Request(
    "http://127.0.0.1:8000/mcp",
    data=body,
    headers={
        "Content-Type": "application/json",
        "Accept": "application/json, text/event-stream",
        "Mcp-Session-Id": sid,
    },
)
text = urllib.request.urlopen(req, timeout=60).read().decode()
for name in re.findall(r'"name":"(editor_toolset\.toolsets\.blueprint\.BlueprintTools\.[^"]+)"', text):
    if "variable" in name.lower() or "compile" in name.lower():
        print(name)
