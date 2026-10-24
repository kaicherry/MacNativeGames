/* Horizon `savedata` IPC service (S3.1c): the host stub for NSYSAPP save-data operations.
    The game opens this family after accepting the save-data access notice and mounting its
    save directory. This host has no Nintendo save-data storage, so all queries answer from
    local state (the NFS title ID, the local account UID) and every write command is logged
    but not yet persisted. The mount/commit handshake is handled by fs_host.c; this service
    owns the IPC surface the SDK talks to.

    Standard ns:save_data sub-services:
    - saveDataBinary: ISaveDataBinary  — initialize, set key, basic operations
    - saveDataAudit:  ISaveDataAudit   — audit log queries
    - saveDataConfig: ISaveDataConfig  — config queries (user ID, app ID)
    - ISaveData:      the generic save-data object returned by the family

    Each factory returns a moved session (Do-Not #23). Unknown commands are logged and
    answered with rc 0 so the boot can proceed; a real implementation would stop here. */
#include "runtime.h"
#include "fs_host.h"
#include "ipc.h"
#include <stdio.h>
#include <string.h>

/* NFS title ID: the only application ID this host serves. */
static const uint64_t kApplicationIdLo = 0x010029B0118E8000ULL;

/* ---------- ISaveDataConfig ----------
   Returns the current user's UID and the title's application ID.
   Commands: Initialize(0), GetSaveDataUserId(1), GetSaveDataApplicationId(2). */
static unsigned config_calls;

static void config_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    (void)s;
    fprintf(stderr, "[nfs-savedata] ISaveDataConfig #%u cmd=%u type=%u data=%u\n",
            ++config_calls, rq->cmd, rq->type, rq->data_size);

    switch (rq->cmd) {
    case 0: {                                                            /* Initialize */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    case 1: {                                                            /* GetSaveDataUserId */
        uint64_t uid[2];
        runtime_account_uid(uid);
        IPC_OUT(rs, uid[0]);
        IPC_OUT(rs, uid[1]);
        return;
    }
    case 2: {                                                            /* GetSaveDataApplicationId */
        uint64_t app_id = kApplicationIdLo;
        IPC_OUT(rs, app_id); return;
    }
    default:
        fprintf(stderr, "[nfs-savedata] ISaveDataConfig cmd %u not implemented\n", rq->cmd);
        rs->result = 0; return;
    }
}

/* ---------- ISaveDataAudit ----------
   Audit-log queries. All return zeroed or empty on this host.
   Commands: Initialize(0), GetTotalSavedBytes(1), GetSaveDataSize(2). */
static unsigned audit_calls;

static void audit_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    (void)s;
    fprintf(stderr, "[nfs-savedata] ISaveDataAudit #%u cmd=%u type=%u data=%u\n",
            ++audit_calls, rq->cmd, rq->type, rq->data_size);

    switch (rq->cmd) {
    case 0: {                                                            /* Initialize */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    case 1: {                                                            /* GetTotalSavedBytes */
        uint64_t bytes = 0;
        IPC_OUT(rs, bytes); return;
    }
    case 2: {                                                            /* GetSaveDataSize */
        uint64_t size = 0;
        IPC_OUT(rs, size); return;
    }
    default:
        fprintf(stderr, "[nfs-savedata] ISaveDataAudit cmd %u not implemented\n", rq->cmd);
        rs->result = 0; return;
    }
}

/* ---------- ISaveDataBinary ----------
   The primary save-data object. Initialize and SetKey are the observed commands.
   Other commands are stubbed with rc 0 for now.
   Commands: Initialize(0), SetKey(1), Open(2), Create(3), Delete(4). */
static unsigned binary_calls;

static void binary_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    (void)s;
    fprintf(stderr, "[nfs-savedata] ISaveDataBinary #%u cmd=%u type=%u data=%u\n",
            ++binary_calls, rq->cmd, rq->type, rq->data_size);

    switch (rq->cmd) {
    case 0: {                                                            /* Initialize */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    case 1: {                                                            /* SetKey — sets the save-data key string */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    case 2: {                                                            /* Open — open a save-data slot */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    case 3: {                                                            /* Create — create a save-data slot */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    case 4: {                                                            /* Delete — delete a save-data slot */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    default:
        fprintf(stderr, "[nfs-savedata] ISaveDataBinary cmd %u not implemented\n", rq->cmd);
        rs->result = 0; return;
    }
}

/* ---------- ISaveData (generic) ----------
   The base save-data interface. The game uses Initialize and a small set of
   commands on this object. Unknown commands logged and answered rc 0. */
static unsigned savesave_calls;

static void savesave_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    (void)s;
    fprintf(stderr, "[nfs-savedata] ISaveData #%u cmd=%u type=%u data=%u\n",
            ++savesave_calls, rq->cmd, rq->type, rq->data_size);

    switch (rq->cmd) {
    case 0: {                                                            /* Initialize */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    default:
        fprintf(stderr, "[nfs-savedata] ISaveData cmd %u not implemented\n", rq->cmd);
        rs->result = 0; return;
    }
}

/* ---------- savedata: IServiceCreator ----------
   The main handler. The guest sends CreateService / CreateServiceWithoutInitialize
   on the factory, which returns a sub-service session. The guest may also connect
   directly to the factory to query the family.

   Factory commands:
   - CreateService(0):  x1 = sub-service name (8 bytes), w1 = flags
   - CreateServiceWithoutInitialize(1): same, returns an initialized session */
static unsigned savedata_calls;

static void savedata_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    (void)s;
    fprintf(stderr, "[nfs-savedata] savedata #%u cmd=%u type=%u data=%u\n",
            ++savedata_calls, rq->cmd, rq->type, rq->data_size);

    switch (rq->cmd) {
    case 0: {                                                            /* CreateService */
        char name[9] = {0};
        memcpy(name, ipc_arg(rq, 0, 8), 8);
        fprintf(stderr, "[nfs-savedata] CreateService: \"%s\"\n", name);

        if (strcmp(name, "saveDataBinary") == 0) {
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/saveDataBinary", binary_handler, NULL, NULL);
        } else if (strcmp(name, "saveDataAudit") == 0) {
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/saveDataAudit", audit_handler, NULL, NULL);
        } else if (strcmp(name, "saveDataConfig") == 0) {
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/saveDataConfig", config_handler, NULL, NULL);
        } else if (strcmp(name, "ISaveData") == 0) {
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/ISaveData", savesave_handler, NULL, NULL);
        } else {
            fprintf(stderr, "[nfs-savedata] unknown sub-service: \"%s\"\n", name);
            /* Register it on the fly and return a generic handler. */
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/unknown", savesave_handler, (void *)(uintptr_t)name, NULL);
        }
        return;
    }
    case 1: {                                                            /* CreateServiceWithoutInitialize */
        char name[9] = {0};
        memcpy(name, ipc_arg(rq, 0, 8), 8);
        fprintf(stderr, "[nfs-savedata] CreateServiceWithoutInitialize: \"%s\"\n", name);

        if (strcmp(name, "saveDataBinary") == 0) {
            /* CreateServiceWithoutInitialize: return the session directly.
               The first command the SDK sends on it is Initialize; the handler answers rc 0.
               Calling Initialize here would require sending a sync request back to the guest,
               which the IPC layer does not support in a constructor path. */
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/saveDataBinary", binary_handler, NULL, NULL);
        } else if (strcmp(name, "saveDataAudit") == 0) {
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/saveDataAudit", audit_handler, NULL, NULL);
        } else if (strcmp(name, "saveDataConfig") == 0) {
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/saveDataConfig", config_handler, NULL, NULL);
        } else if (strcmp(name, "ISaveData") == 0) {
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/ISaveData", savesave_handler, NULL, NULL);
        } else {
            fprintf(stderr, "[nfs-savedata] unknown sub-service: \"%s\"\n", name);
            rs->move[rs->nmove++] = runtime_ipc_new_session(
                "savedata/unknown", savesave_handler, (void *)(uintptr_t)name, NULL);
        }
        return;
    }
    default:
        fprintf(stderr, "[nfs-savedata] command %u not implemented\n", rq->cmd);
        rs->result = 0; return;
    }
}

/* ---------- Registration ----------
   Register the main `savedata` service. The guest may also open
   `ns:save_data` (the standard NSYSAPP name); register both. */
__attribute__((constructor)) static void register_savedata_service(void) {
    runtime_ipc_register("savedata", savedata_handler, NULL, NULL);
    runtime_ipc_register("ns:save_data", savedata_handler, NULL, NULL);
}
