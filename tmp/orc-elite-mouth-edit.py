from __future__ import annotations

import base64
import json
import sys
import time
import urllib.request
from pathlib import Path

from hi3d_client import DEFAULT_LOCAL_STATE, DEFAULT_STATE_DB, connect_client

img_path = Path(r"E:\Dungeon Core\Design\Reference Images\Orcs\Orc-Elite.png")
out_path = Path(r"E:\Dungeon Core\Design\Reference Images\Orcs\Orc-Elite-v3-closed-mouth.png")
b64 = base64.b64encode(img_path.read_bytes()).decode()
client = connect_client(DEFAULT_STATE_DB, DEFAULT_LOCAL_STATE)
up = client.call_tool("upload_input", {"image_base64": b64, "mime_type": "image/png"})
token = up["file_token"]
prompt = (
    "Preserve everything identical: same green orc elite, T-pose, white background, armor, "
    "face, eyes, ears. Change ONLY the mouth: remove ALL protruding tusks and ALL visible teeth. "
    "Lips fully closed in a flat sealed line; no ivory, no fangs, no canines sticking out. "
    "Mouth should look like closed lips on green skin only."
)
create = client.call_tool(
    "create_image_generation",
    {
        "operation": "edit",
        "prompt": prompt,
        "file_tokens": [token],
        "model_name": "Nano-Banana-Pro",
        "aspect_ratio": "3:4",
    },
)
task_id = create.get("task_id") or create.get("id")
print("task", task_id)
for _ in range(60):
    task = client.call_tool("get_task", {"task_id": task_id})
    status = task.get("status")
    print("status", status)
    if status in ("succeeded", "completed", "failed"):
        break
    nap = task.get("next_poll_after", 5)
    time.sleep(float(nap) if nap else 5.0)
if task.get("status") == "failed":
    print(json.dumps(task), file=sys.stderr)
    sys.exit(1)
outs = client.call_tool("get_task_outputs", {"task_id": task_id})
url = None
for key in ("images", "output_images", "result_images"):
    images = outs.get(key)
    if not isinstance(images, list):
        continue
    for item in images:
        if isinstance(item, dict):
            url = item.get("url") or item.get("image_url") or item.get("download_url")
        elif isinstance(item, str):
            url = item
        if url:
            break
    if url:
        break
if not url:
    url = outs.get("url") or outs.get("download_url")
if not url:
    print(json.dumps(outs)[:4000], file=sys.stderr)
    sys.exit(2)
urllib.request.urlretrieve(url, out_path)
print("saved", out_path, out_path.stat().st_size)
