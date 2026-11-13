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
        raise RuntimeError(text[:2000])
    return json.loads(text)


def main() -> None:
    ac = "editor_toolset.toolsets.actor.ActorTools"
    ob = "editor_toolset.toolsets.object.ObjectTools"
    refs = call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": "Creature", "tag": "", "collision_channels": []},
    )["returnValue"]
    for item in refs:
        ref = item["refPath"]
        label = call_tool(ac, "get_label", {"actor": {"refPath": ref}})["returnValue"]
        if not label.startswith("Creature") and label != "elite_orc":
            continue
        comps = call_tool(ac, "get_components", {"actor": {"refPath": ref}})["returnValue"]
        sk = next((c for c in comps if "SkeletalMesh" in c["refPath"]), comps[0] if comps else None)
        if not sk:
            continue
        props = json.loads(
            call_tool(
                ob,
                "get_properties",
                {
                    "instance": {"refPath": sk["refPath"]},
                    "properties": ["SkeletalMesh", "AnimClass", "AnimationMode"],
                },
            )["returnValue"]
        )
        print(label, props)


if __name__ == "__main__":
    main()
