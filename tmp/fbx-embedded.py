import os

ROOT = r"E:\Dungeon Core\imports\blender"
files = [
    "Argiope_v3.fbx",
    "Blue Tarantula.fbx",
    "Karakurt.fbx",
    "Tarantula.fbx",
    "Peacock.fbx",
    "Spider-Yellow.fbx",
]
for name in files:
    data = open(os.path.join(ROOT, name), "rb").read()
    print(
        f"{name:22} png={data.find(bytes([137, 80, 78, 71])) >= 0} "
        f"jpeg={data.find(bytes([255, 216, 255])) >= 0} verts={data.find(b'Vertices') >= 0}"
    )
