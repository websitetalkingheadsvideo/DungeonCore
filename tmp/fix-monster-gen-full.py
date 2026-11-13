import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"

APPLY_DRESSING_DSL = """(fn ApplyDressing ()
  (bind _outactors (Actor|GetAllActorsWithTag "Dress"))
  (for _array_element _outactors
    (Rendering|SetActorHiddenInGame _array_element true))
  (bind _outactors_1 (Actor|GetAllActorsWithTag "ThemePillar"))
  (bind _outactors_2 (Actor|GetAllActorsWithTag "Dress_0"))
  (bind _outactors_3 (Actor|GetAllActorsWithTag "Dress_1"))
  (bind _outactors_4 (Actor|GetAllActorsWithTag "Dress_2"))
  (bind _outactors_5 (Actor|GetAllActorsWithTag "Dress_3"))
  (bind _themeindex (Variables|Default|GetThemeIndex))
  (switch Utilities|FlowControl|Switch|SwitchonInt _themeindex
    (:0
      (for _array_element_1 _outactors_2
        (Rendering|SetActorHiddenInGame _array_element_1 false))
      (for _array_element_2 _outactors_1
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_2 "/Script/Engine.StaticMeshComponent") 0 "/Game/Props/Gothic_props_1/Materials/M_Pillar1.M_Pillar1")))
    (:1
      (for _array_element_1 _outactors_3
        (Rendering|SetActorHiddenInGame _array_element_1 false)))
    (:2
      (for _array_element_1 _outactors_4
        (Rendering|SetActorHiddenInGame _array_element_1 false))
      (for _array_element_2 _outactors_1
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_2 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/AncientCrypt/Materials/M_WA_Inst2.M_WA_Inst2")))
    (:3
      (for _array_element_1 _outactors_5
        (Rendering|SetActorHiddenInGame _array_element_1 false))
      (for _array_element_2 _outactors_1
        (Rendering|Material|SetMaterial (Actor|GetComponentByClass _array_element_2 "/Script/Engine.StaticMeshComponent") 0 "/Game/Environments/AncientCrypt/Materials/M_WA_Inst.M_WA_Inst")))
    (:Default)))"""


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def call_tool(toolset: str, tool: str, arguments: dict, timeout: int = 180):
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
    raw = json.loads(urllib.request.urlopen(req, timeout=timeout).read().decode())
    text = raw["result"]["content"][0]["text"]
    if raw["result"].get("isError"):
        raise RuntimeError(text)
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        return text


def persistent_ref(ref_path: str) -> str:
    return ref_path.replace("/Game/Maps/UEDPIE_0_L_NewGame", "/Game/Maps/L_NewGame")


def main() -> None:
    try:
        print("StopPIE", call_tool("EditorToolset.EditorAppToolset", "StopPIE", {}))
    except RuntimeError as exc:
        if "not currently running" not in str(exc):
            raise

    graph = call_tool(
        "editor_toolset.toolsets.blueprint.BlueprintTools",
        "get_graph",
        {
            "blueprint": {"refPath": "/Game/UI/WBP_NewGame.WBP_NewGame"},
            "graph_name": "ApplyDressing",
        },
    )
    write = call_tool(
        "editor_toolset.toolsets.blueprint.BlueprintTools",
        "write_graph_dsl",
        {"graph": graph["returnValue"], "code": APPLY_DRESSING_DSL},
    )
    print("write_graph_dsl", write)

    refs = call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": "MonsterGen", "tag": "", "collision_channels": []},
    )["returnValue"]
    for item in refs:
        ref = persistent_ref(item["refPath"])
        call_tool(
            "editor_toolset.toolsets.object.ObjectTools",
            "set_properties",
            {
                "instance": {"refPath": ref},
                "values": json.dumps({"bHidden": False}),
            },
        )
        if not call_tool(
            "editor_toolset.toolsets.actor.ActorTools",
            "has_tag",
            {"actor": {"refPath": ref}, "tag": "MonsterGen"},
        ).get("returnValue"):
            call_tool(
                "editor_toolset.toolsets.actor.ActorTools",
                "add_tag",
                {"actor": {"refPath": ref}, "tag": "MonsterGen"},
            )
        tags = call_tool(
            "editor_toolset.toolsets.actor.ActorTools",
            "get_tags",
            {"actor": {"refPath": ref}},
        )["returnValue"]
        label = call_tool(
            "editor_toolset.toolsets.actor.ActorTools",
            "get_label",
            {"actor": {"refPath": ref}},
        )["returnValue"]
        print(label, tags)

    call_tool(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {
            "asset_paths": [
                "/Game/UI/WBP_NewGame",
                "/Game/Maps/L_NewGame",
            ]
        },
    )
    save_key = call_tool(
        "SlateInspectorToolset.SlateInspectorToolset",
        "PressKey",
        {"key": "Ctrl+Shift+S"},
    )
    print("SaveAll", save_key)


if __name__ == "__main__":
    main()
