"""Patch TryPlace + RotateItemTo DSL for Drag Lattice #15."""
from __future__ import annotations

from pathlib import Path

SAVED = Path(r"e:\Dungeon Core\DungeonCore\Saved")

TRY_MIRROR_OLD = (
    "(select (== _selecteditem 14) (Variables|Default|GetMirrorPillars) "
    "(Variables|Default|GetItems))"
)
TRY_MIRROR_NEW = (
    "(select (== _selecteditem 14) (Variables|Default|GetMirrorPillars) "
    "(select (== _selecteditem 15) (Variables|Default|GetDragLattices) "
    "(Variables|Default|GetItems)))"
)

ROT_TAIL_OLD = (
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 15) "
    "(Variables|Default|GetMirrorPillars) (Variables|Default|GetItems))"
)
ROT_TAIL_NEW = (
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 15) "
    "(Variables|Default|GetMirrorPillars) "
    "(select (== (Utilities|Array|Get(acopy) (Variables|Default|GetPlaced) Idx) 16) "
    "(Variables|Default|GetDragLattices) (Variables|Default|GetItems))"
)


def balance_ok(code: str) -> bool:
    depth = 0
    for ch in code:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if depth < 0:
            return False
    return depth == 0


def patch_try_place(raw: str) -> str:
    code = raw.replace("(bind _returnvalue_14 14)", "(bind _returnvalue_14 15)", 1)
    if TRY_MIRROR_OLD not in code:
        raise RuntimeError("TryPlace: mirror select not found")
    code = code.replace(TRY_MIRROR_OLD, TRY_MIRROR_NEW, 1)
    needle = f"{TRY_MIRROR_NEW.split('GetItems')[-1]}"
    # After patch, add one closing paren before AddInstance transform args.
    anchor = f"{TRY_MIRROR_NEW}))))))))))))) (Math|Transform|MakeTransform"
    if anchor in code:
        code = code.replace(
            anchor,
            f"{TRY_MIRROR_NEW}))))))))))))))) (Math|Transform|MakeTransform",
            1,
        )
    elif f"{TRY_MIRROR_NEW}))))))))))))) (Math|Transform|MakeTransform" in code:
        code = code.replace(
            f"{TRY_MIRROR_NEW}))))))))))))) (Math|Transform|MakeTransform",
            f"{TRY_MIRROR_NEW}))))))))))))))) (Math|Transform|MakeTransform",
            1,
        )
    else:
        marker = "(Variables|Default|GetItems)"
        pos = code.index(marker)
        pos = code.index("(Math|Transform|MakeTransform", pos)
        code = code[:pos] + ")" + code[pos:]
    if not balance_ok(code):
        raise RuntimeError("TryPlace: unbalanced parens after patch")
    return code


def patch_rotate(raw: str) -> str:
    if ROT_TAIL_OLD not in raw:
        raise RuntimeError("RotateItemTo: tail not found")
    code = raw.replace(ROT_TAIL_OLD, ROT_TAIL_NEW, 1)
    anchor_old = f"{ROT_TAIL_NEW}))))))))))))) (Utilities|Array|Get"
    anchor_new = f"{ROT_TAIL_NEW}))))))))))))))) (Utilities|Array|Get"
    if anchor_old in code:
        code = code.replace(anchor_old, anchor_new, 1)
    else:
        marker = "(Variables|Default|GetItems)"
        pos = code.index(marker)
        pos = code.index("(Utilities|Array|Get", pos)
        code = code[:pos] + ")" + code[pos:]
    code = code.replace("Idx) 15))) 0.0", "Idx) 16))) 0.0")
    code = code.replace(
        "Idx) 15))) (Math|Vector|MakeVector 1.0",
        "Idx) 16))) (Math|Vector|MakeVector 1.0",
    )
    if not balance_ok(code):
        raise RuntimeError("RotateItemTo: unbalanced parens after patch")
    return code


def main() -> None:
    try_raw = Path(r"e:\Dungeon Core\tmp\tryplace-original.dsl")
    rot_raw = Path(r"e:\Dungeon Core\tmp\rotate-original.dsl")
    if not try_raw.exists():
        raise SystemExit("Save TryPlace read_graph output to tmp/tryplace-original.dsl first")

    try_place = patch_try_place(try_raw.read_text(encoding="utf-8"))
    rotate = patch_rotate(rot_raw.read_text(encoding="utf-8"))
    tint = Path(r"e:\Dungeon Core\tmp\tint-patched.dsl").read_text(encoding="utf-8")

    for name, body in (
        ("tryplace-patched.dsl", try_place),
        ("rotate-patched.dsl", rotate),
        ("tint-patched.dsl", tint),
    ):
        path = SAVED / name
        path.write_text(body, encoding="utf-8")
        print(name, "balance", balance_ok(body), "bytes", path.stat().st_size)


if __name__ == "__main__":
    main()