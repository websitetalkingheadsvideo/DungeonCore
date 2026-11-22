"""Ping the Blender Lab MCP add-on socket (null-delimited JSON).

PowerShell:
  & .\\tmp\\Invoke-BlenderMcpPing.ps1

Or:
  cd tmp\\blender-mcpb-1.0.3
  $env:BLENDER_MCP_HOST = '127.0.0.1'
  uv run python ..\\blender-mcp-ping.py
"""

import json
import os
import socket
import sys

HOST = os.environ.get("BLENDER_MCP_HOST", "127.0.0.1")
PORT = int(os.environ.get("BLENDER_MCP_PORT", "9876"))
TIMEOUT = float(os.environ.get("BLENDER_MCP_TIMEOUT", "30"))


def ping() -> int:
    payload = (
        json.dumps(
            {
                "type": "execute",
                "code": "result = {'ping': True}",
                "strict_json": True,
            }
        )
        + "\0"
    )

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(TIMEOUT)
    try:
        sock.connect((HOST, PORT))
        sock.sendall(payload.encode("utf-8"))
        buf = bytearray()
        while True:
            chunk = sock.recv(65536)
            if not chunk:
                break
            buf.extend(chunk)
            if b"\0" in buf:
                break
    except ConnectionRefusedError:
        print(
            f"FAIL: connection refused at {HOST}:{PORT} (start MCP server in Blender)",
            file=sys.stderr,
        )
        return 1
    except socket.timeout:
        print(
            f"FAIL: no response within {TIMEOUT}s at {HOST}:{PORT} "
            "(server listening but not polling? check Blender System Console)",
            file=sys.stderr,
        )
        return 1
    finally:
        sock.close()

    if not buf:
        print(f"FAIL: empty response from {HOST}:{PORT}", file=sys.stderr)
        return 1

    line = bytes(buf).split(b"\0", 1)[0]
    try:
        response = json.loads(line.decode("utf-8"))
    except (json.JSONDecodeError, UnicodeDecodeError) as ex:
        print(f"FAIL: invalid JSON: {ex!s}\nraw={line[:200]!r}", file=sys.stderr)
        return 1

    print(json.dumps(response, indent=2))
    if response.get("status") != "ok":
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(ping())