"""Build balanced TryPlace / RotateItemTo DSL for Drag Lattice."""
from __future__ import annotations

from pathlib import Path

SAVED = Path(r"e:\Dungeon Core\DungeonCore\Saved")
TMP = Path(r"e:\Dungeon Core\tmp")

TRY_OLD = (
    "(select (== _selecteditem 14) (Variables|Default|GetMirrorPillars) "
    "(Variables|Default|GetItems))"
)
TRY_NEW = (
    "(select (== _selecteditem 14) (Variables|Default|GetMirrorPillars) "
    "(select (== _selecteditem 15) (Variables|Default|GetDragLattices) "
    "(Variables|Default|GetItems)))"
)

ROT_OLD = (
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 15) "
    "(Variables|Default|GetMirrorPillars) (Variables|Default|GetItems))"
)
ROT_NEW = (
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 15) "
    "(Variables|Default|GetMirrorPillars) "
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 16) "
    "(Variables|Default|GetDragLattices) (Variables|Default|GetItems)))"
)


def balanced(code: str) -> bool:
    depth = 0
    for ch in code:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if depth < 0:
            return False
    return depth == 0


def insert_paren_before_marker(code: str, start_marker: str, next_marker: str) -> str:
    start = code.index(start_marker)
    end = code.index(next_marker, start)
    return code[:end] + ")" + code[end:]


def patch_try(raw: str) -> str:
    out = raw.replace("(bind _returnvalue_14 14)", "(bind _returnvalue_14 15)", 1)
    out = out.replace(TRY_OLD, TRY_NEW, 1)
    out = insert_paren_before_marker(
        out,
        "(bind _returnvalue_37",
        "(Utilities|Array|SetArrayElem (Variables|Default|GetItemInst)",
    )
    if not balanced(out):
        raise RuntimeError("TryPlace unbalanced")
    return out


def patch_rotate(raw: str) -> str:
    out = raw.replace(ROT_OLD, ROT_NEW, 1)
    out = insert_paren_before_marker(
        out,
        "(Components|InstancedStaticMesh|UpdateInstanceTransform",
        "(Utilities|Array|Get(acopy) (Variables|Default|GetItemInst)",
    )
    out = out.replace("Idx) 15))) 0.0", "Idx) 16))) 0.0")
    out = out.replace(
        "Idx) 15))) (Math|Vector|MakeVector 1.0",
        "Idx) 16))) (Math|Vector|MakeVector 1.0",
    )
    if not balanced(out):
        raise RuntimeError("RotateItemTo unbalanced")
    return out


def main() -> None:
    try_raw = (TMP / "tryplace-original.dsl").read_text(encoding="utf-8")
    rot_raw = (TMP / "rotate-original.dsl").read_text(encoding="utf-8")
    tint = (TMP / "tint-patched.dsl").read_text(encoding="utf-8")

    try_patched = patch_try(try_raw)
    rot_patched = patch_rotate(rot_raw)

    SAVED.mkdir(parents=True, exist_ok=True)
    (SAVED / "tryplace-patched.dsl").write_text(try_patched, encoding="utf-8")
    (SAVED / "rotate-patched.dsl").write_text(rot_patched, encoding="utf-8")
    (SAVED / "tint-patched.dsl").write_text(tint, encoding="utf-8")
    print("TryPlace OK", balanced(try_patched))
    print("RotateItemTo OK", balanced(rot_patched))


if __name__ == "__main__":
    main()