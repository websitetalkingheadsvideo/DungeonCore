import json
import re
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
BT = "editor_toolset.toolsets.blueprint.BlueprintTools"
EDITOR = "EditorToolset.EditorAppToolset"
PATTERN = "/Game/Maps/Dev/BP_GalleryPawn.BP_GalleryPawn"


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def call_tool(toolset: str, tool: str, args: dict) -> dict:
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
                    "arguments": args,
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
        raise RuntimeError(text[:4000])
    return json.loads(text)


def patch_event_tick(dsl: str) -> str:
    dsl = dsl.replace(
        '  (bind _returnvalue_8 (Game|Player|IsInputKeyDown _returnvalue "LeftShift"))\n',
        '  (bind _mmbdown (Game|Player|IsInputKeyDown _returnvalue "MiddleMouseButton"))\n',
    )
    old_look = """  (if (Utilities|IsValid _returnvalue)
    (if (and _returnvalue_8 (Game|Player|IsInputKeyDown _returnvalue "LeftMouseButton"))
      (Variables|Default|SetDesiredRot _returnvalue_7)
      (Transformation|SetActorRotation _returnvalue_7)
      (if (not _lookactive)
        (Development|PrintString \"GALLERY_LOOK start\" false)
        (Variables|Default|SetLookActive true))
      (elif _lookactive
        (Variables|Default|SetLookActive false)
        (Development|PrintString \"GALLERY_LOOK stop\" false)))
    (else
      (Development|PrintString \"GALLERY_LOOK no valid player controller\" false))"""

    new_look = """  (if (Utilities|IsValid _returnvalue)
    (if _mmbdown
      (if (Utilities|IsValid _selectedactor)
        (bind _orbitpivot (Math|Vector|MakeVector _x_1 _y_1 (+ _z_1 (* _z 0.1))))
        (bind _reloffset (Math|Vector|vector-_vector (Variables|Default|GetDesiredLoc) _orbitpivot))
        (bind _relyaw (Math|Vector|RotateVectorAroundAxis _reloffset (Math|Vector|MakeVector 0.0 0.0 1.0) (* _deltax _looksensitivity)))
        (bind _orbitright (Math|Vector|Normalize (Math|Vector|CrossProduct (Math|Vector|MakeVector 0.0 0.0 1.0) (Math|Vector|Normalize _relyaw))))
        (bind _relpitch (Math|Vector|RotateVectorAroundAxis _relyaw _orbitright (- (* _deltay _looksensitivity))))
        (bind _newcam (Math|Vector|vector+vector _orbitpivot _relpitch))
        (bind _lookrot (Math|Rotator|FindLookAtRotation _newcam _orbitpivot))
        (Variables|Default|SetDesiredLoc _newcam)
        (Variables|Default|SetDesiredRot _lookrot)
        (else
          (Variables|Default|SetDesiredRot _returnvalue_7)
          (Transformation|SetActorRotation _returnvalue_7)))
      (if (not _lookactive)
        (Development|PrintString \"GALLERY_LOOK start\" false)
        (Variables|Default|SetLookActive true)))
    (elif _lookactive
      (Variables|Default|SetLookActive false)
      (Development|PrintString \"GALLERY_LOOK stop\" false))
    (else
      (Development|PrintString \"GALLERY_LOOK no valid player controller\" false))"""

    if old_look not in dsl:
        raise RuntimeError("EventTick look block not found — graph may have changed")
    dsl = dsl.replace(old_look, new_look)

    dsl = dsl.replace(
        '(if (and (Game|Player|WasInputKeyJustPressed _returnvalue_9 "LeftMouseButton") (not _returnvalue_8))',
        '(if (Game|Player|WasInputKeyJustPressed _returnvalue_9 "LeftMouseButton")',
    )
    return dsl


def patch_update_ring(dsl: str) -> str:
    old_tint = (
        '(Rendering|Material|SetVectorParameterValueOnMaterials _galleryring "Tint" '
        "(select (== (Math|Integer|%(Integer) _canonicalindex 4) 0) "
        "(Math|Vector|MakeVector 0.2 1 0.25) "
        '(select (== (Math|Integer|%(Integer) _canonicalindex 4) 1) '
        "(Math|Vector|MakeVector 0.25 0.55 1) "
        '(select (== (Math|Integer|%(Integer) _canonicalindex 4) 2) '
        "(Math|Vector|MakeVector 0.8 0.3 1) "
        '(Math|Vector|MakeVector 1 0.75 0.1)))))'
    )
    new_tint = (
        '(Rendering|Material|SetVectorParameterValueOnMaterials _galleryring "Tint" '
        "(Math|Vector|MakeVector 1.0 0.84 0.0))"
    )
    if old_tint not in dsl:
        raise RuntimeError("UpdateGalleryRing tint block not found")
    return dsl.replace(old_tint, new_tint)


def main() -> None:
    print("StopPIE", call_tool(EDITOR, "StopPIE", {}))
    ev_graph = {"refPath": f"{PATTERN}:EventGraph"}
    ring_graph = {"refPath": f"{PATTERN}:UpdateGalleryRing"}

    ev_dsl = call_tool(BT, "read_graph_dsl", {"graph": ev_graph})["returnValue"]
    ring_dsl = call_tool(BT, "read_graph_dsl", {"graph": ring_graph})["returnValue"]

    ev_patched = patch_event_tick(ev_dsl)
    ring_patched = patch_update_ring(ring_dsl)

    print(
        "write EventGraph",
        call_tool(
            BT,
            "write_graph_dsl",
            {"graph": ev_graph, "code": ev_patched},
        ),
    )
    print(
        "write UpdateGalleryRing",
        call_tool(
            BT,
            "write_graph_dsl",
            {"graph": ring_graph, "code": ring_patched},
        ),
    )
    print(
        "compile",
        call_tool(
            BT,
            "compile_blueprint",
            {"blueprint": {"refPath": f"{PATTERN}.BP_GalleryPawn"}},
        ),
    )


if __name__ == "__main__":
    main()