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
text = urllib.request.urlopen(
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
names = re.findall(
    r'"name":"(editor_toolset\.toolsets\.blueprint\.BlueprintTools\.[^"]+)"', text
)
path = r"e:\Dungeon Core\tmp\bp-tools-list.txt"
open(path, "w", encoding="utf-8").write("\n".join(sorted(names)))
print(len(names), "tools ->", path)
for n in sorted(names):
    low = n.lower()
    if any(x in low for x in ("graph", "node", "pin", "patch", "edit", "replace", "compile")):
        print(n)