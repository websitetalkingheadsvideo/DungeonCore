"""Sunken monster gen was SM_Column_01 — identical to corner ThemePillar dressing."""
import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"

MESH_WATER = (
    "/Game/Environments/AncientCrypt/Meshes/Modular/SM_WaterFloor_01.SM_WaterFloor_01"
)
MI_WATER = "/Game/Environments/AncientCrypt/Materials/MI_Water_Inst.MI_Water_Inst"


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def call_tool(toolset: str, tool: str, arguments: dict):
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
            "Mcp-Session-Id": sid(),
        },
    )
    raw = json.loads(urllib.request.urlopen(req, timeout=180).read().decode())
    text = raw["result"]["content"][0]["text"]
    if raw["result"].get("isError"):
        raise RuntimeError(text)
    return json.loads(text)


def find_actor(name: str) -> str:
    ref = call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": name, "tag": "", "collision_channels": []},
    )["returnValue"][0]["refPath"]
    return ref.replace("/Game/Maps/UEDPIE_0_L_NewGame", "/Game/Maps/L_NewGame")


def main() -> None:
    try:
        call_tool("EditorToolset.EditorAppToolset", "StopPIE", {})
    except RuntimeError as exc:
        if "not currently running" not in str(exc):
            raise

    actor = find_actor("MonsterGen_Sunken")
    comp = call_tool(
        "editor_toolset.toolsets.actor.ActorTools",
        "get_components",
        {"actor": {"refPath": actor}},
    )["returnValue"][0]["refPath"]

    call_tool(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {
            "instance": {"refPath": comp},
            "values": json.dumps(
                {
                    "StaticMesh": {"refPath": MESH_WATER},
                }
            ),
        },
    )
    call_tool(
        "editor_toolset.toolsets.actor.ActorTools",
        "set_actor_transform",
        {
            "actor": {"refPath": actor},
            "xform": {
                "location": {"x": 300.0, "y": 300.0, "z": 12.0},
                "rotation": {"pitch": 0.0, "yaw": 25.0, "roll": 0.0},
                "scale": {"x": 0.22, "y": 0.22, "z": 0.35},
            },
        },
    )

    call_tool(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {"instance": {"refPath": actor}, "values": json.dumps({"bHidden": False})},
    )

    bounds = call_tool(
        "editor_toolset.toolsets.actor.ActorTools",
        "get_actor_bounds",
        {"actor": {"refPath": actor}},
    )
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
    print("MonsterGen_Sunken -> raised water pool", bounds)
    print("Stop, then Play — pick Sunken Ruins.")


if __name__ == "__main__":
    main()
