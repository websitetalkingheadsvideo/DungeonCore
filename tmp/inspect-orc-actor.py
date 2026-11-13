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
        raise RuntimeError(text[:3000])
    return json.loads(text)


def main() -> None:
    ac = "editor_toolset.toolsets.actor.ActorTools"
    ob = "editor_toolset.toolsets.object.ObjectTools"
    for tag in ("Creature_0",):
        refs = call_tool(
            "editor_toolset.toolsets.scene.SceneTools",
            "find_actors",
            {"name": "", "tag": tag, "collision_channels": []},
        )["returnValue"]
        for item in refs:
            ref = item["refPath"].replace("UEDPIE_0_", "")
            label = call_tool(ac, "get_label", {"actor": {"refPath": ref}})["returnValue"]
            tags = call_tool(ac, "get_tags", {"actor": {"refPath": ref}})["returnValue"]
            print("label", label, "tags", tags)
            comps = call_tool(ac, "get_components", {"actor": {"refPath": ref}})["returnValue"]
            for c in comps:
                print("  comp", c.get("refPath", c))
                props = call_tool(
                    ob,
                    "get_properties",
                    {
                        "instance": {"refPath": c["refPath"]},
                        "properties": [
                            "SkeletalMesh",
                            "AnimClass",
                            "AnimationMode",
                            "AnimationData",
                        ],
                    },
                )
                print("   ", props)
    for name in ("elite_orc", "Orc", "Creature"):
        refs = call_tool(
            "editor_toolset.toolsets.scene.SceneTools",
            "find_actors",
            {"name": name, "tag": "", "collision_channels": []},
        )["returnValue"]
        if refs:
            print("by name", name, len(refs))
            for item in refs[:3]:
                ref = item["refPath"]
                print(
                    " ",
                    call_tool(ac, "get_label", {"actor": {"refPath": ref}})["returnValue"],
                )


if __name__ == "__main__":
    main()
