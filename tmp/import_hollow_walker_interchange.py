"""Interchange import: Hollow Walker animated GLB with skeletal mesh + anims."""
import json
import unreal

GLB = (
    r"E:/Dungeon Core/imports/fab/Mutants/Hollow_Walker/glb/"
    r"the_hollow_walker_asset__extracted/The_Hollow_Walker_Asset_Pack/"
    r"01_Animated_GLB/Meshy_AI_The_Hollow_Walker_All_Animations.glb"
)
DEST = "/Game/Monsters/Mutants/Hollow_Walker"


def list_folder():
    return unreal.EditorAssetLibrary.list_assets(DEST, recursive=True, include_folder=False)


def anim_sequences(paths):
    out = []
    for path in paths:
        data = unreal.EditorAssetLibrary.find_asset_data(path)
        if data.asset_class_path.asset_name == "AnimSequence":
            out.append(path)
    return out


def run():
    out = {"glb": GLB, "dest": DEST, "before": list_folder()}
    manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    source = manager.create_source_data(GLB)

    params = unreal.ImportAssetParameters()
    params.is_automated = True
    params.replace_existing = True

    pipeline = unreal.InterchangeGenericAssetsPipeline()
    pipeline.common_meshes_properties.force_all_mesh_as_type = (
        unreal.InterchangeForceMeshType.IFMT_SKELETAL_MESH
    )
    pipeline.common_skeletal_meshes_and_animations_properties.import_only_animations = False
    pipeline.common_meshes_properties.convert_statics_in_bone_hierarchy_to_skeletals = True

    mesh = pipeline.mesh_pipeline
    mesh.import_skeletal_meshes = True
    mesh.import_static_meshes = False
    mesh.create_physics_asset = True

    anim = pipeline.animation_pipeline
    anim.import_animations = True
    anim.import_bone_tracks = True

    mat = pipeline.material_pipeline
    mat.import_materials = True

    params.override_pipelines = [unreal.SoftObjectPath(pipeline.get_path_name())]

    imported = manager.import_asset(DEST, source, params)
    out["imported_objects"] = [obj.get_path_name() for obj in (imported or [])]
    after = list_folder()
    out["after"] = after
    out["anim_sequences"] = anim_sequences(after)
    unreal.EditorAssetLibrary.save_directory(DEST, only_if_is_dirty=False, recursive=True)
    return out


result = run()
Path = __import__("pathlib").Path
Path(r"E:/Dungeon Core/tmp/hollow-walker-interchange-result.json").write_text(
    json.dumps(result, indent=2),
    encoding="utf-8",
)
print(json.dumps(result, indent=2))
result