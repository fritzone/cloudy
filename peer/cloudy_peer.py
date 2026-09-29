#!/usr/bin/env python3
"""The cloudy peer: serves a directory tree of this Linux machine to the DOS cloud client.

Run it, then connect to this machine from cloud.exe:

    python3 peer/cloudy_peer.py                     # serves $HOME to this machine only
    python3 peer/cloudy_peer.py --bind 0.0.0.0      # to reach it from a real DOS box on the LAN
    python3 peer/cloudy_peer.py --root /srv/dos     # serves another directory

There is no authentication: anyone who can reach the port can read and write
the served tree, so only use it on a network you trust.
"""

import argparse
import base64
import datetime
import hashlib
import logging
import os
import socket
import socketserver
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cldproto as proto  # noqa: E402

PORT = 8966

# The DOS client can take frames up to 16K, keep the directory pages well below that
PAGE_BYTES = 8000

# The largest piece of a file we send at once, before base64
MAX_CHUNK = 8192

# A single frame from the client should never be bigger than this
MAX_FRAME = 64 * 1024

# Sizes are 32 bit signed longs on the DOS side
MAX_SIZE = 2 ** 31 - 1

log = logging.getLogger("cloudy")


def display_name(name):
    """Something that can travel in XML: invalid UTF-8 and control characters become '?'."""
    text = os.fsencode(name).decode("utf-8", "replace")
    return "".join(c if c >= " " else "?" for c in text)


class Session:
    """The state of one connected client."""

    def __init__(self, root, send):
        self.root = os.path.realpath(root)
        self.send = send
        # hash -> absolute path, for everything we told the client about
        self.paths = {}
        # directory hash -> the entries of its last listing, to page through
        self.listings = {}

    def hash_of(self, path):
        h = hashlib.sha1(os.fsencode(path)).hexdigest()[:12]
        self.paths[h] = path
        return h

    def resolve(self, h):
        """The path of a hash we handed out, None if unknown or outside of the root."""
        path = self.paths.get(h)
        if path is None:
            return None
        real = os.path.realpath(path)
        if real != self.root and not real.startswith(self.root + os.sep):
            return None
        return path

    def handle(self, msg):
        handler = getattr(self, "on_" + msg.NAME, None)
        if handler is None:
            log.warning("No handler for %s", msg.NAME)
            return
        handler(msg)

    # Messages

    def on_ConnectRequest(self, msg):
        log.info("Client connected: platform=%s id=%s", msg.platform, msg.unique_id)
        self.send(proto.ConnectionRequestReply(accepted=True, authentication_required=False,
                                               host_name=socket.gethostname()))

    def on_StatusRequest(self, msg):
        st = os.statvfs(self.root)
        free_kb = min(st.f_bavail * st.f_frsize // 1024, MAX_SIZE)
        self.send(proto.Status(host_platform="linux", drives=["/"], free_space=[free_kb],
                               working_directories=[self.root], current__work_index=0))

    def on_DirectoryListRequest(self, msg):
        if msg.directory_hash == "":
            path = self.root
            dir_hash = self.hash_of(path)
        else:
            dir_hash = msg.directory_hash
            path = self.resolve(dir_hash)
            if path is None:
                self.send(proto.DirectoryList(directory_hash=dir_hash, error="Unknown directory"))
                return

        if msg.start == 0 or dir_hash not in self.listings:
            try:
                self.listings[dir_hash] = self.read_directory(path)
            except OSError as e:
                self.send(proto.DirectoryList(directory_hash=dir_hash, error=e.strerror or str(e)))
                return

        entries = self.listings[dir_hash]
        page = []
        size = 0
        for e in entries[msg.start:]:
            s = len(e.serialize()) + 13  # + <item></item>
            if page and size + s > PAGE_BYTES:
                break
            page.append(e)
            size += s

        self.send(proto.DirectoryList(directory_name=display_name(path), directory_hash=dir_hash,
                                      start=msg.start, total=len(entries), entries=page))

    def read_directory(self, path):
        """The entries of a directory: .. first (unless it is the root), then directories, then files."""
        dirs = []
        files = []
        with os.scandir(path) as it:
            for de in it:
                try:
                    st = de.stat()
                    is_dir = de.is_dir()
                except OSError:
                    continue  # dangling symlink and the like
                entry = self.entry(os.path.join(path, de.name), de.name, st, is_dir)
                (dirs if is_dir else files).append(entry)

        dirs.sort(key=lambda e: e.name.lower())
        files.sort(key=lambda e: e.name.lower())

        result = []
        if os.path.realpath(path) != self.root:
            parent = os.path.dirname(path)
            result.append(self.entry(parent, "..", os.stat(parent), True))
        return result + dirs + files

    def entry(self, path, name, st, is_dir):
        mtime = datetime.datetime.fromtimestamp(st.st_mtime)
        return proto.DirectoryEntry(name=display_name(name),
                                    date=mtime.year * 10000 + mtime.month * 100 + mtime.day,
                                    time=mtime.hour * 10000 + mtime.minute * 100 + mtime.second,
                                    size=0 if is_dir else min(st.st_size, MAX_SIZE),
                                    attrs="d" if is_dir else "",
                                    hash=self.hash_of(path))

    def on_FileReadRequest(self, msg):
        reply = proto.FileData(file_hash=msg.file_hash, offset=msg.offset)
        path = self.resolve(msg.file_hash)
        if path is None or not os.path.isfile(path):
            reply.error = "Unknown file"
            reply.eof = True
            self.send(reply)
            return

        length = max(0, min(msg.length, MAX_CHUNK))
        try:
            with open(path, "rb") as f:
                f.seek(msg.offset)
                data = f.read(length)
                reply.eof = len(data) < length or f.tell() >= os.fstat(f.fileno()).st_size
        except OSError as e:
            reply.error = e.strerror or str(e)
            reply.eof = True
            self.send(reply)
            return

        reply.data = base64.b64encode(data).decode("ascii")
        if msg.offset == 0:
            log.info("Sending %s", path)
        self.send(reply)

    def on_FileWriteRequest(self, msg):
        reply = proto.FileWriteReply(name=msg.name, offset=msg.offset)
        directory = self.resolve(msg.directory_hash)
        name = msg.name

        if directory is None or not os.path.isdir(directory):
            reply.error = "Unknown directory"
        elif not name or name in (".", "..") or "/" in name or "\0" in name:
            reply.error = "Invalid file name"
        elif not os.path.realpath(os.path.join(directory, name)).startswith(self.root + os.sep):
            reply.error = "Outside of the served directory"
        else:
            path = os.path.join(directory, name)
            try:
                data = base64.b64decode(msg.data, validate=False)
                if msg.offset == 0:
                    log.info("Receiving %s", path)
                    f = open(path, "wb")
                else:
                    f = open(path, "r+b")
                    f.seek(msg.offset)
                with f:
                    f.write(data)
                reply.ok = True
            except (OSError, ValueError) as e:
                reply.error = getattr(e, "strerror", None) or str(e)

        if reply.error:
            log.warning("Cannot write %s: %s", name, reply.error)
        self.send(reply)


class Handler(socketserver.BaseRequestHandler):

    def handle(self):
        peer = "%s:%d" % self.client_address
        log.info("Connection from %s", peer)

        # The replies are request/response, don't let Nagle hold back their last segment
        self.request.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

        def send(msg):
            log.debug("-> %s", msg)
            self.request.sendall(proto.envelope(msg))

        session = Session(self.server.root, send)
        buffer = b""
        try:
            while True:
                data = self.request.recv(16384)
                if not data:
                    break
                buffer += data
                while b"\0" in buffer:
                    frame, buffer = buffer.split(b"\0", 1)
                    if not frame:
                        continue
                    try:
                        msg = proto.parse(frame)
                    except Exception as e:
                        log.error("Cannot parse frame from %s: %s", peer, e)
                        continue
                    if msg is None:
                        log.warning("Unknown message from %s: %r", peer, frame[:200])
                        continue
                    log.debug("<- %s", msg)
                    session.handle(msg)
                if len(buffer) > MAX_FRAME:
                    log.error("Frame too big from %s, disconnecting", peer)
                    break
        except (ConnectionResetError, BrokenPipeError) as e:
            log.info("Connection with %s lost: %s", peer, e)
        log.info("Connection from %s closed", peer)


class Server(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True


def main():
    parser = argparse.ArgumentParser(description="Serves a directory tree to the DOS cloud client")
    parser.add_argument("--root", default=os.path.expanduser("~"), help="the directory to serve (default: $HOME)")
    parser.add_argument("--bind", default="127.0.0.1",
                        help="the address to listen on (default: 127.0.0.1, enough for DOSBox; "
                             "use 0.0.0.0 for a real DOS machine)")
    parser.add_argument("--port", type=int, default=PORT)
    parser.add_argument("-v", "--verbose", action="count", default=0, help="-v: what happens, -vv: all the messages")
    args = parser.parse_args()

    logging.basicConfig(level=[logging.WARNING, logging.INFO, logging.DEBUG][min(args.verbose, 2)],
                        format="%(asctime)s %(levelname)s %(message)s")

    root = os.path.realpath(args.root)
    if not os.path.isdir(root):
        parser.error("not a directory: " + root)

    with Server((args.bind, args.port), Handler) as server:
        server.root = root
        print("cloudy peer serving %s on %s:%d" % (root, args.bind, args.port), flush=True)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    main()
