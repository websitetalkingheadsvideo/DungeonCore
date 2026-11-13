import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
EDITOR_APP = "EditorToolset.EditorAppToolset"
SCENE = "editor_toolset.toolsets.scene.SceneTools"
ACTOR = "editor_toolset.toolsets.actor.ActorTools"
OBJECT = "editor_toolset.toolsets.object.ObjectTools"
ASSET = "editor_toolset.toolsets.asset.AssetTools"


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def rawcall(name: str, arguments: dict, timeout: int = 120):
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {"name": name, "arguments": arguments},
        }
    ).encode()
    req = urllib.request.Request(
        MCP_URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Mcp-Session-Id": sid(),
        },
    )
    raw = json.loads(urllib.request.urlopen(req, timeout=timeout).read().decode())
    text = raw["result"]["content"][0]["text"]
    if raw["result"].get("isError"):
        raise RuntimeError(text)
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return text


def call_tool(toolset: str, tool: str, args: dict):
    return rawcall(
        "call_tool",
        {"toolset_name": toolset, "tool_name": tool, "arguments": args},
    )


def persistent_ref(ref_path: str) -> str:
    return ref_path.replace("/Game/Maps/UEDPIE_0_L_NewGame", "/Game/Maps/L_NewGame")


def main() -> None:
    print("StopPIE", call_tool(EDITOR_APP, "StopPIE", {}))
    refs = call_tool(
        SCENE,
        "find_actors",
        {"name": "MonsterGen", "tag": "", "collision_channels": []},
    )["returnValue"]
    for item in refs:
        ref = persistent_ref(item["refPath"])
        tags = call_tool(ACTOR, "get_tags", {"actor": {"refPath": ref}})[
            "returnValue"
        ]
        before = call_tool(
            OBJECT,
            "get_properties",
            {"instance": {"refPath": ref}, "properties": ["bHidden"]},
        )
        call_tool(
            OBJECT,
            "set_properties",
            {
                "instance": {"refPath": ref},
                "values": json.dumps({"bHidden": False}),
            },
        )
        after = call_tool(
            OBJECT,
            "get_properties",
            {"instance": {"refPath": ref}, "properties": ["bHidden"]},
        )
        print(ref.split("_")[-1], tags, "bHidden", before, "->", after)

    save = call_tool(ASSET, "save_assets", {"asset_paths": ["/Game/Maps/L_NewGame"]})
    print("save_assets", save)


if __name__ == "__main__":
    main()
