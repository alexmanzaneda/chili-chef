"""Minimal reader of GIMP .xcf files: composites the visible 8-bit RGB layers.

    from xcf import read_xcf, read_drawing
    image = read_xcf('art/banda/chili.xcf')   # PIL RGBA image
    band = read_xcf('art/banda.xcf', 'Fondo')   # Only the layer called 'Fondo'
    shield = read_drawing('art/banda/escudo.png')   # .xcf or .png
"""
import struct, zlib
from PIL import Image


def read_xcf(path, only_layer=None):
    data = open(path, 'rb').read()
    assert data[:9] == b'gimp xcf ', 'not an xcf'
    version = 0 if data[9:13] == b'file' else int(data[10:13])
    pos = 14
    ptr_size = 8 if version >= 11 else 4

    def u32(p): return struct.unpack('>I', data[p:p + 4])[0]
    def i32(p): return struct.unpack('>i', data[p:p + 4])[0]
    def ptr(p): return struct.unpack('>Q' if ptr_size == 8 else '>I', data[p:p + ptr_size])[0]

    width, height, base_type = u32(pos), u32(pos + 4), u32(pos + 8)
    pos += 12
    precision = 150
    if version >= 4:
        precision = u32(pos); pos += 4
    assert base_type == 0, 'only RGB images'

    def props(p):
        out = {}
        while True:
            t, n = u32(p), u32(p + 4)
            out[t] = data[p + 8:p + 8 + n]
            p += 8 + n
            if t == 0:
                return out, p

    img_props, pos = props(pos)
    compression = img_props.get(17, b'\x01')[0]
    layers = []
    while True:
        lp = ptr(pos); pos += ptr_size
        if lp == 0:
            break
        layers.append(lp)

    def read_tile(p, tw, th, bpp, end):
        n = tw * th
        if compression == 0:
            raw = data[p:p + n * bpp]
            return [raw[i * bpp:(i + 1) * bpp] for i in range(n)]
        if compression == 2:
            raw = zlib.decompress(data[p:end])
            return [raw[i * bpp:(i + 1) * bpp] for i in range(n)]
        chans = []
        for c in range(bpp):
            out = bytearray()
            while len(out) < n:
                v = data[p]; p += 1
                if v >= 128:
                    if v == 128:
                        cnt = data[p] * 256 + data[p + 1]; p += 2
                    else:
                        cnt = 256 - v
                    out += data[p:p + cnt]; p += cnt
                else:
                    if v == 127:
                        cnt = data[p] * 256 + data[p + 1]; p += 2
                    else:
                        cnt = v + 1
                    out += bytes([data[p]]) * cnt; p += 1
            chans.append(out)
        return [bytes(chans[c][i] for c in range(bpp)) for i in range(n)]

    canvas = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    for lp in reversed(layers):  # The list goes from the top layer to the bottom one
        lw, lh, ltype = u32(lp), u32(lp + 4), u32(lp + 8)
        nlen = u32(lp + 12)
        p = lp + 16 + nlen
        lprops, p = props(p)
        if only_layer is not None and data[lp + 16:lp + 16 + nlen].rstrip(b'\0').decode('utf-8', 'replace') != only_layer:
            continue
        visible = u32(0) if 8 not in lprops else struct.unpack('>I', lprops[8])[0]
        if 8 in lprops and not visible:
            continue
        opacity = struct.unpack('>I', lprops[6])[0] / 255 if 6 in lprops else 1.0
        if 33 in lprops:
            opacity = struct.unpack('>f', lprops[33])[0]
        ox, oy = struct.unpack('>ii', lprops[15]) if 15 in lprops else (0, 0)
        hp = ptr(p)
        hw, hh, bpp = u32(hp), u32(hp + 4), u32(hp + 8)
        assert bpp in (3, 4), 'only 8-bit RGB/RGBA layers (bpp %d)' % bpp
        levp = ptr(hp + 12)
        tp = levp + 8
        tiles = []
        while True:
            t = ptr(tp); tp += ptr_size
            if t == 0:
                break
            tiles.append(t)
        layer = Image.new('RGBA', (lw, lh), (0, 0, 0, 0))
        cols = (lw + 63) // 64
        for i, t in enumerate(tiles):
            tx, ty = (i % cols) * 64, (i // cols) * 64
            tw, th = min(64, lw - tx), min(64, lh - ty)
            end = tiles[i + 1] if i + 1 < len(tiles) else len(data)
            px = read_tile(t, tw, th, bpp, end)
            for j, v in enumerate(px):
                a = v[3] if bpp == 4 else 255
                layer.putpixel((tx + j % tw, ty + j // tw), (v[0], v[1], v[2], round(a * opacity)))
        canvas.alpha_composite(layer, (ox, oy)) if ox >= 0 and oy >= 0 else canvas.paste(layer, (ox, oy), layer)

    return canvas


def read_drawing(path):
    """A drawing of art/: a GIMP .xcf, or any image PIL reads (.png...)."""
    if path.endswith('.xcf'):
        return read_xcf(path)
    return Image.open(path).convert('RGBA')
