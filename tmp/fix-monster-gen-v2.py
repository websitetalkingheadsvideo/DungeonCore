import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"

APPLY_DRESSING_DSL = """(fn ApplyDressing ()
  (bind _monster_gens (Actor|GetAllActorsWithTag "MonsterGen"))
  (for _mg _monster_gens
    (Rendering|SetActorHiddenInGame _mg true))
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
        call_tool("EditorToolset.EditorAppToolset", "StopPIE", {})
    except RuntimeError as exc:
        if "not currently running" not in str(exc):
            raise

    bp = "editor_toolset.toolsets.blueprint.BlueprintTools"
    graph = call_tool(
        bp,
        "get_graph",
        {
            "blueprint": {"refPath": "/Game/UI/WBP_NewGame.WBP_NewGame"},
            "graph_name": "ApplyDressing",
        },
    )
    call_tool(
        bp,
        "write_graph_dsl",
        {"graph": graph["returnValue"], "code": APPLY_DRESSING_DSL},
    )

    refs = call_tool(
        "editor_toolset.toolsets.scene.SceneTools",
        "find_actors",
        {"name": "MonsterGen", "tag": "", "collision_channels": []},
    )["returnValue"]
    actor = "editor_toolset.toolsets.actor.ActorTools"
    obj = "editor_toolset.toolsets.object.ObjectTools"
    for item in refs:
        ref = persistent_ref(item["refPath"])
        if call_tool(actor, "has_tag", {"actor": {"refPath": ref}, "tag": "Dress"}).get(
            "returnValue"
        ):
            call_tool(actor, "remove_tag", {"actor": {"refPath": ref}, "tag": "Dress"})
        for tag_name in ("MonsterGen",):
            if not call_tool(
                actor, "has_tag", {"actor": {"refPath": ref}, "tag": tag_name}
            ).get("returnValue"):
                call_tool(
                    actor, "add_tag", {"actor": {"refPath": ref}, "tag": tag_name}
                )
        call_tool(
            obj,
            "set_properties",
            {
                "instance": {"refPath": ref},
                "values": json.dumps({"bHidden": False}),
            },
        )
        label = call_tool(actor, "get_label", {"actor": {"refPath": ref}})["returnValue"]
        tags = call_tool(actor, "get_tags", {"actor": {"refPath": ref}})["returnValue"]
        print(label, tags)

    call_tool(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {"asset_paths": ["/Game/UI/WBP_NewGame", "/Game/Maps/L_NewGame"]},
    )
    call_tool(
        "SlateInspectorToolset.SlateInspectorToolset",
        "PressKey",
        {"key": "Ctrl+Shift+S"},
    )
    print("done — restart PIE (Stop then Play) so WBP_NewGame recompiles")


if __name__ == "__main__":
    main()
