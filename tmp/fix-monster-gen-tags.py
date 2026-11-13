import json
import urllib.request

SID_PATH = r"e:\Dungeon Core\tmp\mcp-sid.txt"
MCP_URL = "http://127.0.0.1:8000/mcp"
ACTOR = "editor_toolset.toolsets.actor.ActorTools"
SCENE = "editor_toolset.toolsets.scene.SceneTools"
ASSET = "editor_toolset.toolsets.asset.AssetTools"


def sid() -> str:
    return open(SID_PATH, encoding="utf-8").read().strip()


def rawcall(name: str, arguments: dict, timeout: int = 120):
    body = json.dumps(
        {
            "jsonrpc": "2.0",
            "id": 1,
            "method": "tools/call",
            "params": {"name": name, "arguments": arguments},
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


def call_tool(toolset: str, tool: str, args: dict):
    return rawcall(
        "call_tool",
        {"toolset_name": toolset, "tool_name": tool, "arguments": args},
    )


def describe_actor_tools():
    data = rawcall("describe_toolset", {"toolset_name": ACTOR})
    for tool in data["tools"]:
        short = tool["name"].split(".")[-1]
        if any(k in short.lower() for k in ("tag", "hidden", "property", "folder")):
            print(short, (tool.get("description") or "")[:160])


def list_generators():
    folders = [
        "NewGame/Dressing/Gothic",
        "NewGame/Dressing/GoblinTunnels",
        "NewGame/Dressing/FungalWarren",
        "NewGame/Dressing/SunkenRuins",
    ]
    for folder in folders:
        actors = call_tool(SCENE, "get_actors_in_folder", {"folder_path": folder})
        print(f"\n=== {folder} ===")
        if isinstance(actors, dict) and "actors" in actors:
            items = actors["actors"]
        else:
            items = actors
        for item in items or []:
            label = item.get("label") or item.get("name") or item
            ref = item.get("refPath") or item.get("actor_path") or item
            if isinstance(label, str) and "MonsterGen" in label:
                props = call_tool(
                    ACTOR,
                    "get_actor_properties",
                    {"actor_path": ref if isinstance(ref, str) else label},
                )
                print(label, json.dumps(props, indent=2)[:2000])


if __name__ == "__main__":
    describe_actor_tools()
    list_generators()
