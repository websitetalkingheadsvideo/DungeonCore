import os
import re

ROOT = r"E:\Dungeon Core\imports\blender"
for name in ("Argiope_v3.fbx", "Blue Tarantula.fbx", "Karakurt.fbx", "Tarantula.fbx", "Peacock.fbx", "Spider-Yellow.fbx"):
    data = open(os.path.join(ROOT, name), "rb").read()
    print("====", name)
    for match in re.finditer(rb"[A-Za-z]:\\[ -~]{5,180}\.(?:png|jpg|jpeg)", data):
        print(" ", match.group().decode("ascii", "ignore"))
    for match in re.finditer(rb"[ -~]{3,120}Textures\\[ -~]{3,80}\.(?:png|jpg)", data):
        text = match.group().decode("ascii", "ignore")
        if "Rotation" not in text:
            print("  rel", text)
