/* Horizon `friend:u` IPC service (2026-10-22): the friend-service creator the game opens through sm:
   to build its notification queue (nn::friends::NotificationQueue::Initialize). Command numbers and
   layouts follow switchbrew "Friend services". Only the notification path the game reached is answered;
   every other command still stops the runtime with its id. */
#include "runtime.h"
#include "ipc.h"
#include <stdio.h>
#include <string.h>

/* ---------- IFriendService ----------
   Returned by the creator's CreateFriendService. The notification path only stores this session; it
   sends no command on it, so the handler only reports. An unobserved command still stops the runtime. */
static unsigned friend_service_calls;
static void friend_service_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    (void)rs;
    fprintf(stderr, "[nfs-friends] IFriendService #%u cmd=%u data=%u\n",
            ++friend_service_calls, rq->cmd, rq->data_size);
    ipc_unimplemented(s, rq);
}

/* ---------- INotificationService ----------
   The queue the game holds for friend-presence/notification events. Offline there are no friends and
   no notifications: the event is never signalled and Pop has nothing to return. The writable end is
   kept here so the event outlives any single GetEvent copy handle. */
static unsigned notification_calls;
static void notification_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    fprintf(stderr, "[nfs-friends] INotificationService #%u cmd=%u data=%u\n",
            ++notification_calls, rq->cmd, rq->data_size);
    switch (rq->cmd) {
    case 0: {                                                              /* GetEvent: readable event the game waits on */
        static uint32_t ev; if (!ev) ev = runtime_event_new();
        rs->copy[rs->ncopy++] = runtime_event_readable(ev);
        return;
    }
    case 1:                                                                /* Clear: no queued notifications to drop */
        return;
    case 2:                                                                /* Pop: nothing queued, so nothing to return */
        return;
    default: ipc_unimplemented(s, rq);
    }
}

/* ---------- friend:u: IServiceCreator ----------
   The creator answers only the commands the game is observed to send on the notification path:
   CreateFriendService (0) first, then CreateNotificationService (1). CreateDaemonSuspendSessionService
   (2) is not on that path and remains a report-on-call stub. */
static unsigned friend_calls;
static void friend_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    fprintf(stderr, "[nfs-friends] friend:u #%u cmd=%u data=%u\n",
            ++friend_calls, rq->cmd, rq->data_size);
    switch (rq->cmd) {
    case 0:                                                                /* CreateFriendService -> IFriendService */
        rs->move[rs->nmove++] = runtime_ipc_new_session("IFriendService", friend_service_handler, NULL, NULL);
        return;
    case 1:                                                                /* CreateNotificationService(userID) -> INotificationService */
        rs->move[rs->nmove++] = runtime_ipc_new_session("INotificationService", notification_handler, NULL, NULL);
        return;
    default: ipc_unimplemented(s, rq);
    }
}

__attribute__((constructor)) static void register_friend_service(void) {
    runtime_ipc_register("friend:u", friend_handler, NULL, NULL);
}
