"""Import Hollow Walker into /Game/Monsters/Mutants/Hollow_Walker via SkeletalMeshTools.import_file."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(r"e:\Dungeon Core")
sys.path.insert(0, str(ROOT / "tmp"))
from mcp_session import call_tool, new_session  # noqa: E402

DEST = "/Game/Monsters/Mutants/Hollow_Walker"
ASSET_NAME = "SK_Hollow_Walker"
GLB = (
    ROOT
    / "imports/fab/Mutants/Hollow_Walker/glb/the_hollow_walker_asset__extracted"
    / "The_Hollow_Walker_Asset_Pack/01_Animated_GLB"
    / "Meshy_AI_The_Hollow_Walker_All_Animations.glb"
)
FBX = (
    ROOT
    / "imports/fab/Mutants/Hollow_Walker/glb/the_hollow_walker_asset__extracted"
    / "The_Hollow_Walker_Asset_Pack/02_Original_FBX"
    / "Meshy_AI_The_Hollow_Walker_1019171107_texture.fbx"
)


def ue_path(path: Path) -> str:
    return path.as_posix()


def call_json(toolset: str, tool: str, arguments: dict, sid: str) -> dict:
    out = call_tool(toolset, tool, arguments, sid)
    if isinstance(out, str):
        if out.startswith("Function") or out.startswith("Unknown"):
            raise RuntimeError(out)
        return json.loads(out)
    return out


def import_skeletal(source: Path, sid: str) -> list[dict]:
    return call_json(
        "editor_toolset.toolsets.skeletal_mesh.SkeletalMeshTools",
        "import_file",
        {
            "folder_path": DEST,
            "asset_name": ASSET_NAME,
            "source_file": ue_path(source),
            "import_materials": True,
            "import_textures": True,
            "import_animations": True,
            "create_physics_asset": True,
        },
        sid,
    ).get("returnValue", [])


def main() -> None:
    sid = new_session()
    call_json(
        "editor_toolset.toolsets.asset.AssetTools",
        "create_folder",
        {"path": DEST},
        sid,
    )
    sk_path = f"{DEST}/{ASSET_NAME}"
    results: dict = {"attempts": []}
    if call_json(
        "editor_toolset.toolsets.asset.AssetTools",
        "exists",
        {"path": sk_path},
        sid,
    ).get("returnValue"):
        results["already_exists"] = sk_path
    else:
        for label, src in (("glb", GLB), ("fbx", FBX)):
            if not src.is_file():
                results["attempts"].append({"label": label, "error": "missing", "path": str(src)})
                continue
            try:
                imported = import_skeletal(src, sid)
                results["attempts"].append({"label": label, "imported": imported})
                if call_json(
                    "editor_toolset.toolsets.asset.AssetTools",
                    "exists",
                    {"path": sk_path},
                    sid,
                ).get("returnValue"):
                    break
            except RuntimeError as exc:
                results["attempts"].append({"label": label, "error": str(exc)})

    found = call_json(
        "editor_toolset.toolsets.asset.AssetTools",
        "find_assets",
        {"folder_path": DEST, "name": "", "recursive": True},
        sid,
    ).get("returnValue", [])
    results["folder_assets"] = found
    call_json(
        "editor_toolset.toolsets.asset.AssetTools",
        "save_assets",
        {"asset_paths": []},
        sid,
    )

    manifest = ROOT / "tmp" / "hollow-walker-import-manifest.json"
    manifest.write_text(json.dumps(results, indent=2), encoding="utf-8")
    print(json.dumps(results, indent=2))


if __name__ == "__main__":
    main()