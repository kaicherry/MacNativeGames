/* Need for Speed Hot Pursuit Remastered 1.0.0 (main build ID 3515E683…; none of the five modules matches the
   registered D2R build ID, so every rewrite is the generic x18 / thread-pointer / SVC / counter substitution).
   No game-code address is defined on purpose: the TotK GAME_ADDR maps do not apply, so any leftover use is a
   compile error. NFS addresses get named defines here when a patch needs one. */
#pragma once
#define GAME_NFS 1
#define TOTK_GAME_VERSION "1.0.0"
#define TOTK_GAME_DIR "nfs100"
#define GAME_FOLDER "NFS"   /* ~/Library/Application Support/MacNativeGames/<GAME_FOLDER> (app/launcher.m) */
#define GAME_LOG "nfs"      /* ~/Library/Logs/MacNativeGames/<GAME_LOG>.log */
#define GAME_TITLE_ID 0x010029b0118e8000ULL
