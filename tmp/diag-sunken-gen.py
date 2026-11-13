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


def dump_actor(name: str) -> None:
    ref = call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": name, "tag": "", "collision_channels": []},
    )["returnValue"][0]["refPath"]
    ref = ref.replace("/Game/Maps/UEDPIE_0_L_NewGame", "/Game/Maps/L_NewGame")
    ac = "editor_toolset.toolsets.actor.ActorTools"
    ob = "editor_toolset.toolsets.object.ObjectTools"
    label = call_tool(ac, "get_label", {"actor": {"refPath": ref}})["returnValue"]
    tags = call_tool(ac, "get_tags", {"actor": {"refPath": ref}})["returnValue"]
    bh = json.loads(
        call_tool(ob, "get_properties", {"instance": {"refPath": ref}, "properties": ["bHidden"]})[
            "returnValue"
        ]
    )
    xf = call_tool(ac, "get_actor_transform", {"actor": {"refPath": ref}})["returnValue"]
    bounds = call_tool(ac, "get_actor_bounds", {"actor": {"refPath": ref}})
    comp = call_tool(ac, "get_components", {"actor": {"refPath": ref}})["returnValue"][0][
        "refPath"
    ]
    mesh = json.loads(
        call_tool(
            ob, "get_properties", {"instance": {"refPath": comp}, "properties": ["StaticMesh"]}
        )["returnValue"]
    )
    hidden_game = call_tool(
        ob,
        "get_properties",
        {"instance": {"refPath": comp}, "properties": ["bHiddenInGame", "bVisible"]},
    )
    print("===", label, "===")
    print("ref", ref)
    print("tags", tags)
    print("bHidden", bh)
    print("transform", xf)
    print("bounds", bounds)
    print("mesh", mesh)
    print("comp props", hidden_game)


if __name__ == "__main__":
    try:
        call_tool("EditorToolset.EditorAppToolset", "StopPIE", {})
    except RuntimeError:
        pass
    dump_actor("MonsterGen_Gothic")
    dump_actor("MonsterGen_Sunken")
