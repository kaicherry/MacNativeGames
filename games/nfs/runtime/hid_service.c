/* Horizon `hid:` IPC service (2026-10-22): the service the game queries via sm: to get controller input.
   Commands follow the switchbrew "HID services" layout. The runtime already handles the game's
   nn::hid SDK calls through PLT overrides (runtime/hid_feedback.c, runtime/input.c); this service
   answers the IPC queries the game sends at init. Unknown commands still stop the runtime with their id. */
#include "runtime.h"
#include "ipc.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ---------- State ----------
   One logical controller (a Mac-connected Gamepad via GameController framework).
   The game's nn::hid SDK calls bypass this service (nn::hid::utility computes handles itself);
   the hid service owns the application-control-user / vibration / six-axis / battery queries. */

/* Single connected user handle (nn::account persistence; the account service is already HLE'd). */
static const uint64_t kUserHandle = 1;

/* NpadStyleTag bitmask: FullKey(1), JoyDual(4), JoyLeft(8), JoyRight(16), Handheld(32) —
   switchbrew "NpadStyleTag". All reported as available so the game can choose. */
static const uint32_t kSupportedStyles = 0x3f;

/* Battery: on Mac there is no Switch battery, so report 100% and fully charged. */
enum { BATTERY_LEVEL_FULL = 4, BATTERY_STATUS_FULL = 2 };

/* ---------- Helpers ---------- */

/* Write up to `count` vibration device handles to a guest output buffer.
   Handle layout: byte 0 = style tag, byte 3 = full-key(3), handheld(4); byte 1 = controller id(0).
   Vibration device 0 = left (index 0), 1 = right (index 1). */
static void write_vibration_handles(uint64_t addr, uint32_t count) {
    if (!count) return;
    uint32_t *out = ipc_guest(addr, count * sizeof(uint32_t));
    for (uint32_t i = 0; i < count; i++) {
        /* style=full-key(0x03), controller_id=0, index=i, pad=0 */
        out[i] = 0x03000000u | (i << 16);
    }
}

/* ---------- hid: handler ---------- */
static void hid_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    switch (rq->cmd) {
    case 0: {                                                            /* Initialize / RegisterClient */
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    case 1: {                                                            /* GetApplicationControlUserHandle */
        IPC_OUT(rs, kUserHandle); return;
    }
    case 2: {                                                            /* GetNewestControlUserHandle */
        IPC_OUT(rs, kUserHandle); return;
    }
    case 3: {                                                            /* GetControlUserHandle */
        uint64_t app_user = IPC_ARG(rq, uint64_t, 0);
        (void)app_user;
        IPC_OUT(rs, kUserHandle); return;
    }
    case 4: {                                                            /* GetSupportedNpadStyleSet */
        IPC_OUT(rs, kSupportedStyles); return;
    }
    case 5: {                                                            /* IsNewestControlUserEnabled */
        uint8_t enabled = 1;
        IPC_OUT(rs, enabled); return;
    }
    case 6: {                                                            /* GetVibrationDeviceHandles */
        uint64_t out_addr = IPC_ARG(rq, uint64_t, 0);
        uint32_t capacity = IPC_ARG(rq, uint32_t, 4);
        uint32_t count = capacity == 0 ? 0 : (capacity < 2 ? capacity : 2);
        if (count) write_vibration_handles(out_addr, count);
        IPC_OUT(rs, count); return;
    }
    case 7: {                                                            /* GetSixAxisSensorHandle */
        uint64_t out_addr = IPC_ARG(rq, uint64_t, 0);
        uint32_t capacity = IPC_ARG(rq, uint32_t, 4);
        (void)out_addr; (void)capacity;
        /* One six-axis sensor per controller. On Mac we have no real switch controllers,
           so we return zero: no six-axis sensor available. */
        uint32_t count = 0;
        IPC_OUT(rs, count); return;
    }
    case 8: {                                                            /* GetBatteryInfo */
        uint32_t user_id = IPC_ARG(rq, uint32_t, 0);
        (void)user_id;
        uint8_t level = BATTERY_LEVEL_FULL;
        uint8_t status = BATTERY_STATUS_FULL;
        IPC_OUT(rs, level);
        IPC_OUT(rs, status); return;
    }
    case 9: {                                                            /* IsUserAttached (deprecated) */
        uint8_t attached = 1;
        IPC_OUT(rs, attached); return;
    }
    case 10: {                                                           /* GetNpadCounts */
        uint64_t out_addr = IPC_ARG(rq, uint64_t, 0);
        if (!out_addr) runtime_fail("hid GetNpadCounts output buffer");
        struct { uint32_t connected; uint32_t disconnected; } counts = {1, 0};
        memcpy(ipc_guest(out_addr, sizeof(counts)), &counts, sizeof(counts));
        uint32_t ret = 0;
        IPC_OUT(rs, ret); return;
    }
    case 11: {                                                           /* GetSystemProperties */
        uint64_t out_addr = IPC_ARG(rq, uint64_t, 0);
        if (!out_addr) runtime_fail("hid GetSystemProperties output buffer");
        /* Three properties: model(0), language(1), userCount(2). */
        const int nProps = 3;
        typedef struct { uint32_t key; uint64_t value; } SystemProp;
        SystemProp props[nProps];
        props[0].key = 0; props[0].value = 1;   /* model: docked(1) */
        props[1].key = 1; props[1].value = 1;   /* language: en-US(1) */
        props[2].key = 2; props[2].value = 1;   /* userCount: 1 */
        memcpy(ipc_guest(out_addr, nProps * sizeof(SystemProp)), props, nProps * sizeof(SystemProp));
        IPC_OUT(rs, nProps); return;
    }
    case 12: {                                                           /* GetNpadAddr */
        uint64_t addr = 0;
        IPC_OUT(rs, addr); return;
    }
    default:
        fprintf(stderr, "[totk-hid] command %u not implemented\n", rq->cmd);
        ipc_unimplemented(s, rq);
    }
}

__attribute__((constructor)) static void register_hid_service(void) {
    runtime_ipc_register("hid:", hid_handler, NULL, NULL);
}
