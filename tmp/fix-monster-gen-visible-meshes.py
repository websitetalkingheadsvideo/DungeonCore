import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"

MESH_MUD = "/Game/Environments/AncientCrypt/Meshes/SM_Dungeon_Mud_01a.SM_Dungeon_Mud_01a"
MESH_MUSHROOM = "/Game/Environments/GreenCave/Meshes/SM_LargeMushrooms_a.SM_LargeMushrooms_a"
MI_MUSHROOM = "/Game/Maps/Materials/MI_NG_MushroomGlow.MI_NG_MushroomGlow"
MI_MUD = "/Game/Environments/AncientCrypt/Materials/MI_Sewer_Mud_01a.MI_Sewer_Mud_01a"


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
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return text


def set_mesh(ref_path: str, mesh_path: str) -> None:
    call_tool(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {
            "instance": {"refPath": ref_path},
            "values": json.dumps(
                {"StaticMesh": {"refPath": mesh_path}}
            ),
        },
    )


def set_actor(actor_ref: str, location: dict, rotation: dict, scale: dict) -> None:
    call_tool(
        "editor_toolset.toolsets.actor.ActorTools",
        "set_actor_transform",
        {
            "actor": {"refPath": actor_ref},
            "xform": {
                "location": location,
                "rotation": rotation,
                "scale": scale,
            },
        },
    )


def find_actor(name: str) -> str:
    result = call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": name, "tag": "", "collision_channels": []},
    )["returnValue"][0]["refPath"]
    return result.replace("/Game/Maps/UEDPIE_0_L_NewGame", "/Game/Maps/L_NewGame")


def component_ref(actor_ref: str) -> str:
    comps = call_tool(
        "editor_toolset.toolsets.actor.ActorTools",
        "get_components",
        {"actor": {"refPath": actor_ref}},
    )["returnValue"]
    return comps[0]["refPath"]


def main() -> None:
    try:
        call_tool("EditorToolset.EditorAppToolset", "StopPIE", {})
    except RuntimeError as exc:
        if "not currently running" not in str(exc):
            raise

    sunken = find_actor("MonsterGen_Sunken")
    set_mesh(component_ref(sunken), MESH_MUD)
    set_actor(
        sunken,
        {"x": 300.0, "y": 300.0, "z": 0.0},
        {"pitch": 0.0, "yaw": 0.0, "roll": 0.0},
        {"x": 1.4, "y": 1.4, "z": 1.0},
    )

    fungal_primary = find_actor("MonsterGen_Fungal_0")
    set_mesh(component_ref(fungal_primary), MESH_MUSHROOM)
    set_actor(
        fungal_primary,
        {"x": 300.0, "y": 300.0, "z": 0.0},
        {"pitch": 0.0, "yaw": 0.0, "roll": 0.0},
        {"x": 1.3, "y": 1.3, "z": 1.3},
    )

    for suffix in ("1", "2", "3"):
        actor = find_actor(f"MonsterGen_Fungal_{suffix}")
        call_tool(
            "editor_toolset.toolsets.object.ObjectTools",
            "set_properties",
            {
                "instance": {"refPath": actor},
                "values": json.dumps({"bHidden": True}),
            },
        )

    for name in (
        "MonsterGen_Sunken",
        "MonsterGen_Fungal_0",
        "MonsterGen_Gothic",
        "MonsterGen_Goblin",
    ):
        actor = find_actor(name)
        call_tool(
            "editor_toolset.toolsets.object.ObjectTools",
            "set_properties",
            {
                "instance": {"refPath": actor},
                "values": json.dumps({"bHidden": False}),
            },
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
    print("Sunken -> mud mound; Fungal -> single glowing mushroom at (300,300); cobweb extras hidden")


if __name__ == "__main__":
    main()
