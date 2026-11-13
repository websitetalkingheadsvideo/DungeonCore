import json
import urllib.request

BASE = "http://127.0.0.1:8000/mcp"


def rpc(method, params, id_, sid=None):
    headers = {
        "Content-Type": "application/json",
        "Accept": "application/json, text/event-stream",
    }
    if sid:
        headers["Mcp-Session-Id"] = sid
    body = json.dumps({"jsonrpc": "2.0", "id": id_, "method": method, "params": params}).encode()
    req = urllib.request.Request(BASE, data=body, headers=headers)
    with urllib.request.urlopen(req, timeout=60) as response:
        sid_out = response.headers.get("Mcp-Session-Id") or sid
        payload = json.loads(response.read().decode())
    return payload, sid_out


init, sid = rpc(
    "initialize",
    {
        "protocolVersion": "2024-11-05",
        "capabilities": {},
        "clientInfo": {"name": "agent", "version": "1"},
    },
    1,
)
print("sid", sid)
note = urllib.request.Request(
    BASE,
    data=json.dumps({"jsonrpc": "2.0", "method": "notifications/initialized"}).encode(),
    headers={
        "Content-Type": "application/json",
        "Accept": "application/json, text/event-stream",
        "Mcp-Session-Id": sid,
    },
)
try:
    urllib.request.urlopen(note, timeout=15).read()
except Exception as exc:
    print("note", exc)

listed, sid = rpc("tools/call", {"name": "list_toolsets", "arguments": {}}, 2, sid)
text = listed["result"]["content"][0]["text"]
for line in text.splitlines():
    low = line.lower()
    if any(key in low for key in ("import", "interchange", "fbx", "asset", "staticmesh", "skeletal")):
        print(line[:220])

open(r"e:\Dungeon Core\tmp\mcp-sid.txt", "w", encoding="utf-8").write(sid)
