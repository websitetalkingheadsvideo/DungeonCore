"""Wire elite_orc showcase idle on L_NewGame Creature_0 actor."""
import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"

IDLE_ANIM = "/Game/Monsters/Orcs/elite_orc0_Open_A_UE5.elite_orc0_Open_A_UE5"


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
    try:
        call_tool("EditorToolset.EditorAppToolset", "StopPIE", {})
    except RuntimeError as exc:
        if "not currently running" not in str(exc).lower():
            raise

    ac = "editor_toolset.toolsets.actor.ActorTools"
    ob = "editor_toolset.toolsets.object.ObjectTools"
    ref = call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": "elite_orc", "tag": "", "collision_channels": []},
    )["returnValue"][0]["refPath"]
    ref = ref.replace("/Game/Maps/UEDPIE_0_L_NewGame", "/Game/Maps/L_NewGame")
    comp = call_tool(ac, "get_components", {"actor": {"refPath": ref}})["returnValue"][0][
        "refPath"
    ]

    call_tool(
        ob,
        "set_properties",
        {
            "instance": {"refPath": comp},
            "values": json.dumps(
                {
                    "AnimationMode": "AnimationSingleNode",
                    "AnimClass": None,
                    "AnimationData": {
                        "AnimToPlay": {"refPath": IDLE_ANIM},
                        "bSavedLooping": True,
                        "bSavedPlaying": True,
                        "SavedPosition": 0.0,
                        "SavedPlayRate": 1.0,
                    },
                }
            ),
        },
    )

    verify = json.loads(
        call_tool(
            ob,
            "get_properties",
            {
                "instance": {"refPath": comp},
                "properties": ["AnimationMode", "AnimationData", "AnimClass"],
            },
        )["returnValue"]
    )
    print("elite_orc anim", verify)

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
    print("Stop then Play — default Orcs row should idle loop.")


if __name__ == "__main__":
    main()
