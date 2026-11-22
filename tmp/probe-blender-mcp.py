import json
import socket
import time

host = "127.0.0.1"
port = 9876
deadline = time.time() + 15.0

request = json.dumps(
    {
        "type": "execute",
        "code": "result = {'probe': True}",
        "strict_json": False,
    }
) + "\0"

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(5.0)
start = time.time()
try:
    sock.connect((host, port))
    sock.sendall(request.encode("utf-8"))
    buf = bytearray()
    while time.time() < deadline:
        try:
            chunk = sock.recv(4096)
            if not chunk:
                break
            buf.extend(chunk)
            if b"\0" in buf:
                break
        except socket.timeout:
            continue
    elapsed = time.time() - start
    if buf:
        line = bytes(buf).split(b"\0", 1)[0]
        print(f"OK elapsed={elapsed:.2f}s bytes={len(buf)} json={line[:500]!r}")
    else:
        print(f"NO_RESPONSE elapsed={elapsed:.2f}s (connected, sent, no data)")
except Exception as ex:
    print(f"ERROR elapsed={time.time() - start:.2f}s {type(ex).__name__}: {ex}")
finally:
    sock.close()