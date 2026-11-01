from __future__ import annotations

import argparse
import base64
import io
import json
import sys
import time
import urllib.request
from pathlib import Path

from PIL import Image

from hi3d_client import DEFAULT_LOCAL_STATE, DEFAULT_STATE_DB, connect_client

PNG_UPLOAD_FAIL_THRESHOLD_BYTES = 2_800_000
DEFAULT_VERSION = "v3.0"
DEFAULT_MODEL_TYPE = "high_quality"
DEFAULT_GENERATION_TYPE = "model_with_texture"


def prepare_upload_bytes(source: Path) -> tuple[bytes, str]:
    suffix = source.suffix.lower()
    if suffix in (".jpg", ".jpeg"):
        raw = source.read_bytes()
        return raw, "image/jpeg"
    if suffix == ".webp":
        raw = source.read_bytes()
        return raw, "image/webp"
    if suffix != ".png":
        raise ValueError(f"Unsupported image type: {source.suffix}")

    raw_png = source.read_bytes()
    if len(raw_png) <= PNG_UPLOAD_FAIL_THRESHOLD_BYTES:
        return raw_png, "image/png"

    image = Image.open(source).convert("RGBA")
    buffer = io.BytesIO()
    image.save(buffer, format="WEBP", lossless=True)
    webp_bytes = buffer.getvalue()
    return webp_bytes, "image/webp"


def upload_image(client, source: Path) -> tuple[str, dict[str, object]]:
    raw, mime_type = prepare_upload_bytes(source)
    encoded = base64.b64encode(raw).decode("ascii")
    result = client.call_tool(
        "upload_input",
        {"image_base64": encoded, "mime_type": mime_type},
    )
    if "file_token" not in result:
        raise RuntimeError(json.dumps(result, ensure_ascii=False))
    meta = {
        "source": str(source.resolve()),
        "mime_type": mime_type,
        "bytes": len(raw),
    }
    return str(result["file_token"]), meta


def quote_generation(
    client,
    file_token: str,
    prompt: str,
    version: str,
    model_type: str,
    generation_type: str,
    license_name: str,
    pbr: bool,
) -> dict[str, object]:
    return client.call_tool(
        "quote_generation",
        {
            "prompt": prompt,
            "file_tokens": [file_token],
            "version": version,
            "model_type": model_type,
            "generation_type": generation_type,
            "license": license_name,
            "pbr": pbr,
        },
    )


def create_generation(client, quote_id: str) -> dict[str, object]:
    return client.call_tool(
        "create_generation",
        {"quote_id": quote_id, "confirmed": True},
    )


def poll_task(
    client,
    task_id: str,
    timeout_seconds: int,
) -> dict[str, object]:
    deadline = time.time() + timeout_seconds
    last: dict[str, object] = {}
    while time.time() < deadline:
        last = client.call_tool("get_task", {"task_id": task_id})
        status = str(last.get("status", ""))
        if status in ("succeeded", "failed", "error", "cancelled"):
            return last
        wait = int(last.get("next_poll_after") or 15)
        time.sleep(max(10, wait))
    raise TimeoutError(f"Task {task_id} still running after {timeout_seconds}s: {last.get('stage')}")


def pick_glb_url(outputs: dict[str, object]) -> str:
    assets = outputs.get("assets")
    if not isinstance(assets, list):
        raise RuntimeError("get_task_outputs returned no assets")
    for asset in assets:
        if not isinstance(asset, dict):
            continue
        fmt = str(asset.get("format", "")).upper()
        url = asset.get("url")
        name = str(asset.get("name", ""))
        if fmt != "GLB" or not isinstance(url, str):
            continue
        if "/target/model/original/" in url:
            return url
    for asset in assets:
        if not isinstance(asset, dict):
            continue
        fmt = str(asset.get("format", "")).upper()
        url = asset.get("url")
        if fmt == "GLB" and isinstance(url, str):
            return url
    raise RuntimeError("No GLB asset in task outputs")


def download_file(url: str, dest: Path) -> None:
    dest.parent.mkdir(parents=True, exist_ok=True)
    urllib.request.urlretrieve(url, dest)


def write_manifest(
    out_dir: Path,
    slug: str,
    task_id: str,
    prompt: str,
    source_image: str,
    upload_meta: dict[str, object],
    outputs: dict[str, object],
    glb_path: Path,
) -> None:
    manifest = {
        "slug": slug,
        "task_id": task_id,
        "prompt": prompt,
        "source_image": source_image,
        "upload": upload_meta,
        "model_page_url": outputs.get("model_page_url"),
        "preview_url": outputs.get("preview_url"),
        "actual_points": outputs.get("actual_points"),
        "files": [{"name": glb_path.name, "path": str(glb_path.relative_to(out_dir))}],
    }
    manifest_path = out_dir / f"{slug}_hi3d_manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")


def cmd_quote(args: argparse.Namespace) -> int:
    client = connect_client(Path(args.state_db), Path(args.local_state))
    file_token, meta = upload_image(client, Path(args.image))
    quote = quote_generation(
        client,
        file_token,
        args.prompt,
        args.version,
        args.model_type,
        args.generation_type,
        args.license,
        args.pbr,
    )
    print(json.dumps({"upload": meta, "quote": quote}, indent=2))
    return 0


def cmd_generate(args: argparse.Namespace) -> int:
    client = connect_client(Path(args.state_db), Path(args.local_state))
    result = create_generation(client, args.quote_id)
    print(json.dumps(result, indent=2))
    return 0


def cmd_poll(args: argparse.Namespace) -> int:
    client = connect_client(Path(args.state_db), Path(args.local_state))
    task = poll_task(client, args.task_id, args.timeout_seconds)
    print(json.dumps(task, indent=2))
    status = str(task.get("status", ""))
    return 0 if status == "succeeded" else 1


def cmd_download(args: argparse.Namespace) -> int:
    client = connect_client(Path(args.state_db), Path(args.local_state))
    outputs = client.call_tool("get_task_outputs", {"task_id": args.task_id})
    glb_url = pick_glb_url(outputs)
    out_dir = Path(args.out_dir)
    glb_name = args.glb_name
    glb_path = out_dir / glb_name
    download_file(glb_url, glb_path)
    write_manifest(
        out_dir,
        args.slug,
        args.task_id,
        args.prompt,
        args.source_image,
        json.loads(args.upload_meta) if args.upload_meta else {},
        outputs,
        glb_path,
    )
    print(json.dumps({"glb": str(glb_path), "model_page_url": outputs.get("model_page_url")}, indent=2))
    return 0


def cmd_run(args: argparse.Namespace) -> int:
    if not args.confirm_spend:
        print("Refusing to spend points without --confirm-spend", file=sys.stderr)
        return 2
    client = connect_client(Path(args.state_db), Path(args.local_state))
    source = Path(args.image)
    file_token, upload_meta = upload_image(client, source)
    quote = quote_generation(
        client,
        file_token,
        args.prompt,
        args.version,
        args.model_type,
        args.generation_type,
        args.license,
        args.pbr,
    )
    if not quote.get("can_submit"):
        raise RuntimeError(json.dumps(quote, ensure_ascii=False))
    quote_id = str(quote["quote_id"])
    created = create_generation(client, quote_id)
    task_id = str(created["task_id"])
    task = poll_task(client, task_id, args.timeout_seconds)
    if str(task.get("status")) != "succeeded":
        print(json.dumps(task, indent=2))
        return 1
    outputs = client.call_tool("get_task_outputs", {"task_id": task_id})
    glb_url = pick_glb_url(outputs)
    out_dir = Path(args.out_dir)
    glb_path = out_dir / args.glb_name
    download_file(glb_url, glb_path)
    write_manifest(
        out_dir,
        args.slug,
        task_id,
        args.prompt,
        str(source.resolve()),
        upload_meta,
        outputs,
        glb_path,
    )
    print(
        json.dumps(
            {
                "task_id": task_id,
                "quote_id": quote_id,
                "estimated_points": quote.get("estimated_points"),
                "actual_points": outputs.get("actual_points"),
                "model_page_url": outputs.get("model_page_url"),
                "glb": str(glb_path),
            },
            indent=2,
        )
    )
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Hi3D image-to-3D via Cursor OAuth MCP HTTP")
    parser.add_argument("--state-db", default=str(DEFAULT_STATE_DB))
    parser.add_argument("--local-state", default=str(DEFAULT_LOCAL_STATE))

    sub = parser.add_subparsers(dest="command", required=True)

    quote = sub.add_parser("quote", help="upload + quote only (no spend)")
    quote.add_argument("--image", required=True)
    quote.add_argument("--prompt", required=True)
    quote.add_argument("--version", default=DEFAULT_VERSION)
    quote.add_argument("--model-type", default=DEFAULT_MODEL_TYPE)
    quote.add_argument("--generation-type", default=DEFAULT_GENERATION_TYPE)
    quote.add_argument("--license", default="private")
    quote.add_argument("--pbr", action="store_true", default=True)
    quote.set_defaults(func=cmd_quote)

    gen = sub.add_parser("generate", help="create_generation from existing quote_id")
    gen.add_argument("--quote-id", required=True)
    gen.set_defaults(func=cmd_generate)

    poll = sub.add_parser("poll", help="wait for task completion")
    poll.add_argument("--task-id", required=True)
    poll.add_argument("--timeout-seconds", type=int, default=900)
    poll.set_defaults(func=cmd_poll)

    dl = sub.add_parser("download", help="download GLB + manifest for succeeded task")
    dl.add_argument("--task-id", required=True)
    dl.add_argument("--out-dir", required=True)
    dl.add_argument("--slug", required=True)
    dl.add_argument("--glb-name", default="model.glb")
    dl.add_argument("--prompt", default="")
    dl.add_argument("--source-image", default="")
    dl.add_argument("--upload-meta", default="")
    dl.set_defaults(func=cmd_download)

    run = sub.add_parser("run", help="upload, quote, generate, poll, download")
    run.add_argument("--image", required=True)
    run.add_argument("--prompt", required=True)
    run.add_argument("--out-dir", required=True)
    run.add_argument("--slug", required=True)
    run.add_argument("--glb-name", default="model.glb")
    run.add_argument("--confirm-spend", action="store_true")
    run.add_argument("--timeout-seconds", type=int, default=900)
    run.add_argument("--version", default=DEFAULT_VERSION)
    run.add_argument("--model-type", default=DEFAULT_MODEL_TYPE)
    run.add_argument("--generation-type", default=DEFAULT_GENERATION_TYPE)
    run.add_argument("--license", default="private")
    run.add_argument("--pbr", action="store_true", default=True)
    run.set_defaults(func=cmd_run)

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    return int(args.func(args))


if __name__ == "__main__":
    raise SystemExit(main())
