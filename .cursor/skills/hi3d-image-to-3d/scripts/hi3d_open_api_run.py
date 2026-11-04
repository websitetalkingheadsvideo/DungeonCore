from __future__ import annotations

import argparse
import base64
import json
import os
import struct
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

API_BASE = "https://api.hitem3d.ai"
TOKEN_PATH = "/open-api/v1/auth/token"
SUBMIT_PATH = "/open-api/v1/submit-task"
QUERY_PATH = "/open-api/v1/query-task"


def env_credentials() -> tuple[str, str]:
    access = os.environ.get("HI3D_ACCESS_KEY") or os.environ.get("HI3D_CLIENT_ID")
    secret = os.environ.get("HI3D_SECRET_KEY") or os.environ.get("HI3D_CLIENT_SECRET")
    if not access or not secret:
        raise RuntimeError(
            "Set HI3D_ACCESS_KEY and HI3D_SECRET_KEY (platform.hi3d.ai → API keys), "
            "then rerun. Cursor MCP OAuth cannot set Open API face count."
        )
    return access, secret


def fetch_bearer_token(access: str, secret: str) -> str:
    credentials = base64.b64encode(f"{access}:{secret}".encode("utf-8")).decode("ascii")
    req = urllib.request.Request(
        f"{API_BASE}{TOKEN_PATH}",
        method="POST",
        headers={
            "Authorization": f"Basic {credentials}",
            "Content-Type": "application/json",
        },
        data=b"{}",
    )
    with urllib.request.urlopen(req, timeout=60) as resp:
        payload = json.loads(resp.read().decode("utf-8"))
    if payload.get("code") != 200:
        raise RuntimeError(json.dumps(payload, ensure_ascii=False))
    token = payload.get("data", {}).get("accessToken")
    if not isinstance(token, str):
        raise RuntimeError("Token response missing accessToken")
    return token


def submit_image_to_3d(
    token: str,
    image: Path,
    face: int,
    resolution: str,
    model: str,
) -> str:
    boundary = f"----DungeonCoreHi3d{int(time.time())}"
    image_bytes = image.read_bytes()
    suffix = image.suffix.lower()
    mime = "image/png" if suffix == ".png" else "image/jpeg" if suffix in (".jpg", ".jpeg") else "image/webp"

    fields: list[tuple[str, str]] = [
        ("request_type", "3"),
        ("model", model),
        ("resolution", resolution),
        ("face", str(face)),
        ("pbr", "1"),
        ("shading", "0.5"),
        ("format", "2"),
    ]
    body = bytearray()
    for name, value in fields:
        body.extend(f"--{boundary}\r\n".encode("utf-8"))
        body.extend(f'Content-Disposition: form-data; name="{name}"\r\n\r\n'.encode("utf-8"))
        body.extend(value.encode("utf-8"))
        body.extend(b"\r\n")
    body.extend(f"--{boundary}\r\n".encode("utf-8"))
    body.extend(
        f'Content-Disposition: form-data; name="images"; filename="{image.name}"\r\n'.encode("utf-8")
    )
    body.extend(f"Content-Type: {mime}\r\n\r\n".encode("utf-8"))
    body.extend(image_bytes)
    body.extend(b"\r\n")
    body.extend(f"--{boundary}--\r\n".encode("utf-8"))

    req = urllib.request.Request(
        f"{API_BASE}{SUBMIT_PATH}",
        method="POST",
        headers={
            "Authorization": f"Bearer {token}",
            "Content-Type": f"multipart/form-data; boundary={boundary}",
        },
        data=bytes(body),
    )
    with urllib.request.urlopen(req, timeout=120) as resp:
        payload = json.loads(resp.read().decode("utf-8"))
    if payload.get("code") != 200:
        raise RuntimeError(json.dumps(payload, ensure_ascii=False))
    task_id = payload.get("data", {}).get("task_id")
    if not isinstance(task_id, str):
        raise RuntimeError("submit-task missing task_id")
    return task_id


def query_task(token: str, task_id: str) -> dict[str, object]:
    url = f"{API_BASE}{QUERY_PATH}?task_id={urllib.parse.quote(task_id)}"
    req = urllib.request.Request(url, headers={"Authorization": f"Bearer {token}"})
    with urllib.request.urlopen(req, timeout=60) as resp:
        payload = json.loads(resp.read().decode("utf-8"))
    if payload.get("code") != 200:
        raise RuntimeError(json.dumps(payload, ensure_ascii=False))
    data = payload.get("data")
    if not isinstance(data, dict):
        raise RuntimeError(json.dumps(payload, ensure_ascii=False))
    return data


def pick_glb_url(data: dict[str, object]) -> str:
    for key in ("model_url", "url", "glb_url", "modelUrl"):
        value = data.get(key)
        if isinstance(value, str) and (".glb" in value.lower() or "/model/" in value):
            return value
    files = data.get("files")
    if isinstance(files, list):
        for item in files:
            if not isinstance(item, dict):
                continue
            url = item.get("url")
            name = str(item.get("name", ""))
            if isinstance(url, str) and (name.endswith(".glb") or "/model/original/" in url):
                return url
    nested = data.get("result")
    if isinstance(nested, dict):
        return pick_glb_url(nested)
    raise RuntimeError(json.dumps(data, ensure_ascii=False)[:800])


def count_triangles(glb_path: Path) -> int:
    data = glb_path.read_bytes()
    jlen = struct.unpack_from("<I", data, 12)[0]
    j = json.loads(data[20 : 20 + jlen])
    blob = data[20 + jlen + 8 :]
    comp_bytes = {5121: 1, 5123: 2, 5125: 4}
    tris = 0
    for mesh in j.get("meshes", []):
        for pr in mesh.get("primitives", []):
            if pr.get("mode", 4) != 4:
                continue
            idx_acc = j["accessors"][pr["indices"]]
            bv = j["bufferViews"][idx_acc["bufferView"]]
            start = bv.get("byteOffset", 0) + idx_acc.get("byteOffset", 0)
            comp = {5121: 1, 5123: 2, 5125: 4}[idx_acc["componentType"]]
            nbytes = idx_acc["count"] * comp_bytes[idx_acc["componentType"]]
            tris += idx_acc["count"] // 3
    return tris


def download_url(url: str, dest: Path) -> None:
    dest.parent.mkdir(parents=True, exist_ok=True)
    urllib.request.urlretrieve(url, dest)


def main() -> int:
    parser = argparse.ArgumentParser(description="Hi3D Open API image-to-3D with face count")
    parser.add_argument("--image", required=True)
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--glb-name", required=True)
    parser.add_argument("--slug", required=True)
    parser.add_argument("--face", type=int, default=100000)
    parser.add_argument("--resolution", default="2048quality")
    parser.add_argument("--model", default="hi3dv3.0")
    parser.add_argument("--timeout-seconds", type=int, default=1800)
    args = parser.parse_args()

    access, secret = env_credentials()
    token = fetch_bearer_token(access, secret)
    image = Path(args.image)
    task_id = submit_image_to_3d(token, image, args.face, args.resolution, args.model)
    deadline = time.time() + args.timeout_seconds
    last: dict[str, object] = {}
    while time.time() < deadline:
        last = query_task(token, task_id)
        status = str(last.get("status", last.get("state", ""))).lower()
        if status in ("success", "succeeded", "failed", "error"):
            break
        time.sleep(max(10, int(last.get("poll_interval") or 15)))
    if str(last.get("status", last.get("state", ""))).lower() not in ("success", "succeeded"):
        print(json.dumps(last, indent=2))
        return 1

    glb_url = pick_glb_url(last)
    out_dir = Path(args.out_dir)
    glb_path = out_dir / args.glb_name
    download_url(glb_url, glb_path)
    tris = count_triangles(glb_path)
    manifest = {
        "slug": args.slug,
        "task_id": task_id,
        "source_image": str(image.resolve()),
        "face_requested": args.face,
        "exported_triangle_faces": tris,
        "api": "open-api",
        "resolution": args.resolution,
        "model": args.model,
        "files": [{"name": glb_path.name, "path": glb_path.name}],
    }
    (out_dir / f"{args.slug}_open_api_manifest.json").write_text(
        json.dumps(manifest, indent=2),
        encoding="utf-8",
    )
    print(json.dumps({"task_id": task_id, "glb": str(glb_path), "triangle_faces": tris}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
