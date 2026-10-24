# Need for Speed Hot Pursuit Remastered (games/nfs)

The native port tree for title `010029B0118E8000` v0 (CNMT version `0`, display `1.0.0`), self-contained like
`games/by3`: the Horizon host (`runtime/`), the binary rewriter (`third_party/d2r`), the NVN-on-Metal graphics
layer (`third_party/d2r_gfx`) and the preparation scripts (`scripts/`) are this folder's own copies, forked from
`games/by3` (itself forked from `games/totk`). Nothing here reads either tree.

Every build input is checked against `assets/expected-v1.0.0.json` first — the six ExeFS files, the five rewritten
images, the RomFS image, and the six shared-font reference pins. The manifest is the authority; a value is never
re-derived here.

Build and boot (the root `build_app.py` drives the code/data steps from the original NSP):

    python3 build_app.py <game.nsp> -k <prod.keys> --firmware <unpacked firmware> --work <work>
    make -C games/nfs WORK=<work>/nfs prepare
    make -C games/nfs WORK=<work>/nfs runtime
    make -C games/nfs WORK=<work>/nfs shader-libs   # compile the runtime shader cache ahead of the boot
    make -C games/nfs WORK=<work>/nfs boot

The runtime load set is the title's whole ExeFS: `rtld`, `main`, `sdk`, `subsdk0`, `subsdk1` as mapped rewrite
images, and `main.npdm` loaded as process metadata (digest bound by `scripts/prepare_runtime.py`). `make boot`
runs from the work folder and writes its log to `<work>/nfs/logs/boot.log`.

A dev-tree boot carries no shader seed, so every variant it meets is compiled on demand. `make shader-libs`
(`scripts/build_shader_libs.py`) compiles the work tree's own runtime cache, `build/shader-cache/rt-<chain>`, into
`build/shader-libs/<SHA-256 of the MSL>.metallib` — the exact name `nvn_mtl.m` `shader_load` prefers over compiling
(a packaged app gets the same files from `build_app.py step_shader_libs`). S2.26 measured `0 loaded precompiled,
601 compiled on demand` before and `601 loaded precompiled, 0 compiled on demand` after. The cache fills during a
boot, so the first boot in a fresh tree compiles as before; this has something to compile from the second.

Status: the loader boot is being brought up. No gameplay claim is made. The shader translator is carried in
`prebuilt/macos-arm64/` and `make boot` builds it (with `build/spirv-cross/spirv-cross`) so the headless boot can
translate shaders; the launcher bundle is not wired yet (it arrives with the app-assembly slice).
