"""Clear actor bHidden on all MonsterGens; Sunken/Fungal get high-contrast showcase meshes."""
import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"

MESH_TOMB = (
    "/Game/Environments/AncientCrypt/Meshes/SM_Tomb_Crypt_Damage_Var_01."
    "SM_Tomb_Crypt_Damage_Var_01"
)
MESH_CAGE = "/Game/FantasyDungeon/Meshes/SM_Cage01.SM_Cage01"


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
        raise RuntimeError(text[:2000])
    return json.loads(text)


def persistent(ref: str) -> str:
    return ref.replace("/Game/Maps/UEDPIE_0_L_NewGame", "/Game/Maps/L_NewGame")


def find_by_name(name: str) -> str:
    ref = call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": name, "tag": "", "collision_channels": []},
    )["returnValue"][0]["refPath"]
    return persistent(ref)


def comp_ref(actor_ref: str) -> str:
    return call_tool(
        "editor_toolset.toolsets.actor.ActorTools",
        "get_components",
        {"actor": {"refPath": actor_ref}},
    )["returnValue"][0]["refPath"]


def set_mesh(actor_ref: str, mesh_path: str) -> None:
    call_tool(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {
            "instance": {"refPath": comp_ref(actor_ref)},
            "values": json.dumps({"StaticMesh": {"refPath": mesh_path}}),
        },
    )


def clear_hidden(actor_ref: str) -> None:
    call_tool(
        "editor_toolset.toolsets.object.ObjectTools",
        "set_properties",
        {"instance": {"refPath": actor_ref}, "values": json.dumps({"bHidden": False})},
    )


def set_xform(actor_ref: str, loc: dict, rot: dict, sc: dict) -> None:
    call_tool(
        "editor_toolset.toolsets.actor.ActorTools",
        "set_actor_transform",
        {
            "actor": {"refPath": actor_ref},
            "xform": {"location": loc, "rotation": rot, "scale": sc},
        },
    )


def main() -> None:
    try:
        call_tool("EditorToolset.EditorAppToolset", "StopPIE", {})
    except RuntimeError as exc:
        if "not currently running" not in str(exc).lower():
            raise

    for item in call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": "", "tag": "MonsterGen", "collision_channels": []},
    )["returnValue"]:
        actor = persistent(item["refPath"])
        clear_hidden(actor)
        label = call_tool(
            "editor_toolset.toolsets.actor.ActorTools",
            "get_label",
            {"actor": {"refPath": actor}},
        )["returnValue"]
        print("cleared bHidden", label)

    sunken = find_by_name("MonsterGen_Sunken")
    set_mesh(sunken, MESH_TOMB)
    set_xform(
        sunken,
        {"x": 300.0, "y": 300.0, "z": 0.0},
        {"pitch": 0.0, "yaw": 0.0, "roll": 0.0},
        {"x": 1.0, "y": 1.0, "z": 1.0},
    )

    fungal = find_by_name("MonsterGen_Fungal_0")
    set_xform(
        fungal,
        {"x": 300.0, "y": 300.0, "z": 0.0},
        {"pitch": 0.0, "yaw": 0.0, "roll": 0.0},
        {"x": 1.3, "y": 1.3, "z": 1.3},
    )

    for suffix in ("1", "2", "3"):
        fungal_extra = find_by_name(f"MonsterGen_Fungal_{suffix}")
        call_tool(
            "editor_toolset.toolsets.object.ObjectTools",
            "set_properties",
            {
                "instance": {"refPath": fungal_extra},
                "values": json.dumps({"bHidden": True}),
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

    # Creature reference for where the player looks
    for tag in ("Creature_0", "Creature_1"):
        refs = call_tool(
            "editor_toolset.toolsets.scene.SceneTools",
            "find_actors",
            {"name": "", "tag": tag, "collision_channels": []},
        )["returnValue"]
        for r in refs[:1]:
            actor = persistent(r["refPath"])
            loc = call_tool(
                "editor_toolset.toolsets.actor.ActorTools",
                "get_actor_transform",
                {"actor": {"refPath": actor}},
            )["returnValue"]["location"]
            print(tag, loc)

    bounds = call_tool(
        "editor_toolset.toolsets.actor.ActorTools",
        "get_actor_bounds",
        {"actor": {"refPath": sunken}},
    )
    print("Sunken tomb bounds", bounds)
    print("Stop then Play — compare Gothic vs Sunken at back-right (plan NE cell).")


if __name__ == "__main__":
    main()
