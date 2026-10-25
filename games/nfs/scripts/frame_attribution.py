#!/usr/bin/env python3
"""Attribute a visible artefact in a capture to a pass and a colour target of a dumper frame.

Two questions, one tool, because the second is useless without the first:

  chain  <boot.log>            the frame's passes in order: colour targets, draw count, sampled serials
  rank   <capture.png> <dir>   which dumped target first carries the capture's black blocks

`rank` takes the blocks from the capture itself (near-black components whose surroundings are
bright) and reports, per target, the share of that mask that is exactly (0,0,0) plus the mean
luminance inside and in the ring just outside. Exact zero is the statistic that compares across
targets: each target has its own colour space, so luminance alone is not comparable, but a pixel
that was never written (or written 0) is 0 in every one of them.

Both read a frame produced by DM_NVN_PASSDUMP=<frame> and DM_NVN_TEXDUMP=<dir>/f-%d.png (see
docs/ — the dump does not create <dir>, and a png is named f-<serial*100 + mip>.png).

Usage:
  frame_attribution.py chain analysis/boot.log
  frame_attribution.py rank capture.png analysis/texdump [x1,y1,x2,y2]
"""
import glob
import os
import re
import sys
from collections import OrderedDict

import numpy as np
from PIL import Image

CAPTURE_BLOCK = (600, 440, 1320, 960)   # x1,y1,x2,y2 in a 1920x1080 frame: where a chase camera's car sits

PASS = re.compile(r'^\[pass\] F(\d+) P (\d+) c(\d+) ser=(\d+) fmt=(\w+) (\d+)x(\d+) (.*)$')
DEPTH = re.compile(r'^\[pass\] F(\d+) P (\d+) d ser=(\d+)')
DRAW = re.compile(r'^\[pass\] F(\d+) d (\d+) ')
BIND = re.compile(r'^\[pass\] F(\d+) B (\d+) st\d+ tex reg\d+ tid=\d+ real ser=(\d+)')


# ---------------------------------------------------------------- pass chain

def chain(path):
    """Print the frame's pass sequence.

    A draw is the lowercase `d` record; uppercase `D` is the prim= summary and counting it makes a
    330-draw pass read as one fullscreen draw (Do-Not #29).
    """
    passes = OrderedDict()
    for line in open(path, errors='replace'):
        if not line.startswith('[pass]'):
            continue
        m = PASS.match(line)
        if m:
            _, pno, c, ser, fmt, w, h, rest = m.groups()
            p = passes.setdefault(pno, {'c': [], 'd': 0, 's': set()})
            p['c'].append(f'c{c}ser={ser}({fmt},{w}x{h})')
            if 'load clear=' in rest:
                p['clear'] = rest.split('load ')[1].split(' mem=')[0].replace('clear=', '', 1)
            continue
        m = DEPTH.match(line)
        if m:
            _, pno, ser = m.groups()
            passes.setdefault(pno, {'c': [], 'd': 0, 's': set()})['depth'] = ser
            continue
        m = DRAW.match(line)
        if m:
            passes.setdefault(m.group(2), {'c': [], 'd': 0, 's': set()})['d'] += 1
            continue
        m = BIND.match(line)
        if m:
            passes.setdefault(m.group(2), {'c': [], 'd': 0, 's': set()})['s'].add(int(m.group(3)))
    print(f'{path}: {len(passes)} pass(es)')
    for pno, p in passes.items():
        tg = ' '.join(p['c']) or ('depth ser=' + p.get('depth', '?'))
        clear = f"  clear={p['clear']}" if 'clear' in p else ''
        samples = sorted(p['s'])
        print(f'  {pno}  draws={p["d"]:3d}  {tg}{clear}  samples={samples}')


# ---------------------------------------------------------------- target ranking

def _label(mask):
    """8-connected components of a bool mask, as (area, x1, y1, x2, y2) with x2/y2 exclusive.

    Run-length union-find rather than a per-pixel flood fill: a 1920x1080 dump has 64 targets and a
    Python flood fill over the dark half of a frame takes long enough to discourage using this.
    """
    parent = []

    def find(a):
        while parent[a] != a:
            parent[a] = parent[parent[a]]
            a = parent[a]
        return a

    def union(a, b):
        ra, rb = find(a), find(b)
        if ra != rb:
            parent[rb] = ra

    meta, prev = [], []
    for y in range(mask.shape[0]):
        cur = []
        row = mask[y]
        if row.any():
            d = np.diff(np.concatenate(([0], row.view(np.int8), [0])))
            for x1, x2 in zip(np.flatnonzero(d == 1).tolist(), np.flatnonzero(d == -1).tolist()):
                parent.append(len(meta))
                meta.append((y, int(x1), int(x2)))
                cur.append(len(meta) - 1)
        i = j = 0
        while i < len(prev) and j < len(cur):
            _, px1, px2 = meta[prev[i]]
            _, cx1, cx2 = meta[cur[j]]
            if px2 <= cx1:
                i += 1
            elif cx2 <= px1:
                j += 1
            else:
                union(prev[i], cur[j])
                if px2 < cx2:
                    i += 1
                else:
                    j += 1
        prev = cur
    agg = {}
    for rid, (y, x1, x2) in enumerate(meta):
        a = agg.get(find(rid))
        if a is None:
            agg[find(rid)] = [x2 - x1, 1, x1, y, x2, y + 1]
        else:
            a[0] += x2 - x1
            a[1] += 1
            a[2] = min(a[2], x1)
            a[4] = max(a[4], x2)
            a[5] = y + 1
    return [(a[0], a[2], a[3], a[4], a[5]) for a in agg.values()]


def _block_mask(rgb, region, min_area=40, max_area=900, ring_min=50, max_side=30):
    """The capture's own blocks: dark components on a bright ground.

    Without the ring test the car's dark interior joins the mask and every target averages ~30
    inside, which erases the difference the ranking exists to show.
    """
    lum = rgb.mean(2)
    mask = np.zeros(lum.shape, bool)
    x1r, y1r, x2r, y2r = region
    for area, x1, y1, x2, y2 in _label(lum <= 10):
        if not (min_area <= area <= max_area) or (x2 - x1) > max_side or (y2 - y1) > max_side:
            continue
        if not (x1r <= (x1 + x2) / 2 <= x2r and y1r <= (y1 + y2) / 2 <= y2r):
            continue
        pad = 3
        sub = lum[max(0, y1 - pad):y2 + pad, max(0, x1 - pad):x2 + pad]
        comp = np.zeros(sub.shape, bool)
        comp[y1 - max(0, y1 - pad):y2 - max(0, y1 - pad),
             x1 - max(0, x1 - pad):x2 - max(0, x1 - pad)] = True
        if sub[~comp].mean() < ring_min:
            continue
        mask[y1:y2, x1:x2] |= lum[y1:y2, x1:x2] <= 10
    return mask


def _ring(mask, pad=4):
    grown = np.zeros_like(mask)
    h, w = mask.shape
    ys, xs = np.nonzero(mask)
    for y, x in zip(ys, xs):
        grown[max(0, y - pad):min(h, y + pad + 1), max(0, x - pad):min(w, x + pad + 1)] = True
    return grown & ~mask


def rank(capture, dump, region=CAPTURE_BLOCK):
    shot = np.asarray(Image.open(capture).convert('RGB')).astype(np.float32)
    inside = _block_mask(shot, region)
    if not inside.any():
        sys.exit(f'{capture}: no blocks found in {region} — nothing to rank')
    ring = _ring(inside)
    print(f'{capture}: {int(inside.sum())} block pixels, {int(ring.sum())} ring pixels')
    rows = []
    for p in sorted(glob.glob(os.path.join(dump, 'f-*.png'))):
        n = int(os.path.basename(p)[2:-4])
        if n % 100:                     # mip 0 only: a mipped view is a different surface
            continue
        im = Image.open(p).convert('RGB')
        if im.size != (shot.shape[1], shot.shape[0]):
            im = im.resize((shot.shape[1], shot.shape[0]), Image.NEAREST)
        rgb = np.asarray(im).astype(np.float32)
        zero = float((rgb.sum(2)[inside] == 0).mean())
        rows.append((n // 100, zero, float(rgb.mean(2)[inside].mean()), float(rgb.mean(2)[ring].mean())))
    rows.sort(key=lambda r: -r[1])
    print(f'{"serial":>8} {"zero%":>7} {"inside":>8} {"ring":>8}  verdict')
    for ser, zero, i, r in rows:
        if r < 20:
            v = 'both dark (empty there)'
        elif zero > 0.3:
            v = 'CARRIES the blocks'
        elif zero < 0.05 and i > 30:
            v = 'clean'
        else:
            v = 'partial'
        print(f'{ser:>8} {zero*100:6.1f}% {i:8.1f} {r:8.1f}  {v}')


def main(argv):
    if len(argv) < 2:
        sys.exit(__doc__)
    cmd = argv[1]
    if cmd == 'chain' and len(argv) == 3:
        chain(argv[2])
    elif cmd == 'rank' and len(argv) >= 4:
        region = tuple(int(v) for v in argv[4].split(',')) if len(argv) > 4 else CAPTURE_BLOCK
        rank(argv[2], argv[3], region)
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main(sys.argv)
