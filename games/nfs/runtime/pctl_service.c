/* Horizon `pctl` IPC service: the game opens it after it accepts the save-data access notice and mounts
   its save directory. This host has no parental controls, so every query answers unrestricted (rc 0,
   zeroed out buffers); an unrecognised command is logged with its descriptor shape rather than guessed.

   A factory-family session answers CreateService / CreateServiceWithoutInitialize with a service object.
   That object is the value the SDK stores and then calls Initialize through: a result-only reply leaves
   it dereferencing nothing (`sdk+0x3489dc`, S3.1b). */
#include "runtime.h"
#include "ipc.h"
#include <stdio.h>
#include <string.h>

/* ---------- pctl: handler ---------- */
static unsigned pctl_call_count;

static void pctl_log(const char *who, const IpcRequest *rq) {
    fprintf(stderr, "[nfs-pctl] %s #%u cmd=%u type=%u x%u a%u b%u w%u c%u data=%u\n",
            who, ++pctl_call_count, rq->cmd, rq->type, rq->nx, rq->na, rq->nb, rq->nw, rq->nc, rq->data_size);
    if (rq->data_size > 0 && rq->data_size <= 256) {
        const uint32_t *d = (const uint32_t *)rq->data;
        unsigned words = rq->data_size / 4;
        fprintf(stderr, "[nfs-pctl]   in:");
        for (unsigned i = 0; i < words && i < 16; i++) fprintf(stderr, " %08x", d[i]);
        fputc('\n', stderr);
    }
}

/* A query's value is the client's own output buffer (C): zero is "not restricted", written explicitly
   rather than left at whatever the guest held, so the value is this host's and not the guest's. */
static void pctl_unrestricted(const IpcRequest *rq) {
    for (unsigned i = 0; i < rq->nc; i++) {
        if (!rq->c[i].addr || !rq->c[i].size) continue;
        memset(ipc_guest(rq->c[i].addr, rq->c[i].size), 0, rq->c[i].size);
    }
}

/* The `IParentalControlService` a factory hands back. `state` is the factory's name, so a line names the
   object as that factory's rather than as a fifth service the guest never asked for. */
static void pctl_service_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    char who[32];
    snprintf(who, sizeof who, "%s/service", s->state ? (const char *)s->state : s->name);
    pctl_log(who, rq);
    pctl_unrestricted(rq);
    rs->result = 0;
}

static void pctl_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    pctl_log(s->name, rq);
    pctl_unrestricted(rq);
    switch (rq->cmd) {
    case 0:                                                     /* CreateService */
    case 1:                                                     /* CreateServiceWithoutInitialize */
        /* The reply's value is the object itself — respond() moves the session into the client's domain
           and the SDK then calls Initialize on it — so a result-only reply is not the shape it reads. */
        rs->move[rs->nmove++] = runtime_ipc_new_session(
            "pctl:service", pctl_service_handler, (void *)(uintptr_t)s->name, NULL);
        return;
    default:
        rs->result = 0;
        return;
    }
}

/* The guest opens the whole pctl family in turn — `pctl`, `pctl:s`, `pctl:a`, then `pctl:r` — and sends
   cmd=1 to each; `sm` fails the boot for any name it does not hold, so every observed name is registered. */
__attribute__((constructor)) static void register_pctl_service(void) {
    static const char *const names[] = { "pctl", "pctl:s", "pctl:a", "pctl:r" };
    for (unsigned i = 0; i < sizeof names / sizeof *names; i++)
        runtime_ipc_register(names[i], pctl_handler, NULL, NULL);
}
