import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
SUNKEN_MESH = "/Game/Environments/AncientCrypt/Meshes/Modular/SM_Column_01.SM_Column_01"


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
    try:
        call_tool("EditorToolset.EditorAppToolset", "StopPIE", {})
    except RuntimeError as exc:
        if "not currently running" not in str(exc):
            raise

    scene = "editor_toolset.toolsets.scene.SceneTools"
    ac = "editor_toolset.toolsets.actor.ActorTools"
    ob = "editor_toolset.toolsets.object.ObjectTools"

    ref = call_tool(
        scene,
        "find_actors",
        {"name": "MonsterGen_Sunken", "tag": "", "collision_channels": []},
    )["returnValue"][0]["refPath"]
    ref = ref.replace("UEDPIE_0_", "")

    comp = call_tool(ac, "get_components", {"actor": {"refPath": ref}})["returnValue"][0][
        "refPath"
    ]
    call_tool(
        ob,
        "set_properties",
        {
            "instance": {"refPath": comp},
            "values": json.dumps({"StaticMesh": {"refPath": SUNKEN_MESH}}),
        },
    )
    call_tool(
        ac,
        "set_actor_transform",
        {
            "actor": {"refPath": ref},
            "xform": {
                "location": {"x": 300.0, "y": 300.0, "z": 0.0},
                "rotation": {"pitch": 0.0, "yaw": 45.0, "roll": 0.0},
                "scale": {"x": 1.0, "y": 1.0, "z": 1.0},
            },
        },
    )
    call_tool(
        ob,
        "set_properties",
        {
            "instance": {"refPath": ref},
            "values": json.dumps({"bHidden": False}),
        },
    )

    bounds = call_tool(ac, "get_actor_bounds", {"actor": {"refPath": ref}})
    mesh = json.loads(
        call_tool(
            ob,
            "get_properties",
            {"instance": {"refPath": comp}, "properties": ["StaticMesh"]},
        )["returnValue"]
    )
    print("MonsterGen_Sunken updated", mesh, bounds)

    call_tool(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {"asset_paths": ["/Game/Maps/L_NewGame"]},
    )
    call_tool(
        "SlateInspectorToolset.SlateInspectorToolset",
        "PressKey",
        {"key": "Ctrl+Shift+S"},
    )


if __name__ == "__main__":
    main()
