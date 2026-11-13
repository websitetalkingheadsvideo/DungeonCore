import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"


def call_tool(toolset: str, tool: str, arguments: dict):
    sid = open(SID_PATH, encoding="utf-8").read().strip()
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
    return json.loads(text)


def main() -> None:
    bp = "editor_toolset.toolsets.blueprint.BlueprintTools"
    graph = call_tool(
        bp,
        "get_graph",
        {
            "blueprint": {"refPath": "/Game/UI/WBP_NewGame.WBP_NewGame"},
            "graph_name": "ApplyDressing",
        },
    )
    nodes = call_tool(
        bp,
        "find_nodes",
        {"graph": graph["returnValue"], "title": "Set Actor Hidden In Game"},
    )["returnValue"]
    for index, node in enumerate(nodes):
        info = call_tool(bp, "get_node_infos", {"nodes": [node]})["returnValue"][0]
        pins = {}
        for pin in info["input_pins"]:
            pins[pin["name"]] = {
                "value": pin.get("value"),
                "connected": len(pin.get("connected_pins", [])),
            }
        print(index, pins)


if __name__ == "__main__":
    main()
