#!/usr/bin/env python3
r"""Build dist\Interface\BackPocket\category_icon.swf with two icons (1.0.3; the owner, 2026-09-28: "We should probably make
the other back pocket icon distinct").

    frame 1  "back_pocket"       the satchel - the original movie's shape and placement, kept byte for byte
    frame 2  "favourite_pocket"  the satchel with a star in its empty top-right corner - the Favourites Pocket

The satchel (DefineShape3 id 1, fill #E5E5E5, 240 x 240 units, contours filled on the right, outline clockwise) is read
from the committed 1.0.x movie, so its art never changes; the star is added as DefineShape3 id 2 in the same colour and
fill convention and placed above it. assets\back_pocket_category_icon.svg is the satchel; favourites_pocket_icon.svg
beside it shows the combined art.

Usage:  python tools/build-category-icon.py [source.swf] [out.swf]
        (default: rebuild the dist movie in place from its own frame 1)
"""
import math
import os
import struct
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT = os.path.join(REPO, "dist", "Interface", "BackPocket", "category_icon.swf")
STAR_CENTRE = (205.0, 33.0)
STAR_OUTER, STAR_INNER = 28.0, 11.5
FILL_RGBA = bytes.fromhex("e5e5e5ff")


class Bits:
    def __init__(self):
        self.bits = []

    def ub(self, v, n):
        for i in range(n - 1, -1, -1):
            self.bits.append((v >> i) & 1)

    def sb(self, v, n):
        self.ub(v + (1 << n) if v < 0 else v, n)

    def bytes(self):
        out = bytearray()
        for i in range(0, len(self.bits), 8):
            chunk = self.bits[i:i + 8] + [0] * (8 - len(self.bits[i:i + 8]))
            out.append(int("".join(map(str, chunk)), 2))
        return bytes(out)


def sb_width(*values):
    n = 1
    for v in values:
        while not (-(1 << (n - 1)) <= v < (1 << (n - 1))):
            n += 1
    return n


def rect(xmin, xmax, ymin, ymax):
    b = Bits()
    n = sb_width(xmin, xmax, ymin, ymax)
    b.ub(n, 5)
    for v in (xmin, xmax, ymin, ymax):
        b.sb(v, n)
    return b.bytes()


def tag(code, payload):
    if len(payload) < 0x3F:
        return struct.pack("<H", (code << 6) | len(payload)) + payload
    return struct.pack("<HI", (code << 6) | 0x3F, len(payload)) + payload


def star_points():
    cx, cy = STAR_CENTRE
    pts = []
    for i in range(10):
        a = -math.pi / 2 + i * math.pi / 5            # from the top point, clockwise on screen (y down)
        r = STAR_OUTER if i % 2 == 0 else STAR_INNER
        pts.append((round(cx + r * math.cos(a)), round(cy + r * math.sin(a))))
    return pts


def star_shape(shape_id):
    pts = star_points()
    xs, ys = [p[0] for p in pts], [p[1] for p in pts]
    body = struct.pack("<H", shape_id) + rect(min(xs), max(xs), min(ys), max(ys))
    body += bytes([1, 0x00]) + FILL_RGBA + bytes([0])   # one solid fill, no line styles
    b = Bits()
    b.ub(1, 4)            # NumFillBits
    b.ub(0, 4)            # NumLineBits
    # StyleChangeRecord: move to the first point, FillStyle1 = 1 (the satchel's convention)
    b.ub(0, 1)
    b.ub(0, 1); b.ub(0, 1); b.ub(1, 1); b.ub(0, 1); b.ub(1, 1)
    x0, y0 = pts[0]
    mb = sb_width(x0, y0)
    b.ub(mb, 5); b.sb(x0, mb); b.sb(y0, mb)
    b.ub(1, 1)            # FillStyle1
    cur = pts[0]
    for nxt in pts[1:] + [pts[0]]:
        dx, dy = nxt[0] - cur[0], nxt[1] - cur[1]
        nb = max(2, sb_width(dx, dy))
        b.ub(1, 1); b.ub(1, 1); b.ub(nb - 2, 4); b.ub(1, 1)   # edge, straight, NumBits, general line
        b.sb(dx, nb); b.sb(dy, nb)
        cur = nxt
    b.ub(0, 1); b.ub(0, 5)                                     # EndShapeRecord
    return tag(32, body + b.bytes())


def read_frame1(path):
    raw = open(path, "rb").read()
    assert raw[:3] == b"FWS", "expected an uncompressed movie"
    body = raw[8:]
    nbits = body[0] >> 3
    head = (5 + 4 * nbits + 7) // 8
    rect_bytes, rate = body[:head], body[head:head + 2]
    off = head + 4
    tags = []
    while off < len(body):
        h = struct.unpack("<H", body[off:off + 2])[0]
        code, ln, hl = h >> 6, h & 0x3F, 2
        if ln == 0x3F:
            ln, hl = struct.unpack("<I", body[off + 2:off + 6])[0], 6
        tags.append((code, body[off:off + hl + ln]))
        off += hl + ln
        if code in (1, 0):
            break
    codes = [c for c, _ in tags]
    assert codes[:4] == [32, 26, 43, 1], f"frame 1 is not DefineShape3 / PlaceObject2 / FrameLabel / ShowFrame: {codes}"
    assert b"back_pocket\x00" in tags[2][1]
    return raw[3], rect_bytes, rate, b"".join(t for _, t in tags)


def main():
    src = sys.argv[1] if len(sys.argv) > 1 else DEFAULT
    out = sys.argv[2] if len(sys.argv) > 2 else DEFAULT
    version, rect_bytes, rate, frame1 = read_frame1(src)
    frame2 = (star_shape(2) +
              tag(26, bytes([0x06]) + struct.pack("<HH", 2, 2) + b"\x00") +   # place shape 2 at depth 2, no transform
              tag(43, b"favourite_pocket\x00") + tag(1, b""))
    body = rect_bytes + rate + struct.pack("<H", 2) + frame1 + frame2 + tag(0, b"")
    movie = b"FWS" + bytes([version]) + struct.pack("<I", 8 + len(body)) + body
    open(out, "wb").write(movie)
    print(f"build-category-icon: {out} ({len(movie)} bytes) - frames back_pocket, favourite_pocket; star points {star_points()}")


if __name__ == "__main__":
    main()
