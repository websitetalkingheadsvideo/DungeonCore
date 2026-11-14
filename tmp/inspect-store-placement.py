import json
import urllib.request

SID = open(r"e:\Dungeon Core\tmp\mcp-sid.txt", encoding="utf-8").read().strip()
URL = "http://127.0.0.1:8000/mcp"


def call(ts, tool, args):
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {
                "name": "call_tool",
                "arguments": {"toolset_name": ts, "tool_name": tool, "arguments": args},
            },
        }
    ).encode()
    req = urllib.request.Request(
        URL,
        data=body,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Mcp-Session-Id": SID,
        },
    )
    text = json.loads(urllib.request.urlopen(req, timeout=300).read())["result"]["content"][0]["text"]
    if text.startswith("Function") or text.startswith("Unknown"):
        raise RuntimeError(text[:800])
    return json.loads(text) if text.strip() else {}


def dump_graph(bp_path: str, graph_name: str) -> None:
    gref = {"refPath": f"{bp_path}:{graph_name}"}
    nodes = call(
        "editor_toolset.toolsets.blueprint.BlueprintTools",
        "find_nodes",
        {"graph": gref, "pattern": ".*"},
    )
    print(f"\n=== {bp_path} :: {graph_name} ({len(nodes.get('returnValue', nodes))} nodes) ===")
    for n in nodes.get("returnValue", nodes)[:80]:
        title = n.get("title") or n.get("nodeTitle") or n.get("name")
        cls = n.get("class") or n.get("nodeClass")
        print(f"  {title} | {cls}")


HUD = "/Game/UI/WBP_DungeonHUD.WBP_DungeonHUD"
GRID = "/Game/Core/BP_DungeonGrid.BP_DungeonGrid"

for fn in ["GetPressedItem", "GetHoveredItem", "Highlight", "CanAffordItem"]:
    dump_graph(HUD, fn)

for fn in ["TickStore", "UpdateDrag", "TryPlace", "BeginPlay", "EventGraph"]:
    try:
        dump_graph(GRID, fn)
    except Exception as exc:
        print("skip", fn, exc)

# button press method
ob = "editor_toolset.toolsets.object.ObjectTools"
for i in (0, 1):
    props = call(
        ob,
        "get_properties",
        {
            "instance": {"refPath": f"{HUD}:WidgetTree.Btn_{i}"},
            "properties": ["pressMethod", "clickMethod", "visibility", "isEnabled"],
        },
    )
    print(f"Btn_{i}", props.get("returnValue", props))
