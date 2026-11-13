import json
import urllib.request
import time

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"


def call_tool(toolset: str, tool: str, arguments: dict, timeout: int = 120):
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
    raw = json.loads(urllib.request.urlopen(req, timeout=timeout).read().decode())
    text = raw["result"]["content"][0]["text"]
    if raw["result"].get("isError"):
        raise RuntimeError(text[:2000])
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return text


def actor_hidden(ref: str) -> dict:
    ac = "editor_toolset.toolsets.actor.ActorTools"
    ob = "editor_toolset.toolsets.object.ObjectTools"
    comp = call_tool(ac, "get_components", {"actor": {"refPath": ref}})["returnValue"][0][
        "refPath"
    ]
    actor_h = json.loads(
        call_tool(ob, "get_properties", {"instance": {"refPath": ref}, "properties": ["bHidden"]})[
            "returnValue"
        ]
    )
    comp_h = json.loads(
        call_tool(
            ob,
            "get_properties",
            {
                "instance": {"refPath": comp},
                "properties": ["bHiddenInGame", "bVisible", "bOwnerNoSee"],
            },
        )["returnValue"]
    )
    return {"actor": actor_h, "component": comp_h}


def main() -> None:
    pie = call_tool("EditorToolset.EditorAppToolset", "IsPIERunning", {})
    print("IsPIERunning", pie)

    scene = "editor_toolset.toolsets.scene.SceneTools"
    for name in (
        "MonsterGen_Gothic",
        "MonsterGen_Goblin",
        "MonsterGen_Sunken",
        "MonsterGen_Fungal_0",
    ):
        refs = call_tool(
            scene, "find_actors", {"name": name, "tag": "", "collision_channels": []}
        )["returnValue"]
        if not refs:
            print(name, "NOT FOUND")
            continue
        ref = refs[0]["refPath"]
        print(name, "ref=", ref)
        print("  hidden", actor_hidden(ref))


if __name__ == "__main__":
    main()
