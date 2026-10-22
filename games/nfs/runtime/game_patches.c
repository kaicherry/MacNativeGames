#include "game_patches.h"
#include <stdio.h>

/* Need for Speed: no code patches are wired yet. A patch table arrives with the first
   patch that needs one, and every site will be checked against the loaded bytes before
   it is written (a wrong version or a rewritten word must never be patched). */
void runtime_game_patches(Module *main_module) {
    (void)main_module;
    fprintf(stderr, "[nfs] game patches: none wired\n");
}

/* The host hooks the shared graphics layer and the input keys call keep their names and answer "off". */
void runtime_frame_step_pin(double ms) { (void)ms; }
void runtime_frame_step_update(uint64_t now_ns) { (void)now_ns; }
int runtime_output_res(unsigned *w, unsigned *h) { (void)w; (void)h; return 0; }
uint64_t runtime_output_res_extra_bytes(void) { return 0; }
uint64_t runtime_output_size_current(void) { return (uint64_t)1080 << 32 | 1920; }
int runtime_output_res_toggle(void) { return 0; }
int runtime_output_mode_cycle(void) { return 0; }
int runtime_aspect_toggle(void) { return 0; }
float runtime_aspect_now(void) { return 0; }
