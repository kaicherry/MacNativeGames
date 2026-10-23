#include "game_patches.h"
#include "runtime.h"
#include <stdio.h>
#include <string.h>

/* The SDK's semaphore acquires decrement a credit count under the semaphore's own critical section and issue no SVC,
   so the credit ledger sees the count drop and cannot name the thread that made it. Each store below is replaced by
   udf #0 and the SIGILL handler (loader.c) performs that store from the faulting thread's registers, so a run's
   ledger names the taker. A site is written only when the loaded word is the one the sdk image was verified to hold:
   a different SDK must fail loudly rather than report a take that never happened. All three store the count from w8. */
typedef struct { uint64_t off; uint32_t original; unsigned val_reg, base_reg; } SemTakeSite;
static const SemTakeSite sem_take_sites[] = {
    {0x33b924, 0xb9001688, 8, 20},   /* AcquireSemaphore: count-- then str w8, [x20, #0x14] */
    {0x33b96c, 0xb9001688, 8, 20},   /* TryAcquireSemaphore */
    {0x33ba0c, 0xb9001668, 8, 19},   /* TimedAcquireSemaphore: str w8, [x19, #0x14] */
};

/* Need for Speed: no behaviour patch is wired yet. A behaviour table arrives with the first patch that needs one, and
   every site will be checked against the loaded bytes before it is written (a wrong version or a rewritten word must
   never be patched). */
void runtime_game_patches(Module *main_module) {
    Module *m = main_module;
    if (strcmp(m->name, "sdk")) {
        fprintf(stderr, "[nfs] game patches: %s: none wired\n", m->name);
        return;
    }
    const uint32_t udf = 0;
    for (unsigned i = 0; i < sizeof sem_take_sites / sizeof *sem_take_sites; i++) {
        const SemTakeSite *s = &sem_take_sites[i];
        uint32_t have;
        memcpy(&have, m->base + s->off, 4);
        if (have != s->original) {
            fprintf(stderr, "[nfs] semaphore take site sdk+%#llx: found %08x expected %08x\n",
                    (unsigned long long)s->off, have, s->original);
            runtime_fail("semaphore take site: original instruction mismatch");
        }
        memcpy(m->base + s->off, &udf, 4);
        runtime_sem_take_site((uintptr_t)m->base + s->off, s->val_reg, s->base_reg);
    }
    fprintf(stderr, "[nfs] semaphore take sites: %zu sdk instructions trap to the SIGILL handler, which names the taking thread\n",
            sizeof sem_take_sites / sizeof *sem_take_sites);
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
