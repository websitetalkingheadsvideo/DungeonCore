import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
GEN = (300.0, 300.0)


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
    ac = "editor_toolset.toolsets.actor.ActorTools"
    for tag in ("Dress_0", "Dress_3"):
        refs = call_tool(
            "editor_toolset.toolsets.scene.SceneTools",
            "find_actors",
            {"name": "", "tag": tag, "collision_channels": []},
        )["returnValue"]
        near = []
        for item in refs:
            ref = item["refPath"]
            loc = call_tool(
                ac, "get_actor_transform", {"actor": {"refPath": ref}}
            )["returnValue"]["location"]
            dx = abs(loc["x"] - GEN[0])
            dy = abs(loc["y"] - GEN[1])
            if dx <= 120 and dy <= 120:
                label = call_tool(ac, "get_label", {"actor": {"refPath": ref}})["returnValue"]
                tags = call_tool(ac, "get_tags", {"actor": {"refPath": ref}})["returnValue"]
                near.append((label, loc, tags))
        print("===", tag, "within 120uu of gen ===")
        for row in near:
            print(" ", row)


if __name__ == "__main__":
    main()
