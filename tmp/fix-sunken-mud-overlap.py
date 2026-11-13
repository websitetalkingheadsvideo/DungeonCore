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


def find_name(name: str) -> str:
    return call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": name, "tag": "", "collision_channels": []},
    )["returnValue"][0]["refPath"]


def set_loc(actor_ref: str, x: float, y: float, z: float) -> None:
    ac = "editor_toolset.toolsets.actor.ActorTools"
    xf = call_tool(ac, "get_actor_transform", {"actor": {"refPath": actor_ref}})[
        "returnValue"
    ]
    xf["location"] = {"x": x, "y": y, "z": z}
    call_tool(ac, "set_actor_transform", {"actor": {"refPath": actor_ref}, "xform": xf})


def main() -> None:
    try:
        call_tool("EditorToolset.EditorAppToolset", "StopPIE", {})
    except RuntimeError as exc:
        if "not currently running" not in str(exc).lower():
            raise

    set_loc(find_name("Mud_Back_2"), 380.0, 300.0, 0.0)
    set_loc(find_name("Mud_Right_8"), 300.0, 380.0, 0.0)

    sunken = find_name("MonsterGen_Sunken")
    set_loc(sunken, 300.0, 300.0, 8.0)

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
    print("Mud nudged; Sunken gen Z=8 (tomb mesh from prior fix)")


if __name__ == "__main__":
    main()
