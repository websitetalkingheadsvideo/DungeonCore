import json
import sys
import urllib.request

MCP_URL = "http://127.0.0.1:8000/mcp"
SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"


def rpc(method: str, params: dict, sid: str | None = None) -> dict:
    body = json.dumps({"jsonrpc": "2.0", "id": 1, "method": method, "params": params}).encode()
    headers = {
        "Content-Type": "application/json",
        "Accept": "application/json, text/event-stream",
    }
    if sid:
        headers["Mcp-Session-Id"] = sid
    req = urllib.request.Request(MCP_URL, data=body, headers=headers)
    raw = urllib.request.urlopen(req, timeout=120).read().decode()
    return json.loads(raw)


def init_session() -> str:
    result = rpc(
        "initialize",
        {
            "protocolVersion": "2024-11-05",
            "capabilities": {},
            "clientInfo": {"name": "mcp-call", "version": "1"},
        },
    )
    sid = result.get("result", {}).get("sessionId") or result.get("headers", {})
    # session id often returned via response header - urllib doesn't expose easily
    # try reading from mcp-sid or from initialize notification
    try:
        req = urllib.request.Request(
            MCP_URL,
            data=json.dumps(
                {
                    "jsonrpc": "2.0",
                    "id": 2,
                    "method": "notifications/initialized",
                    "params": {},
                }
            ).encode(),
            headers={
                "Content-Type": "application/json",
                "Accept": "application/json, text/event-stream",
                "Mcp-Session-Id": open(SID_PATH, encoding="utf-8").read().strip(),
            },
        )
        urllib.request.urlopen(req, timeout=30)
    except Exception:
        pass
    return open(SID_PATH, encoding="utf-8").read().strip()


def call_tool(toolset: str, tool: str, arguments: dict, sid: str) -> object:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {
                    "toolset_name": toolset,
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
            "Mcp-Session-Id": sid,
        },
    )
    raw = json.loads(urllib.request.urlopen(req, timeout=120).read().decode())
    text = raw["result"]["content"][0]["text"]
    if raw["result"].get("isError"):
        raise RuntimeError(text)
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return text


if __name__ == "__main__":
    sid = init_session()
    result = call_tool(sys.argv[1], sys.argv[2], json.loads(sys.argv[3]), sid)
    print(json.dumps(result, indent=2)[:8000])
