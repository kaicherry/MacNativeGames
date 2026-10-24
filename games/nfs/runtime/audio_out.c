/* audout:u — nn::audio AudioOut, which Need for Speed's Wwise uses for its final mix (TOTK only uses the audio renderer,
   audren:u). Commands and layouts follow the public switchbrew description ("Audio services": IAudioOutManager, IAudioOut).
   Each AudioOut is a Core Audio AudioQueue: an appended buffer's samples are copied and enqueued; when the device has
   played it, the queue's callback gives the buffer back (GetReleasedAudioOutBuffers) and signals the buffer event, so
   the game is paced by the real output. 5.1 output is mixed down to stereo (Switch order FL FR FC LFE BL BR).
   NFS_AUDIO=0 keeps the silent real-time sink (each buffer held for its duration) for tests without a sound device. */
#include "runtime.h"
#include "ipc.h"
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { STATE_STARTED = 0, STATE_STOPPED = 1, FORMAT_PCM16 = 2, DEFAULT_RATE = 48000, QUEUE_MAX = 32 };
/* MONITOR_PERIOD_MS is the longest the sound may stay dead after the output device goes away. */
enum { MONITOR_PERIOD_MS = 200 };
/* STALL_TICKS monitor periods a started queue holding data may play nothing before it is rebuilt; the
   recorded trace advances ~9600 frames a period and a queue that has lost its device advances none. */
enum { STALL_TICKS = 3 };
typedef struct { uint64_t next, sample_buffer, capacity, data_size, data_offset; } AudioOutBuffer;   /* the client's descriptor */
_Static_assert(sizeof(AudioOutBuffer) == 0x28, "AudioOutBuffer");
typedef struct { uint64_t tag, bytes; } Queued;
typedef struct {
    pthread_mutex_t lock; pthread_mutex_t aq_lock; pthread_t thread; int running, silent;
    uint32_t rate, channels, state, event; float volume;
    AudioQueueRef aq;
    Queued queue[QUEUE_MAX]; unsigned nqueue;          /* appended, not yet played (head first) */
    uint64_t released[QUEUE_MAX]; unsigned nreleased;  /* played, not yet collected by GetReleasedAudioOutBuffers */
    uint64_t played_samples;
    uint64_t appends;                                  /* NFS_AUDIO_TRACE: appended buffers, and the loudest sample in them */
    unsigned peak_sample;
    uint64_t nz_samples, sum_abs, total_samples;       /* NFS_AUDIO_TRACE: is the appended audio continuous or sparse spikes? */
} AudioOut;

static uint64_t now_ns(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec; }
static void need(int ok, const char *what) { if (!ok) runtime_fail(what); }

/* The head of the queue has been played: hand its tag back. Called with the lock held. */
static void release_head(AudioOut *o) {
    if (!o->nqueue) return;
    const Queued head = o->queue[0];
    memmove(o->queue, o->queue + 1, (--o->nqueue) * sizeof *o->queue);
    if (o->nreleased < QUEUE_MAX) o->released[o->nreleased++] = head.tag;
    o->played_samples += head.bytes / (2ull * o->channels);
}
static void played(void *user, AudioQueueRef q, AudioQueueBufferRef b) {   /* Core Audio thread */
    AudioOut *o = user;
    pthread_mutex_lock(&o->aq_lock);
    if (q != o->aq) { pthread_mutex_unlock(&o->aq_lock); return; }   /* a rebuild replaced it: the dispose frees the buffer */
    pthread_mutex_lock(&o->lock); release_head(o); pthread_mutex_unlock(&o->lock);
    AudioQueueFreeBuffer(q, b);
    pthread_mutex_unlock(&o->aq_lock);
    runtime_event_signal(o->event);
}
static void *silent_playback(void *arg) {
    AudioOut *o = arg; uint64_t due = 0;
    for (;;) {
        pthread_mutex_lock(&o->lock);
        if (!o->running) { pthread_mutex_unlock(&o->lock); return NULL; }
        uint64_t now = now_ns();
        if (o->state == STATE_STARTED && o->nqueue) {
            if (!due) due = now + o->queue[0].bytes / (2ull * o->channels) * 1000000000ull / o->rate;
            if (now >= due) { release_head(o); due = 0; pthread_mutex_unlock(&o->lock); runtime_event_signal(o->event); continue; }
        } else due = 0;
        pthread_mutex_unlock(&o->lock);
        struct timespec ts = {0, 1000000}; nanosleep(&ts, NULL);
    }
}
/* Reads the system default output device's UID, for diagnostics only: the queue is created on Core
   Audio's AQDefaultDevice pseudo-device and follows the system default output by itself. */
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
   old queue, build a fresh one on the current default device, and hand the tags it will never play
   back to the guest as released, or it waits on them forever. */
static void rebuild_queue(AudioOut *o) {
    AudioStreamBasicDescription f = {.mSampleRate = o->rate, .mFormatID = kAudioFormatLinearPCM,
        .mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked, .mBytesPerPacket = 4, .mFramesPerPacket = 1,
        .mBytesPerFrame = 4, .mChannelsPerFrame = 2, .mBitsPerChannel = 16};
    AudioQueueRef old, fresh = NULL; unsigned lost; int made;
    pthread_mutex_lock(&o->aq_lock);
    pthread_mutex_lock(&o->lock);
    lost = o->nqueue;
    while (o->nqueue) release_head(o);            /* the old queue is gone: report the tags it will never play */
    pthread_mutex_unlock(&o->lock);
    old = o->aq;
    made = AudioQueueNewOutput(&f, played, o, NULL, kCFRunLoopCommonModes, 0, &fresh) == 0;
    o->aq = made ? fresh : old;                   /* the old queue keeps serving if no new one can be made */
    pthread_mutex_unlock(&o->aq_lock);
    if (!made) { fprintf(stderr, "[totk-audio] AudioOut: the output is fed but playing nothing and a new queue could not be made; keeping it\n"); return; }
    {   /* the fresh queue is on the system default output and follows it */
        AudioDeviceID dev = 0; CFStringRef want = default_output_uid(&dev); char b[256];
        fprintf(stderr, "[totk-audio] AudioOut: the output is fed but playing nothing (interruption); rebuilt on the system default output (%u uid %s), %u tag(s) returned\n",
                (unsigned)dev, uid_text(want, b, sizeof b), lost);
        if (want) CFRelease(want);
    }
    if (old) AudioQueueDispose(old, true);        /* no lock held: an in-flight callback sees o->aq changed and frees nothing */
    AudioQueueStart(fresh, NULL);
    runtime_event_signal(o->event);               /* wake the guest for the returned tags */
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
        unsigned queued = o->nqueue; uint64_t played = o->played_samples;
        if (trace && ++ticks % 10 == 0) {                         /* every ~2 s: is the guest feeding real audio, and is it draining? */
            fprintf(stderr, "[nfs] audout trace: appends=%llu peak=%u played=%llu queued=%u vol=%g nonzero=%llu/%llu mean=%.0f\n",
                    (unsigned long long)o->appends, o->peak_sample, (unsigned long long)o->played_samples, o->nqueue, o->volume,
                    (unsigned long long)o->nz_samples, (unsigned long long)o->total_samples,
                    o->total_samples ? (double)o->sum_abs / (double)o->total_samples : 0.0);
            o->peak_sample = 0; o->nz_samples = 0; o->sum_abs = 0; o->total_samples = 0;
        }
        pthread_mutex_unlock(&o->lock);
        if (!alive) return NULL;
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
    }
    need(pthread_create(&o->thread, NULL, o->silent ? silent_playback : device_monitor, o) == 0, "AudioOut playback thread");
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
    pthread_mutex_lock(&o->aq_lock);              /* the monitor rebuilds the queue under this lock */
    pthread_mutex_lock(&o->lock);
    need(o->nqueue < QUEUE_MAX, "AudioOut queue full");
    o->queue[o->nqueue++] = (Queued){tag, d.data_size};
    o->appends++;
    pthread_mutex_unlock(&o->lock);
    if (o->silent) { pthread_mutex_unlock(&o->aq_lock); return; }
    uint64_t frames = d.data_size / (2ull * o->channels);
    AudioQueueBufferRef qb;
    need(AudioQueueAllocateBuffer(o->aq, (UInt32)(frames * 4 ? frames * 4 : 4), &qb) == 0, "AudioOut queue buffer");
    to_stereo(ipc_guest(d.sample_buffer + offset, d.data_size), qb->mAudioData, frames, o->channels);
    if (getenv("NFS_AUDIO_TEST") && frames) {   /* a steady tone through this same queue: is the process audible at all? */
        static double phase; static unsigned n; int tone = getenv("NFS_AUDIO_TEST")[0] != '2' || !((n++ / 400) & 1);
        if (tone) {   /* "2" leaves the game's own samples in every other ~2 s window, for an A/B */
            int16_t *pcm = qb->mAudioData;
            for (uint64_t i = 0; i < frames; i++) {
                int v = (int)(8000.0 * sin(phase));
                phase += 2.0 * M_PI * 440.0 / (double)o->rate;
                if (phase > 2.0 * M_PI) phase -= 2.0 * M_PI;
                pcm[2 * i] = pcm[2 * i + 1] = (int16_t)v;
            }
        }
    }
    if (getenv("NFS_AUDIO_TRACE") && frames) {   /* what actually reaches the device: the downmixed stereo */
        const int16_t *s = qb->mAudioData;
        unsigned peak = 0; uint64_t nz = 0, sum = 0, n = frames * 2;
        for (uint64_t i = 0; i < n; i++) { int v = s[i]; unsigned a = v < 0 ? (unsigned)-v : (unsigned)v; if (v) nz++; sum += a; if (a > peak) peak = a; }
        pthread_mutex_lock(&o->lock);
        o->nz_samples += nz; o->sum_abs += sum; o->total_samples += n;
        if (peak > o->peak_sample) o->peak_sample = peak;
        pthread_mutex_unlock(&o->lock);
    }
    qb->mAudioDataByteSize = (UInt32)(frames * 4);
    if (!frames) { memset(qb->mAudioData, 0, 4); qb->mAudioDataByteSize = 4; }   /* an empty buffer still has to come back */
    if (getenv("NFS_AUDIO_DUMP")) {   /* the exact stereo bytes handed to Core Audio, for offline listening */
        static FILE *dump; if (!dump) dump = fopen(getenv("NFS_AUDIO_DUMP"), "wb");
        if (dump) fwrite(qb->mAudioData, 1, qb->mAudioDataByteSize, dump);
    }
    need(AudioQueueEnqueueBuffer(o->aq, qb, 0, NULL) == 0, "AudioOut enqueue");
    pthread_mutex_unlock(&o->aq_lock);
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
    pthread_mutex_lock(&o->lock); o->state = state; pthread_mutex_unlock(&o->lock);
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
        for (unsigned i = 0; i < o->nqueue; i++) found |= o->queue[i].tag == tag;
        for (unsigned i = 0; i < o->nreleased; i++) found |= o->released[i] == tag;
        pthread_mutex_unlock(&o->lock); IPC_OUT(rs, found); return;
    }
    case 9: { pthread_mutex_lock(&o->lock); uint32_t v = o->nqueue; pthread_mutex_unlock(&o->lock); IPC_OUT(rs, v); return; }   /* GetAudioOutBufferCount */
    case 10: { pthread_mutex_lock(&o->lock); uint64_t v = o->played_samples; pthread_mutex_unlock(&o->lock); IPC_OUT(rs, v); return; }   /* GetAudioOutPlayedSampleCount */
    case 11: {                                                                                  /* FlushAudioOutBuffers */
        pthread_mutex_lock(&o->lock); uint8_t any = o->nqueue != 0; unsigned n = o->nqueue;
        while (o->nqueue) release_head(o);
        pthread_mutex_unlock(&o->lock);
        fprintf(stderr, "[nfs] audout: Flush (%u queued)\n", n);
        pthread_mutex_lock(&o->aq_lock); if (o->aq) AudioQueueReset(o->aq); pthread_mutex_unlock(&o->aq_lock);   /* its callbacks then find nothing left to release */
        if (any) runtime_event_signal(o->event);
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
    pthread_mutex_lock(&o->lock); o->running = 0; pthread_mutex_unlock(&o->lock);
    pthread_join(o->thread, NULL);                 /* the monitor must be gone before the queue is disposed */
    pthread_mutex_lock(&o->aq_lock); AudioQueueRef aq = o->aq; o->aq = NULL; pthread_mutex_unlock(&o->aq_lock);
    if (aq) AudioQueueDispose(aq, true);
    pthread_mutex_destroy(&o->lock); pthread_mutex_destroy(&o->aq_lock); free(o);
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
        pthread_mutex_init(&o->lock, NULL); pthread_mutex_init(&o->aq_lock, NULL); o->rate = rate; o->channels = channels; o->state = STATE_STOPPED; o->volume = 1.0f;
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
