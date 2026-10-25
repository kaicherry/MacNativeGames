/* audout:u — nn::audio AudioOut, which Need for Speed's Wwise uses for its final mix (TOTK only uses the audio renderer,
   audren:u). Commands and layouts follow the public switchbrew description ("Audio services": IAudioOutManager, IAudioOut).
   An appended buffer's samples are mixed down to stereo and copied into a host ring (RING_MS); a feeder thread drains that
   ring into the Core Audio AudioQueue, holding CUSHION_MS enqueued, so a frame hitch drains the cushion and not the sound.
   The tag is handed back (GetReleasedAudioOutBuffers) as soon as its samples are in the ring, which is what lets the guest
   run ahead of the device instead of being paced by it. 5.1 output is mixed down to stereo (Switch order FL FR FC LFE BL BR).
   NFS_AUDIO=0 keeps the silent real-time sink (each buffer held for its duration) for tests without a sound device. */
#include "runtime.h"
#include "ipc.h"
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { STATE_STARTED = 0, STATE_STOPPED = 1, FORMAT_PCM16 = 2, DEFAULT_RATE = 48000 };
/* MONITOR_PERIOD_MS is the longest the sound may stay dead after the output device goes away. */
enum { MONITOR_PERIOD_MS = 200 };
/* STALL_TICKS monitor periods a started queue holding data may play nothing before it is rebuilt; the
   recorded trace advances ~9600 frames a period and a queue that has lost its device advances none. */
enum { STALL_TICKS = 3 };
/* RING_MS is the cushion between the guest's append and the device: it has to outlast the longest frame
   hitch measured under GPU submission (166 ms), because the device's own queue holds only ~10 ms.
   CUSHION_MS is what the feeder keeps enqueued so its own 5 ms wake cannot starve the device. */
enum { RING_MS = 240, CUSHION_MS = 40 };
/* RING_BLOCK_MS is how long an append waits for room before it drops its chunk: a guest outrunning the
   device for longer than this is not a hitch, and blocking its audio thread forever is worse. */
enum { RING_BLOCK_MS = 200 };
enum { CHUNK_MAX = 128, RELEASED_MAX = 128 };
typedef struct { uint64_t next, sample_buffer, capacity, data_size, data_offset; } AudioOutBuffer;   /* the client's descriptor */
_Static_assert(sizeof(AudioOutBuffer) == 0x28, "AudioOutBuffer");
typedef struct {
    pthread_mutex_t lock, aq_lock;
    pthread_cond_t wake;                               /* ring room or device room: append and the feeder wait here */
    pthread_t thread, feeder_thread; int running, silent;
    uint32_t rate, channels, state, event; float volume;
    AudioQueueRef aq;
    int16_t *ring; uint32_t ring_frames;               /* the cushion: stereo PCM between append and the device */
    uint64_t ring_head, ring_tail;                     /* frames read and written; index = n % ring_frames */
    uint32_t chunks[CHUNK_MAX]; unsigned chunk_head, chunk_count;   /* one appended buffer's frames, in ring order */
    uint32_t cushion_frames;                           /* frames the feeder keeps enqueued on the device */
    uint64_t device_frames; unsigned device_buffers;   /* enqueued on the device, not yet played */
    int16_t *staging;                                  /* feeder scratch: one chunk, stereo */
    uint64_t released[RELEASED_MAX]; unsigned nreleased;   /* appended and copied into the ring, not yet collected */
    uint64_t played_samples, dropped_chunks;
    uint64_t appends;                                  /* NFS_AUDIO_TRACE: appended buffers, and the loudest sample in them */
    unsigned peak_sample;
    uint64_t nz_samples, sum_abs, total_samples;       /* NFS_AUDIO_TRACE: is the appended audio continuous or sparse spikes? */
} AudioOut;

static uint64_t now_ns(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec; }
static void need(int ok, const char *what) { if (!ok) runtime_fail(what); }

static struct timespec deadline_ms(unsigned ms) {
    struct timespec t; clock_gettime(CLOCK_REALTIME, &t);
    t.tv_nsec += (long)ms * 1000000L;
    if (t.tv_nsec >= 1000000000L) { t.tv_sec++; t.tv_nsec -= 1000000000L; }
    return t;
}

/* Waits up to RING_BLOCK_MS for room for one whole chunk, with the lock held. Chunks are never split,
   or the device's buffer count would stop lining up with what the guest appended. */
static int ring_room(AudioOut *o, uint32_t frames) {
    struct timespec until = deadline_ms(RING_BLOCK_MS);
    while (o->running && (o->ring_frames - (uint32_t)(o->ring_tail - o->ring_head) < frames || o->chunk_count >= CHUNK_MAX))
        if (pthread_cond_timedwait(&o->wake, &o->lock, &until) == ETIMEDOUT) break;
    return o->ring_frames - (uint32_t)(o->ring_tail - o->ring_head) >= frames && o->chunk_count < CHUNK_MAX;
}
static void ring_write(AudioOut *o, const int16_t *stereo, uint32_t frames) {
    uint32_t at = (uint32_t)(o->ring_tail % o->ring_frames);
    uint32_t first = o->ring_frames - at < frames ? o->ring_frames - at : frames;
    memcpy(o->ring + 2 * at, stereo, (size_t)first * 4);
    if (frames > first) memcpy(o->ring, stereo + 2 * first, (size_t)(frames - first) * 4);
    o->ring_tail += frames;
}
static void ring_read(AudioOut *o, int16_t *stereo, uint32_t frames) {
    uint32_t at = (uint32_t)(o->ring_head % o->ring_frames);
    uint32_t first = o->ring_frames - at < frames ? o->ring_frames - at : frames;
    memcpy(stereo, o->ring + 2 * at, (size_t)first * 4);
    if (frames > first) memcpy(stereo + 2 * first, o->ring, (size_t)(frames - first) * 4);
    o->ring_head += frames;
}

/* The device has played a buffer: its samples leave the played count and the device's room comes back. */
static void played(void *user, AudioQueueRef q, AudioQueueBufferRef b) {   /* Core Audio thread */
    AudioOut *o = user;
    pthread_mutex_lock(&o->aq_lock);
    if (q != o->aq) { pthread_mutex_unlock(&o->aq_lock); return; }   /* a rebuild replaced it: the dispose frees the buffer */
    uint32_t frames = b->mAudioDataByteSize / 4;
    pthread_mutex_lock(&o->lock);
    o->played_samples += frames;
    o->device_frames = o->device_frames > frames ? o->device_frames - frames : 0;
    if (o->device_buffers) o->device_buffers--;
    pthread_cond_signal(&o->wake);                /* room on the device: the feeder may send the next chunk */
    pthread_mutex_unlock(&o->lock);
    AudioQueueFreeBuffer(q, b);
    pthread_mutex_unlock(&o->aq_lock);
}

/* Holds CUSHION_MS enqueued on the device from the ring. The guest's frame cadence cannot do this: a
   GPU frame of 120-166 ms stops its feed for longer than the device queue holds. */
static void *feeder(void *arg) {
    AudioOut *o = arg;
    for (;;) {
        pthread_mutex_lock(&o->lock);
        while (o->running && !(o->state == STATE_STARTED && o->chunk_count && o->device_frames < o->cushion_frames)) {
            struct timespec ts = deadline_ms(5);   /* a lost wake must not wedge the feeder */
            pthread_cond_timedwait(&o->wake, &o->lock, &ts);
        }
        if (!o->running) { pthread_mutex_unlock(&o->lock); return NULL; }
        uint32_t frames = o->chunks[o->chunk_head];
        if (o->device_frames && o->device_frames + frames > o->cushion_frames) {   /* over the cushion: wait for the device */
            struct timespec ts = deadline_ms(5);
            pthread_cond_timedwait(&o->wake, &o->lock, &ts);
            pthread_mutex_unlock(&o->lock);
            continue;
        }
        o->chunk_head = (o->chunk_head + 1) % CHUNK_MAX; o->chunk_count--;
        ring_read(o, o->staging, frames);
        o->device_frames += frames; o->device_buffers++;
        pthread_cond_signal(&o->wake);             /* ring room: wake an append that is waiting */
        pthread_mutex_unlock(&o->lock);

        int queued = 0;
        pthread_mutex_lock(&o->aq_lock);
        if (o->aq) {
            AudioQueueBufferRef qb;
            if (AudioQueueAllocateBuffer(o->aq, (UInt32)(frames * 4), &qb) == 0) {
                memcpy(qb->mAudioData, o->staging, (size_t)frames * 4);
                if (getenv("NFS_AUDIO_TEST")) {   /* a steady tone through this same queue: is the process audible at all? */
                    static double phase; static unsigned n; int tone = getenv("NFS_AUDIO_TEST")[0] != '2' || !((n++ / 400) & 1);
                    if (tone) {   /* "2" leaves the game's own samples in every other ~2 s window, for an A/B */
                        int16_t *pcm = qb->mAudioData;
                        for (uint32_t i = 0; i < frames; i++) {
                            int v = (int)(8000.0 * sin(phase));
                            phase += 2.0 * M_PI * 440.0 / (double)o->rate;
                            if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;
                            pcm[2 * i] = pcm[2 * i + 1] = (int16_t)v;
                        }
                    }
                }
                if (getenv("NFS_AUDIO_TRACE")) {   /* what actually reaches the device: the downmixed stereo */
                    const int16_t *s = qb->mAudioData;
                    unsigned peak = 0; uint64_t nz = 0, sum = 0, n = (uint64_t)frames * 2;
                    for (uint64_t i = 0; i < n; i++) { int v = s[i]; unsigned a = v < 0 ? (unsigned)-v : (unsigned)v; if (v) nz++; sum += a; if (a > peak) peak = a; }
                    pthread_mutex_lock(&o->lock);
                    o->nz_samples += nz; o->sum_abs += sum; o->total_samples += n;
                    if (peak > o->peak_sample) o->peak_sample = peak;
                    pthread_mutex_unlock(&o->lock);
                }
                qb->mAudioDataByteSize = (UInt32)(frames * 4);
                if (getenv("NFS_AUDIO_DUMP")) {   /* the exact stereo bytes handed to Core Audio, for offline listening */
                    static FILE *dump; if (!dump) dump = fopen(getenv("NFS_AUDIO_DUMP"), "wb");
                    if (dump) fwrite(qb->mAudioData, 1, qb->mAudioDataByteSize, dump);
                }
                queued = AudioQueueEnqueueBuffer(o->aq, qb, 0, NULL) == 0;
            }
        }
        pthread_mutex_unlock(&o->aq_lock);
        if (!queued) {                             /* the queue went away mid-chunk: give the room back and drop it */
            pthread_mutex_lock(&o->lock);
            o->device_frames -= frames; o->device_buffers--;
            pthread_mutex_unlock(&o->lock);
        }
    }
}

/* NFS_AUDIO=0: the ring is drained at each chunk's real duration, so the guest is paced as if playing. */
static void *silent_playback(void *arg) {
    AudioOut *o = arg; uint64_t due = 0;
    for (;;) {
        struct timespec ts = {0, 1000000}; nanosleep(&ts, NULL);
        pthread_mutex_lock(&o->lock);
        if (!o->running) { pthread_mutex_unlock(&o->lock); return NULL; }
        uint64_t now = now_ns();
        if (o->state == STATE_STARTED && o->chunk_count) {
            uint32_t frames = o->chunks[o->chunk_head];
            if (!due) due = now + (uint64_t)frames * 1000000000ull / o->rate;
            if (now >= due) {
                o->chunk_head = (o->chunk_head + 1) % CHUNK_MAX; o->chunk_count--;
                o->ring_head += frames; o->played_samples += frames; due = 0;
                pthread_cond_signal(&o->wake);
                pthread_mutex_unlock(&o->lock);
                continue;
            }
        } else due = 0;
        pthread_mutex_unlock(&o->lock);
    }
}
/* Reads the system default output device's UID, for the open/rebuild logs and the device trace: the queue is
   created on Core Audio's AQDefaultDevice pseudo-device and follows the system default output by itself. */
static CFStringRef default_output_uid(AudioDeviceID *id_out) {
    AudioDeviceID dev = 0; UInt32 size = sizeof dev;
    AudioObjectPropertyAddress a = {kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, &dev) != noErr) dev = 0;
    if (id_out) *id_out = dev;
    if (!dev) return NULL;
    CFStringRef uid = NULL; size = sizeof uid;
    AudioObjectPropertyAddress u = {kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
    if (AudioObjectGetPropertyData(dev, &u, 0, NULL, &size, &uid) != noErr) return NULL;
    return uid;
}
static const char *uid_text(CFStringRef uid, char *buf, size_t n) {
    buf[0] = 0;
    if (uid) CFStringGetCString(uid, buf, (CFIndex)n, kCFStringEncodingUTF8);
    return buf;
}
static CFStringRef queue_uid(AudioQueueRef aq) {
    CFStringRef uid = NULL; UInt32 size = sizeof uid;
    if (AudioQueueGetProperty(aq, kAudioQueueProperty_CurrentDevice, &uid, &size) != noErr) return NULL;
    return uid;
}

/* A queue the monitor cannot see fail: Core Audio leaves IsRunning true while nothing plays. Drop the
   old queue and build a fresh one on the current default device. The ring keeps the audio, so only the
   cushion the old queue was holding is lost. */
/* Bind the queue to the system default output's device UID. A queue created on Core Audio's AQDefaultDevice pseudo-device
   can report IsRunning with samples advancing and still be heard nowhere (S2.21 measured this), so the device is named
   explicitly and renamed whenever the default changes. Returns 0 when nothing had to change. */
static int bind_default_device(AudioOut *o) {
    AudioDeviceID dev = 0; CFStringRef want = default_output_uid(&dev);
    if (!want) return 1;   /* no default output right now: the monitor tries again */
    int changed = 0;
    pthread_mutex_lock(&o->aq_lock);
    if (o->aq) {
        CFStringRef have = queue_uid(o->aq);
        changed = !have || !CFEqual(have, want);
        if (changed) {
            char b[256];
            OSStatus s = AudioQueueSetProperty(o->aq, kAudioQueueProperty_CurrentDevice, &want, sizeof want);
            fprintf(stderr, "[totk-audio] AudioOut: bound to the system default output %u uid %s -> %d\n",
                    (unsigned)dev, uid_text(want, b, sizeof b), (int)s);
        }
        if (have) CFRelease(have);
    }
    pthread_mutex_unlock(&o->aq_lock);
    CFRelease(want);
    return changed;
}

static void rebuild_queue(AudioOut *o) {
    AudioStreamBasicDescription f = {.mSampleRate = o->rate, .mFormatID = kAudioFormatLinearPCM,
        .mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked, .mBytesPerPacket = 4, .mFramesPerPacket = 1,
        .mBytesPerFrame = 4, .mChannelsPerFrame = 2, .mBitsPerChannel = 16};
    AudioQueueRef old, fresh = NULL; uint64_t lost; int made;
    pthread_mutex_lock(&o->aq_lock);
    old = o->aq;
    made = AudioQueueNewOutput(&f, played, o, NULL, kCFRunLoopCommonModes, 0, &fresh) == 0;
    o->aq = made ? fresh : old;                   /* the old queue keeps serving if no new one can be made */
    pthread_mutex_lock(&o->lock);
    lost = o->device_frames;                      /* enqueued on the old queue: its dispose frees it unplayed */
    o->device_frames = 0; o->device_buffers = 0;
    pthread_mutex_unlock(&o->lock);
    pthread_mutex_unlock(&o->aq_lock);
    if (!made) { fprintf(stderr, "[totk-audio] AudioOut: the output is fed but playing nothing and a new queue could not be made; keeping it\n"); return; }
    {   /* the fresh queue is on the system default output and follows it */
        AudioDeviceID dev = 0; CFStringRef want = default_output_uid(&dev); char b[256];
        fprintf(stderr, "[totk-audio] AudioOut: the output is fed but playing nothing (interruption); rebuilt on the system default output (%u uid %s), %llu frame(s) of cushion dropped\n",
                (unsigned)dev, uid_text(want, b, sizeof b), (unsigned long long)lost);
        if (want) CFRelease(want);
    }
    bind_default_device(o);                       /* the fresh queue is bound to the default output, not the pseudo-device */
    if (old) AudioQueueDispose(old, true);        /* no lock held: an in-flight callback sees o->aq changed and frees nothing */
    AudioQueueStart(fresh, NULL);
    pthread_mutex_lock(&o->lock); pthread_cond_broadcast(&o->wake); pthread_mutex_unlock(&o->lock);
}

/* Core Audio stops the queue when its output device goes away — a device change, an interruption,
   another app taking the device. The guest cannot see it and keeps appending, so left alone the
   session is silent and then stalls on a full queue. Restart it on the current default device. */
static void *device_monitor(void *arg) {
    AudioOut *o = arg; int stopped = 0, trace = getenv("NFS_AUDIO_TRACE") != NULL; unsigned ticks = 0, stall = 0; uint64_t last_played = 0;
    for (;;) {
        struct timespec ts = {0, MONITOR_PERIOD_MS * 1000000};
        nanosleep(&ts, NULL);
        pthread_mutex_lock(&o->lock);
        int alive = o->running, started = o->state == STATE_STARTED;
        unsigned queued = o->chunk_count + o->device_buffers; uint64_t played = o->played_samples;
        unsigned long long ring_ms = (unsigned long long)((o->ring_tail - o->ring_head) * 1000ull / o->rate);
        unsigned long long dev_ms = (unsigned long long)(o->device_frames * 1000ull / o->rate);
        if (trace && ++ticks % 10 == 0) {                         /* every ~2 s: is the guest feeding real audio, and is it draining? */
            fprintf(stderr, "[nfs] audout trace: appends=%llu peak=%u played=%llu ring=%llums dev=%llums queued=%u vol=%g nonzero=%llu/%llu mean=%.0f\n",
                    (unsigned long long)o->appends, o->peak_sample, (unsigned long long)o->played_samples, ring_ms, dev_ms, queued, o->volume,
                    (unsigned long long)o->nz_samples, (unsigned long long)o->total_samples,
                    o->total_samples ? (double)o->sum_abs / (double)o->total_samples : 0.0);
            o->peak_sample = 0; o->nz_samples = 0; o->sum_abs = 0; o->total_samples = 0;
        }
        pthread_mutex_unlock(&o->lock);
        if (!alive) return NULL;
        bind_default_device(o);   /* a default-output change moves the queue with it */
        if (trace && ticks % 10 == 0) {   /* which device the system default is, whether it is running, and what the queue is on */
            AudioDeviceID d = 0; CFStringRef u = default_output_uid(&d); char b[256], q[256];
            AudioObjectPropertyAddress r = {kAudioDevicePropertyDeviceIsRunningSomewhere, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMain};
            UInt32 running = 0, sz = sizeof running;
            if (d) AudioObjectGetPropertyData(d, &r, 0, NULL, &sz, &running);
            pthread_mutex_lock(&o->aq_lock); CFStringRef qu = o->aq ? queue_uid(o->aq) : NULL; pthread_mutex_unlock(&o->aq_lock);
            fprintf(stderr, "[nfs] audout device: system default %u uid %s running=%u, queue uid %s\n",
                    (unsigned)d, uid_text(u, b, sizeof b), (unsigned)running, uid_text(qu, q, sizeof q));
            if (qu) CFRelease(qu);
            if (u) CFRelease(u);
        }
        if (!started) { stopped = 0; stall = 0; last_played = played; continue; }   /* the guest paused; a stop is expected */
        pthread_mutex_lock(&o->aq_lock);
        UInt32 running = 0, size = sizeof running;
        int playing = o->aq != NULL && AudioQueueGetProperty(o->aq, kAudioQueueProperty_IsRunning, &running, &size) == 0 && running != 0;
        pthread_mutex_unlock(&o->aq_lock);
        if (playing) {
            if (stopped) fprintf(stderr, "[totk-audio] AudioOut: the output is playing again on the current device\n");
            stopped = 0;
            if (queued && played == last_played) {                /* fed, started, playing nothing: the IsRunning flag cannot see this */
                if (++stall >= STALL_TICKS) { rebuild_queue(o); stall = 0; last_played = played; continue; }
            } else stall = 0;
        } else {
            pthread_mutex_lock(&o->aq_lock); OSStatus s = o->aq ? AudioQueueStart(o->aq, NULL) : -1; pthread_mutex_unlock(&o->aq_lock);   /* retried quietly while the device is away */
            if (!stopped) fprintf(stderr, "[totk-audio] AudioOut: the output queue had stopped (device change or interruption); restart -> %d\n", (int)s);
            stopped = 1;
        }
        last_played = played;
    }
}

static void open_device(AudioOut *o) {
    const char *e = getenv("NFS_AUDIO");
    o->silent = e && !strcmp(e, "0");
    o->ring_frames = (uint32_t)((uint64_t)o->rate * RING_MS / 1000);
    o->cushion_frames = (uint32_t)((uint64_t)o->rate * CUSHION_MS / 1000);
    o->ring = calloc((size_t)o->ring_frames * 2, sizeof(int16_t));
    o->staging = malloc((size_t)o->ring_frames * 4);
    need(o->ring && o->staging, "AudioOut ring");
    if (!o->silent) {
        AudioStreamBasicDescription f = {.mSampleRate = o->rate, .mFormatID = kAudioFormatLinearPCM,
            .mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked, .mBytesPerPacket = 4, .mFramesPerPacket = 1,
            .mBytesPerFrame = 4, .mChannelsPerFrame = 2, .mBitsPerChannel = 16};
        pthread_mutex_lock(&o->aq_lock);
        OSStatus s = AudioQueueNewOutput(&f, played, o, NULL, kCFRunLoopCommonModes, 0, &o->aq);
        if (s) { fprintf(stderr, "[totk-audio] AudioOut: no Core Audio output (%d); continuing silent\n", (int)s); o->silent = 1; }
        else {
            AudioDeviceID dev = 0; CFStringRef want = default_output_uid(&dev), have = queue_uid(o->aq);
            char a[256], b[256];
            fprintf(stderr, "[totk-audio] AudioOut: on the system default output (default %u uid %s; queue uid %s), following it\n",
                    (unsigned)dev, uid_text(want, a, sizeof a), uid_text(have, b, sizeof b));
            if (have) CFRelease(have);
            if (want) CFRelease(want);
        }
        pthread_mutex_unlock(&o->aq_lock);
        if (!o->silent) bind_default_device(o);   /* outside aq_lock: binding takes it */
    }
    need(pthread_create(&o->thread, NULL, o->silent ? silent_playback : device_monitor, o) == 0, "AudioOut playback thread");
    if (!o->silent) need(pthread_create(&o->feeder_thread, NULL, feeder, o) == 0, "AudioOut feeder thread");
}
/* Interleaved PCM16 of the client's layout -> stereo PCM16 (the queue's format). */
static void to_stereo(const int16_t *in, int16_t *out, uint64_t frames, uint32_t channels) {
    for (uint64_t i = 0; i < frames; i++) {
        const int16_t *s = in + i * channels;
        if (channels == 2) { out[2 * i] = s[0]; out[2 * i + 1] = s[1]; continue; }
        if (channels == 1) { out[2 * i] = out[2 * i + 1] = s[0]; continue; }
        float c = s[2] * 0.7071f, l = s[0] + c + s[4] * 0.7071f, r = s[1] + c + s[5] * 0.7071f;   /* ITU-style 5.1 downmix, LFE left out */
        out[2 * i] = (int16_t)(l > 32767 ? 32767 : l < -32768 ? -32768 : l); out[2 * i + 1] = (int16_t)(r > 32767 ? 32767 : r < -32768 ? -32768 : r);
    }
}

static const IpcBuffer *in_buffer(const IpcRequest *rq) { return rq->na && rq->a[0].size ? &rq->a[0] : rq->nx && rq->x[0].size ? &rq->x[0] : NULL; }
static const IpcBuffer *out_buffer(const IpcRequest *rq, int *pointer) {
    if (rq->nb && rq->b[0].size) { *pointer = 0; return &rq->b[0]; }
    if (rq->nc && rq->c[0].size) { *pointer = 1; return &rq->c[0]; }
    return NULL;
}
static void append(AudioOut *o, const IpcRequest *rq) {
    const IpcBuffer *b = in_buffer(rq); need(b && b->size >= sizeof(AudioOutBuffer), "AppendAudioOutBuffer descriptor");
    AudioOutBuffer d; memcpy(&d, ipc_guest(b->addr, sizeof d), sizeof d);
    uint64_t tag = IPC_ARG(rq, uint64_t, 0);
    /* The SDK copies all 0x28 bytes, but this title never initialises data_offset, so the
       field can hold allocator fill. An offset past the buffer can only be that. */
    uint64_t offset = d.data_offset <= d.capacity ? d.data_offset : 0;
    static int offset_noted;
    if (d.data_offset != offset && !offset_noted++)
        fprintf(stderr, "[nfs] audout: data_offset=0x%llx is not a valid offset into capacity=0x%llx; treated as 0\n",
                (unsigned long long)d.data_offset, (unsigned long long)d.capacity);
    static unsigned traced;
    if (getenv("NFS_AUDIO_TRACE") && traced++ < 4)
        fprintf(stderr, "[nfs] audout append cmd=%u tag=%llu in=0x%llx+0x%llx (na=%u nx=%u) "
                        "desc next=0x%llx sample=0x%llx capacity=0x%llx size=0x%llx offset=0x%llx (used 0x%llx)\n",
                rq->cmd, (unsigned long long)tag, (unsigned long long)b->addr, (unsigned long long)b->size,
                rq->na, rq->nx, (unsigned long long)d.next, (unsigned long long)d.sample_buffer,
                (unsigned long long)d.capacity, (unsigned long long)d.data_size, (unsigned long long)d.data_offset,
                (unsigned long long)offset);
    if (offset + d.data_size > d.capacity && d.capacity) {
        fprintf(stderr, "[nfs] AudioOut buffer bounds: offset=0x%llx + size=0x%llx > capacity=0x%llx (cmd=%u tag=%llu)\n",
                (unsigned long long)offset, (unsigned long long)d.data_size,
                (unsigned long long)d.capacity, rq->cmd, (unsigned long long)tag);
        runtime_fail("AudioOut buffer bounds");
    }
    uint64_t frames64 = d.data_size / (2ull * o->channels);
    need(frames64 <= 0xffffffffull, "AudioOut buffer size");
    uint32_t frames = (uint32_t)frames64;
    int16_t *mix = frames ? malloc((size_t)frames * 4) : NULL;
    need(!frames || mix, "AudioOut mix buffer");
    if (frames) to_stereo(ipc_guest(d.sample_buffer + offset, d.data_size), mix, frames, o->channels);

    pthread_mutex_lock(&o->lock);
    o->appends++;
    if (frames) {
        if (frames <= o->ring_frames && ring_room(o, frames)) {
            ring_write(o, mix, frames);
            o->chunks[(o->chunk_head + o->chunk_count) % CHUNK_MAX] = frames; o->chunk_count++;
            pthread_cond_signal(&o->wake);         /* the feeder has a chunk to send */
        } else {                                   /* the guest outran the ring for RING_BLOCK_MS: drop, do not wedge it */
            o->dropped_chunks++;
            static unsigned drops;
            if (drops++ < 4)
                fprintf(stderr, "[nfs] audout: no ring room for %d ms; dropped a %u-frame buffer (%llu dropped)\n",
                        RING_BLOCK_MS, frames, (unsigned long long)o->dropped_chunks);
        }
    }
    need(o->nreleased < RELEASED_MAX, "AudioOut released tags");
    o->released[o->nreleased++] = tag;             /* its samples are in the ring: the guest may reuse it */
    pthread_mutex_unlock(&o->lock);
    free(mix);
    runtime_event_signal(o->event);                /* the tag is back */
}
static void released(AudioOut *o, const IpcRequest *rq, IpcResponse *rs) {
    int pointer; const IpcBuffer *b = out_buffer(rq, &pointer); need(b != NULL, "GetReleasedAudioOutBuffers buffer");
    uint64_t *out = ipc_guest(b->addr, b->size); uint32_t n = 0;
    pthread_mutex_lock(&o->lock);
    while (n < o->nreleased && (n + 1) * 8 <= b->size) { out[n] = o->released[n]; n++; }
    memmove(o->released, o->released + n, (o->nreleased - n) * sizeof *o->released); o->nreleased -= n;
    pthread_mutex_unlock(&o->lock);
    if (n < b->size / 8) out[n] = 0;
    if (pointer) rs->xout[rs->nxout++] = (IpcBuffer){b->addr, (uint64_t)n * 8, 0};
    IPC_OUT(rs, n);
}
static void set_state(AudioOut *o, uint32_t state) {
    pthread_mutex_lock(&o->lock); o->state = state; pthread_cond_broadcast(&o->wake); pthread_mutex_unlock(&o->lock);
    pthread_mutex_lock(&o->aq_lock);
    if (o->aq) { if (state == STATE_STARTED) AudioQueueStart(o->aq, NULL); else AudioQueuePause(o->aq); }
    pthread_mutex_unlock(&o->aq_lock);
}
static void audio_out_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    AudioOut *o = s->state;
    switch (rq->cmd) {
    case 0: { uint32_t v = o->state; IPC_OUT(rs, v); return; }                                   /* GetAudioOutState */
    case 1: fprintf(stderr, "[nfs] audout: Start\n"); set_state(o, STATE_STARTED); return;                /* Start */
    case 2: fprintf(stderr, "[nfs] audout: Stop\n"); set_state(o, STATE_STOPPED); return;                  /* Stop */
    case 3: case 7: append(o, rq); return;                                                     /* AppendAudioOutBuffer(Auto) */
    case 4: rs->copy[rs->ncopy++] = runtime_event_readable(o->event); return;                   /* RegisterBufferEvent */
    case 5: case 8: released(o, rq, rs); return;                                               /* GetReleasedAudioOutBuffers(Auto) */
    case 6: {                                                                                   /* ContainsAudioOutBuffer(tag) */
        uint64_t tag = IPC_ARG(rq, uint64_t, 0); uint8_t found = 0;
        pthread_mutex_lock(&o->lock);
        for (unsigned i = 0; i < o->nreleased; i++) found |= o->released[i] == tag;
        pthread_mutex_unlock(&o->lock); IPC_OUT(rs, found); return;
    }
    case 9: { pthread_mutex_lock(&o->lock); uint32_t v = o->chunk_count + o->device_buffers; pthread_mutex_unlock(&o->lock); IPC_OUT(rs, v); return; }   /* GetAudioOutBufferCount */
    case 10: { pthread_mutex_lock(&o->lock); uint64_t v = o->played_samples; pthread_mutex_unlock(&o->lock); IPC_OUT(rs, v); return; }   /* GetAudioOutPlayedSampleCount */
    case 11: {                                                                                  /* FlushAudioOutBuffers */
        pthread_mutex_lock(&o->lock);
        uint8_t any = o->chunk_count || o->device_buffers; unsigned n = o->chunk_count + o->device_buffers;
        o->ring_head = o->ring_tail; o->chunk_count = 0; o->chunk_head = 0;
        pthread_cond_broadcast(&o->wake);
        pthread_mutex_unlock(&o->lock);
        fprintf(stderr, "[nfs] audout: Flush (%u queued)\n", n);
        pthread_mutex_lock(&o->aq_lock); if (o->aq) AudioQueueReset(o->aq);   /* its callbacks then find nothing left to play */
        pthread_mutex_lock(&o->lock); o->device_frames = 0; o->device_buffers = 0; pthread_mutex_unlock(&o->lock);
        pthread_mutex_unlock(&o->aq_lock);
        IPC_OUT(rs, any); return;
    }
    case 12: o->volume = IPC_ARG(rq, float, 0); if (o->aq) AudioQueueSetParameter(o->aq, kAudioQueueParam_Volume, o->volume);
        { static int noted; if (!noted++) fprintf(stderr, "[nfs] audout volume -> %g\n", o->volume); } return;   /* SetAudioOutVolume */
    case 13: { float v = o->volume; IPC_OUT(rs, v); return; }                                   /* GetAudioOutVolume */
    default: ipc_unimplemented(s, rq);
    }
}
static void audio_out_close(void *p) {
    AudioOut *o = p;
    fprintf(stderr, "[nfs] audout: Close\n");
    pthread_mutex_lock(&o->lock); o->running = 0; pthread_cond_broadcast(&o->wake); pthread_mutex_unlock(&o->lock);
    pthread_join(o->thread, NULL);                 /* the monitor must be gone before the queue is disposed */
    if (!o->silent) pthread_join(o->feeder_thread, NULL);
    pthread_mutex_lock(&o->aq_lock); AudioQueueRef aq = o->aq; o->aq = NULL; pthread_mutex_unlock(&o->aq_lock);
    if (aq) AudioQueueDispose(aq, true);
    pthread_mutex_destroy(&o->lock); pthread_mutex_destroy(&o->aq_lock); pthread_cond_destroy(&o->wake);
    free(o->ring); free(o->staging); free(o);
}

static const char device_name[] = "DeviceOut";
static void write_name(const IpcRequest *rq, IpcResponse *rs) {
    int pointer; const IpcBuffer *b = out_buffer(rq, &pointer);
    if (!b) return;
    char *out = ipc_guest(b->addr, b->size); size_t n = b->size < sizeof device_name ? b->size : sizeof device_name;
    memcpy(out, device_name, n); if (n) out[n - 1] = 0;
    if (pointer) rs->xout[rs->nxout++] = (IpcBuffer){b->addr, n, 0};
}
static void manager_handler(IpcSession *s, const IpcRequest *rq, IpcResponse *rs) {
    switch (rq->cmd) {
    case 0: case 2: { write_name(rq, rs); uint32_t n = 1; IPC_OUT(rs, n); return; }            /* ListAudioOuts(Auto) */
    case 1: case 3: {                                                                           /* OpenAudioOut(Auto) */
        uint32_t rate = IPC_ARG(rq, uint32_t, 0); uint16_t channels = IPC_ARG(rq, uint16_t, 4);
        if (!rate) rate = DEFAULT_RATE;
        if (!channels) channels = 2;
        if (rate != DEFAULT_RATE || (channels != 1 && channels != 2 && channels != 6)) {
            fprintf(stderr, "[totk-audio] OpenAudioOut %u Hz x%u\n", rate, channels); runtime_fail("AudioOut format not supported");
        }
        AudioOut *o = calloc(1, sizeof *o); need(o != NULL, "AudioOut allocation");
        pthread_mutex_init(&o->lock, NULL); pthread_mutex_init(&o->aq_lock, NULL); pthread_cond_init(&o->wake, NULL); o->rate = rate; o->channels = channels; o->state = STATE_STOPPED; o->volume = 1.0f;
        o->event = runtime_event_new(); o->running = 1;
        open_device(o);
        write_name(rq, rs);
        uint32_t out[4] = {rate, channels, FORMAT_PCM16, STATE_STOPPED}; ipc_out(rs, out, sizeof out);
        rs->move[rs->nmove++] = runtime_ipc_new_session("IAudioOut", audio_out_handler, o, audio_out_close);
        fprintf(stderr, "[totk-audio] AudioOut open %u Hz x%u PCM16 -> %s\n", rate, channels, o->silent ? "silent sink" : "Core Audio (stereo)");
        return;
    }
    default: ipc_unimplemented(s, rq);
    }
}

__attribute__((constructor)) static void register_audio_out(void) { runtime_ipc_register("audout:u", manager_handler, NULL, NULL); }
