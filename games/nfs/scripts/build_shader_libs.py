#!/usr/bin/env python3
"""Compile the runtime shader cache ahead of time, so a dev-tree boot loads its shaders instead of compiling them.

nvn_mtl.m shader_load: a variant in build/shader-cache/rt-<chain>/<key>_b.metal is loaded from
build/shader-libs/<SHA-256 of the MSL>.metallib when that exact file is there, and compiled from the source
otherwise. A packaged app gets those libraries from build_app.py step_shader_libs (the app carries a shader
seed); a dev-tree boot has no seed, so every variant it meets is compiled on demand -- S2.25 measured
`0 loaded precompiled, 601 compiled on demand` with a 3,937-entry rt- cache sitting right there. This writes
the libraries from that cache, named by source hash, so a rebuild reuses every shader that did not change.
"""
import argparse
import concurrent.futures
import hashlib
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

# nvn_mtl.m totk_precise_opts(): the translated shaders need exactly rounded float arithmetic.
METAL_FLAGS = ['-fmetal-math-mode=safe']


def repo_root():
    return Path(__file__).resolve().parent.parent  # games/nfs


def chain(root):
    text = (root / 'third_party/d2r_gfx/gfx/nvn/dmsh_chain.h').read_text()
    m = re.search(r'#define\s+DMSH_CHAIN\s+"([^"]+)"', text)
    if not m:
        raise SystemExit('dmsh_chain.h declares no DMSH_CHAIN')
    return m.group(1)


def developer_dirs():
    """Developer folders that may hold the Metal toolchain: DEVELOPER_DIR, the one xcode-select points at, then
    Xcode*.app in the usual places (one kept on another volume included)."""
    found = [os.environ['DEVELOPER_DIR']] if os.environ.get('DEVELOPER_DIR') else []
    try:
        found.append(subprocess.check_output(['xcode-select', '-p'], text=True, stderr=subprocess.DEVNULL).strip())
    except (OSError, subprocess.CalledProcessError):
        pass
    found += [str(p / 'Contents/Developer') for p in sorted(Path('/Applications').glob('Xcode*.app'))]
    found += [str(p / 'Contents/Developer') for p in sorted(Path.home().glob('Applications/Xcode*.app'))]
    found += [str(p / 'Contents/Developer') for p in sorted(Path('/Volumes').glob('*/Xcode*.app'))]
    found += [str(p / 'Contents/Developer') for p in sorted(Path('/Volumes').glob('*/*/Xcode*.app'))]
    return list(dict.fromkeys(d for d in found if d))


def find_metal():
    """(compiler, its version line, the environment to run it with), or None. Apple's offline shader compiler is an
    Xcode's optional Metal toolchain, not the command line tools, so the other Xcodes on this Mac are tried."""
    for developer in developer_dirs():
        env = dict(os.environ, DEVELOPER_DIR=developer)
        found = subprocess.run(['xcrun', '--find', 'metal'], capture_output=True, text=True, env=env)
        metal = found.stdout.strip()
        if found.returncode or not metal:
            continue
        version = subprocess.run([metal, '--version'], capture_output=True, text=True, env=env)
        if not version.returncode and version.stdout:
            return metal, version.stdout.splitlines()[0], env
    return None


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--work', default='.', help='work folder whose build/ holds shader-cache/ and shader-libs/')
    ap.add_argument('--cache', default=None, help='runtime translation cache (default <work>/build/shader-cache/rt-<chain>)')
    ap.add_argument('--out', default=None, help='output folder (default <work>/build/shader-libs)')
    ap.add_argument('--chain', default=None, help='translation chain (default: DMSH_CHAIN in dmsh_chain.h)')
    ap.add_argument('--metal', default=None, help='Metal compiler to run (default: xcrun --find metal)')
    ap.add_argument('--threads', type=int, default=os.cpu_count() or 4)
    args = ap.parse_args(argv)

    work = Path(args.work)
    ch = args.chain or chain(repo_root())
    cache = Path(args.cache) if args.cache else work / f'build/shader-cache/rt-{ch}'
    out = Path(args.out) if args.out else work / 'build/shader-libs'
    sources = sorted(cache.glob('*.metal'))
    if not sources:
        print(f'shader-libs: no runtime translation cache at {cache}; nothing to compile')
        return 0

    if args.metal:
        metal, env, identity = args.metal, dict(os.environ), args.metal
    else:
        found = find_metal()
        if not found:
            print('shader-libs: no Metal toolchain (xcodebuild -downloadComponent MetalToolchain); '
                  'the boot compiles its shaders on demand', file=sys.stderr)
            return 0
        version, env = found[1], found[2]
        metal = found[0]
        identity = ' '.join([version, *METAL_FLAGS])

    out.mkdir(parents=True, exist_ok=True)
    stamp = out / 'compiler.txt'
    if stamp.is_file() and stamp.read_text() != identity:
        # The libraries were built with other options or another compiler; the hash names the source, not the build.
        shutil.rmtree(out)
        out.mkdir(parents=True, exist_ok=True)
    stamp.write_text(identity)

    todo, reused = {}, 0
    for src in sources:
        digest = hashlib.sha256(src.read_bytes()).hexdigest()
        if (out / f'{digest}.metallib').is_file():
            reused += 1
        else:
            todo.setdefault(digest, src)

    def compile_one(item):
        digest, src = item
        partial = out / f'{digest}.partial'
        r = subprocess.run([metal, *METAL_FLAGS, '-o', str(partial), str(src)], capture_output=True, text=True, env=env)
        if r.returncode:
            partial.unlink(missing_ok=True)
            return f'{src.name}\n{r.stdout}{r.stderr}'
        partial.replace(out / f'{digest}.metallib')

    with concurrent.futures.ThreadPoolExecutor(args.threads) as pool:
        failed = [f for f in pool.map(compile_one, todo.items()) if f]
    print(f'shader-libs: {len(todo) - len(failed)} compiled, {reused} reused, {len(failed)} failed of {len(sources)} in {cache}')
    if failed:  # the boot compiles these from source, as before
        print('shader-libs: compiled at launch instead:\n' + '\n'.join(failed), file=sys.stderr)
    return 0


if __name__ == '__main__':
    sys.exit(main())
