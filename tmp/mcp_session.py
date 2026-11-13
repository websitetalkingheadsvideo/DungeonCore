import json
import urllib.request

MCP_URL = "http://127.0.0.1:8000/mcp"
SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"


def new_session() -> str:
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "initialize",
            "params": {
                "protocolVersion": "2024-11-05",
                "capabilities": {},
                "clientInfo": {"name": "dungeon-core", "version": "1"},
            },
        }
    ).encode()
    req = urllib.request.Request(
        MCP_URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
        },
    )
    with urllib.request.urlopen(req, timeout=30) as resp:
        sid = resp.headers.get("Mcp-Session-Id") or resp.headers.get("mcp-session-id")
        if not sid:
            raise RuntimeError("No Mcp-Session-Id header on initialize")
        initialized = json.dumps(
            {"jsonrpc": "2.0", "method": "notifications/initialized", "params": {}}
        ).encode()
        req2 = urllib.request.Request(
            MCP_URL,
            data=initialized,
            headers={
                "Content-Type": "application/json",
                "Accept": "application/json, text/event-stream",
                "Mcp-Session-Id": sid,
            },
        )
        urllib.request.urlopen(req2, timeout=30)
        with open(SID_PATH, "w", encoding="utf-8") as f:
            f.write(sid)
        return sid


def call_tool(toolset: str, tool: str, arguments: dict, sid: str):
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 2,
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
    raw = json.loads(urllib.request.urlopen(req, timeout=180).read().decode())
    text = raw["result"]["content"][0]["text"]
    if raw["result"].get("isError"):
        raise RuntimeError(text)
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return text


if __name__ == "__main__":
    sid = new_session()
    print("sid", sid)
    print(
        call_tool(
            "EditorToolset.EditorAppToolset",
            "IsPIERunning",
            {},
            sid,
        )
    )
