/* Horizon `pctl` IPC service: the game opens it after it accepts the save-data access notice and mounts
   its save directory. This host has no parental controls, so every query answers unrestricted (rc 0,
   zeroed out buffers); an unrecognised command is logged with its descriptor shape rather than guessed. */
#include "runtime.h"
#include "ipc.h"
#include <stdio.h>
#include <string.h>

/* ---------- pctl: handler ---------- */
static unsigned pctl_call_count;

static void pctl_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    (void)s;
    fprintf(stderr, "[nfs-pctl] #%u cmd=%u type=%u x%u a%u b%u w%u c%u data=%u\n",
            ++pctl_call_count, rq->cmd, rq->type, rq->nx, rq->na, rq->nb, rq->nw, rq->nc, rq->data_size);
    if (rq->data_size > 0 && rq->data_size <= 256) {
        const uint32_t *d = (const uint32_t *)rq->data;
        unsigned words = rq->data_size / 4;
        fprintf(stderr, "[nfs-pctl]   in:");
        for (unsigned i = 0; i < words && i < 16; i++) fprintf(stderr, " %08x", d[i]);
        fputc('\n', stderr);
    }
    /* A query's value is the client's own output buffer (C): zero is "not restricted", written explicitly
       rather than left at whatever the guest held, so the value is this host's and not the guest's. */
    for (unsigned i = 0; i < rq->nc; i++) {
        if (!rq->c[i].addr || !rq->c[i].size) continue;
        memset(ipc_guest(rq->c[i].addr, rq->c[i].size), 0, rq->c[i].size);
    }
    uint32_t ret = 0;
    IPC_OUT(rs, ret);
}

__attribute__((constructor)) static void register_pctl_service(void) {
    runtime_ipc_register("pctl", pctl_handler, NULL, NULL);
}
