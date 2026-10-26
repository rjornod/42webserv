import argparse
import socket
import sys
import time
from dataclasses import dataclass, field
 
HOST = "127.0.0.1"
PORT = 8080
TIMEOUT = 3.0

# --------------------------------------------------------------------------
# Building requests
# --------------------------------------------------------------------------
 
def build_request(
    method: str = "GET",
    path: str = "/",
    headers: dict[str, str] | None = None,
    body: bytes | str = b"",
    version: str = "HTTP/1.1",
    host: str | None = "localhost",
    close: bool = True,
    auto_content_length: bool = True,
) -> bytes:
    """Build a raw HTTP request.
 
    host=None            -> omit the Host header (to test error handling)
    close=False          -> don't send "Connection: close" (keep-alive)
    auto_content_length  -> add Content-Length when there is a body; set to
                            False if you want to send a wrong/missing one by
                            passing your own in `headers`.
    """
    if isinstance(body, str):
        body = body.encode()
 
    hdrs: dict[str, str] = {}
    if host is not None:
        hdrs["Host"] = host
    if close:
        hdrs["Connection"] = "close"
    if body and auto_content_length:
        hdrs["Content-Length"] = str(len(body))
    if headers:
        hdrs.update(headers)  # caller wins
 
    head = f"{method} {path} {version}\r\n"
    head += "".join(f"{k}: {v}\r\n" for k, v in hdrs.items())
    head += "\r\n"
    return head.encode() + body
 
 
# --------------------------------------------------------------------------
# Parsing responses
# --------------------------------------------------------------------------
 
@dataclass
class Response:
    raw: bytes = b""
    status_line: str = ""
    status: int = 0
    headers: dict[str, str] = field(default_factory=dict)  # lower-cased keys
    body: bytes = b""
 
    @property
    def text(self) -> str:
        return self.body.decode(errors="replace")
 
 
def parse_response(data: bytes) -> Response:
    resp = Response(raw=data)
    head, _, body = data.partition(b"\r\n\r\n")
    lines = head.decode(errors="replace").split("\r\n")
    resp.status_line = lines[0] if lines else ""
    parts = resp.status_line.split()
    if len(parts) >= 2 and parts[1].isdigit():
        resp.status = int(parts[1])
    for line in lines[1:]:
        key, _, value = line.partition(":")
        if key:
            resp.headers[key.strip().lower()] = value.strip()
    resp.body = body
    return resp
 
 
def recv_response(sock: socket.socket) -> Response:
    """Read exactly one response from `sock`.
 
    Works for keep-alive connections: reads headers, then Content-Length
    bytes of body. If there is no Content-Length, reads until the server
    closes the connection (or the timeout hits).
    Chunked transfer encoding is not decoded here; add it if you implement it.
    """
    data = b""
    try:
        # 1. read until end of headers
        while b"\r\n\r\n" not in data:
            chunk = sock.recv(4096)
            if not chunk:
                return parse_response(data)
            data += chunk
 
        head, _, body = data.partition(b"\r\n\r\n")
        resp = parse_response(data)
 
        # 2. read the body
        if "content-length" in resp.headers:
            want = int(resp.headers["content-length"])
            while len(body) < want:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                body += chunk
            data = head + b"\r\n\r\n" + body
        else:
            while chunk := sock.recv(4096):
                body += chunk
            data = head + b"\r\n\r\n" + body
    except socket.timeout:
        pass  # return whatever we got; the test decides if that's a failure
    return parse_response(data)
 
 
# --------------------------------------------------------------------------
# Sending
# --------------------------------------------------------------------------
 
def connect(host: str = HOST, port: int = PORT, timeout: float = TIMEOUT) -> socket.socket:
    return socket.create_connection((host, port), timeout=timeout)
 
 
def send_raw(raw: bytes | str, host: str = HOST, port: int = PORT) -> Response:
    """One connection, one request, one response."""
    if isinstance(raw, str):
        raw = raw.encode()
    with connect(host, port) as s:
        s.sendall(raw)
        return recv_response(s)
 
 
def request(method: str = "GET", path: str = "/", **kwargs) -> Response:
    """Convenience: build_request(...) + send_raw(...)."""
    return send_raw(build_request(method, path, **kwargs))
 
 
def send_in_pieces(pieces: list[bytes], delay: float = 0.2) -> Response:
    """Send a request split across several send() calls (tests partial reads)."""
    with connect() as s:
        for piece in pieces:
            s.sendall(piece)
            time.sleep(delay)
        return recv_response(s)
 
 
# --------------------------------------------------------------------------
# Tiny test framework
# --------------------------------------------------------------------------
 
TESTS: list[tuple[str, callable]] = []
 
 
def test(fn):
    """Decorator: register a test function. Use plain `assert` inside."""
    TESTS.append((fn.__name__, fn))
    return fn
 
 
GREEN, RED, DIM, RESET = "\033[32m", "\033[31m", "\033[2m", "\033[0m"
 
 
def run(pattern: str = "") -> int:
    selected = [(n, f) for n, f in TESTS if pattern in n]
    passed = failed = 0
    for name, fn in selected:
        try:
            fn()
            print(f"{GREEN}PASS{RESET} {name}")
            passed += 1
        except AssertionError as e:
            print(f"{RED}FAIL{RESET} {name}: {e}")
            failed += 1
        except (ConnectionError, socket.timeout, OSError) as e:
            print(f"{RED}ERR {RESET} {name}: {type(e).__name__}: {e}")
            failed += 1
    print(f"{DIM}---{RESET}\n{passed} passed, {failed} failed, {len(selected)} run")
    return 1 if failed else 0
 
 
def expect_status(resp: Response, code: int):
    assert resp.status == code, f"expected {code}, got {resp.status} ({resp.status_line!r})"
 
 
# --------------------------------------------------------------------------
# Tests: edit the paths / expectations to match your config
# --------------------------------------------------------------------------
 
@test
def get_root():
    r = request("GET", "/")
    expect_status(r, 200)
 
 
@test
def get_missing_file_is_404():
    r = request("GET", "/this/does/not/exist")
    expect_status(r, 404)
 
 
@test
def unknown_method_is_rejected():
    r = request("BREW", "/")
    assert r.status in (400, 405, 501), f"got {r.status}"
 
 
@test
def missing_host_header_is_400():
    r = request("GET", "/", host=None)
    expect_status(r, 400)
 
 
@test
def path_traversal_is_blocked():
    r = request("GET", "/../../etc/passwd")
    assert r.status in (400, 403, 404), f"got {r.status}"
    assert b"root:" not in r.body, "leaked /etc/passwd!"
 
 
@test
def delete_missing_file_is_404():
    r = request("DELETE", "/uploads/nope.txt")
    expect_status(r, 404)
 
 
@test
def post_upload_then_delete():
    r = request("POST", "/uploads/test.txt", body="hello")  # adjust to your upload route
    assert r.status in (200, 201), f"upload got {r.status}"
    r = request("DELETE", "/uploads/test.txt")
    assert r.status in (200, 204), f"delete got {r.status}"
 
 
@test
def request_split_across_packets():
    full = build_request("GET", "/")
    r = send_in_pieces([full[:5], full[5:20], full[20:]])
    expect_status(r, 200)
 
 
@test
def keep_alive_two_requests_one_connection():
    with connect() as s:
        s.sendall(build_request("GET", "/", close=False))
        r1 = recv_response(s)
        expect_status(r1, 200)
        s.sendall(build_request("GET", "/", close=False))
        r2 = recv_response(s)
        expect_status(r2, 200)
 
 
@test
def content_length_has_correct_value():
    r = request("GET", "/")
    if "content-length" in r.headers:
        assert int(r.headers["content-length"]) == len(r.body), "Content-Length mismatch"
 
 
# --------------------------------------------------------------------------
 
if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("-H", "--host", default=HOST)
    ap.add_argument("-p", "--port", type=int, default=PORT)
    ap.add_argument("-k", "--keyword", default="", help="only run tests whose name contains this")
    args = ap.parse_args()
    HOST, PORT = args.host, args.port
 
    # make helpers pick up the CLI host/port as their defaults
    connect.__defaults__ = (HOST, PORT, TIMEOUT)
    send_raw.__defaults__ = (HOST, PORT)
 
    sys.exit(run(args.keyword))