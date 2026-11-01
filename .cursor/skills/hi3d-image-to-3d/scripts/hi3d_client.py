from __future__ import annotations

import base64
import ctypes
import json
import os
import sqlite3
import urllib.error
import urllib.request
from pathlib import Path
from typing import Any

from cryptography.hazmat.primitives.ciphers.aead import AESGCM

MCP_URL = "https://agent.hi3d.ai/mcp"
DEFAULT_STATE_DB = Path(
    os.environ.get(
        "CURSOR_STATE_DB",
        r"C:\Users\paris\AppData\Roaming\Cursor\User\globalStorage\state.vscdb",
    )
)
DEFAULT_LOCAL_STATE = Path(
    os.environ.get(
        "CURSOR_LOCAL_STATE",
        r"C:\Users\paris\AppData\Roaming\Cursor\Local State",
    )
)
HI3D_TOKEN_KEY = (
    "mcpOAuth.secret.W3VzZXItaGkzZDo6bWNwU2NvcGU6cHJvZmlsZTpaR1ZtWVhWc2RBXSBtY3BfdG9rZW5z"
)


class DATA_BLOB(ctypes.Structure):
    _fields_ = [("cbData", ctypes.c_uint), ("pbData", ctypes.POINTER(ctypes.c_byte))]


def dpapi_decrypt(cipher: bytes) -> bytes:
    blob_in = DATA_BLOB(
        len(cipher),
        ctypes.cast(ctypes.create_string_buffer(cipher), ctypes.POINTER(ctypes.c_byte)),
    )
    blob_out = DATA_BLOB()
    ok = ctypes.windll.crypt32.CryptUnprotectData(
        ctypes.byref(blob_in), None, None, None, None, 0, ctypes.byref(blob_out)
    )
    if not ok:
        raise OSError("CryptUnprotectData failed; run from the Windows user session that owns Cursor OAuth.")
    try:
        return ctypes.string_at(blob_out.pbData, blob_out.cbData)
    finally:
        ctypes.windll.kernel32.LocalFree(blob_out.pbData)


def chromium_aes_key(local_state_path: Path) -> bytes:
    state = json.loads(local_state_path.read_text(encoding="utf-8"))
    enc_key = base64.b64decode(state["os_crypt"]["encrypted_key"])
    if enc_key.startswith(b"DPAPI"):
        enc_key = enc_key[5:]
    return dpapi_decrypt(enc_key)


def decrypt_v10(blob: bytes, local_state_path: Path) -> bytes:
    aes = AESGCM(chromium_aes_key(local_state_path))
    return aes.decrypt(blob[3:15], blob[15:], None)


def load_hi3d_access_token(
    state_db: Path,
    local_state: Path,
) -> str:
    if not state_db.is_file():
        raise FileNotFoundError(f"Cursor state DB not found: {state_db}")
    con = sqlite3.connect(state_db)
    cur = con.cursor()
    cur.execute("SELECT value FROM ItemTable WHERE key = ?", (HI3D_TOKEN_KEY,))
    row = cur.fetchone()
    con.close()
    if row is None:
        raise RuntimeError(
            "Hi3D OAuth not found. In Cursor: Settings → MCP → hi3d → Login, then retry."
        )
    wrapper = json.loads(row[0].decode("utf-8") if isinstance(row[0], bytes) else row[0])
    tokens = json.loads(decrypt_v10(bytes(wrapper["data"]), local_state).decode("utf-8"))
    access = tokens.get("access_token")
    if not isinstance(access, str):
        raise RuntimeError("Hi3D token payload missing access_token")
    return access


class Hi3dMcpClient:
    def __init__(self, access_token: str) -> None:
        self.access_token = access_token
        self.session_id: str | None = None
        self.req_id = 0

    def _post(self, body: dict[str, Any]) -> dict[str, Any]:
        self.req_id += 1
        payload_body = {**body, "jsonrpc": "2.0", "id": self.req_id}
        headers = {
            "Content-Type": "application/json",
            "Accept": "application/json, text/event-stream",
            "Authorization": f"Bearer {self.access_token}",
        }
        if self.session_id is not None:
            headers["Mcp-Session-Id"] = self.session_id
        req = urllib.request.Request(
            MCP_URL,
            data=json.dumps(payload_body).encode("utf-8"),
            headers=headers,
            method="POST",
        )
        try:
            with urllib.request.urlopen(req, timeout=300) as resp:
                if "mcp-session-id" in resp.headers:
                    self.session_id = resp.headers["mcp-session-id"]
                payload = resp.read().decode("utf-8")
        except urllib.error.HTTPError as exc:
            err_body = exc.read().decode("utf-8", errors="replace")
            raise RuntimeError(f"HTTP {exc.code}: {err_body[:800]}") from exc
        for line in payload.splitlines():
            if line.startswith("data:"):
                payload = line[5:].strip()
                break
        parsed = json.loads(payload)
        if "error" in parsed:
            raise RuntimeError(json.dumps(parsed["error"]))
        result = parsed.get("result")
        if isinstance(result, dict):
            return result
        return parsed

    def initialize(self) -> None:
        self._post(
            {
                "method": "initialize",
                "params": {
                    "protocolVersion": "2024-11-05",
                    "capabilities": {},
                    "clientInfo": {"name": "hi3d-image-to-3d", "version": "1.0"},
                },
            }
        )

    def call_tool(self, name: str, arguments: dict[str, Any]) -> dict[str, Any]:
        result = self._post(
            {
                "method": "tools/call",
                "params": {"name": name, "arguments": arguments},
            }
        )
        text = result["content"][0]["text"]
        return json.loads(text)


def connect_client(
    state_db: Path,
    local_state: Path,
) -> Hi3dMcpClient:
    client = Hi3dMcpClient(load_hi3d_access_token(state_db, local_state))
    client.initialize()
    return client
