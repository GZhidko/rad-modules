#ifndef OPENRADIUS_STATS_C_H
#define OPENRADIUS_STATS_C_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

typedef struct stats_c {
    const char *module;
    const char *file;
    int interval_sec;
    time_t last;
    uint64_t count;
    uint64_t total_ms;
    uint64_t min_ms;
    uint64_t max_ms;
} stats_c_t;

static inline uint64_t stats_c_now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000ULL) + ((uint64_t)tv.tv_usec / 1000ULL);
}

static inline void stats_c_init(stats_c_t *s, const char *module, const char *file, int interval_min) {
    s->module = module;
    s->file = file;
    s->interval_sec = (interval_min > 0 ? interval_min * 60 : 300);
    s->last = time(NULL);
    s->count = 0;
    s->total_ms = 0;
    s->min_ms = 0;
    s->max_ms = 0;
}

static inline void stats_c_flush(stats_c_t *s, time_t now) {
    if (!s->file || s->count == 0) {
        s->last = now;
        return;
    }
    FILE *f = fopen(s->file, "a");
    if (f) {
        double avg = (double)s->total_ms / (double)s->count;
        fprintf(f,
                "{\"ts\":%ld,\"module\":\"%s\",\"pid\":%ld,\"interval_sec\":%d,\"count\":%llu,\"total_ms\":%llu,\"min_ms\":%llu,\"max_ms\":%llu,\"avg_ms\":%.3f}\n",
                (long)now,
                s->module ? s->module : "",
                (long)getpid(),
                s->interval_sec,
                (unsigned long long)s->count,
                (unsigned long long)s->total_ms,
                (unsigned long long)s->min_ms,
                (unsigned long long)s->max_ms,
                avg);
        fclose(f);
    }
    s->count = 0;
    s->total_ms = 0;
    s->min_ms = 0;
    s->max_ms = 0;
    s->last = now;
}

static inline void stats_c_record(stats_c_t *s, uint64_t ms) {
    if (!s->file) return;
    s->count++;
    s->total_ms += ms;
    if (s->min_ms == 0 || ms < s->min_ms) s->min_ms = ms;
    if (ms > s->max_ms) s->max_ms = ms;
    time_t now = time(NULL);
    if ((now - s->last) >= s->interval_sec) {
        stats_c_flush(s, now);
    }
}

#endif
