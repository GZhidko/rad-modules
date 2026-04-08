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
    double mean_ms;
    double m2_ms;
    int outlier_active;
    uint64_t outlier_count;
} stats_c_t;

#define STATS_C_OUTLIER_MIN_COUNT 100

static inline uint64_t stats_c_now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000ULL) + ((uint64_t)tv.tv_usec / 1000ULL);
}

static inline double stats_c_sqrt(double x) {
    double guess;
    int i;

    if (x <= 0.0) return 0.0;
    guess = x > 1.0 ? x : 1.0;
    for (i = 0; i < 16; i++) guess = 0.5 * (guess + x / guess);
    return guess;
}

static inline void stats_c_format_time(char *buf, size_t buflen, time_t now) {
    struct tm *tm_info = localtime(&now);
    if (tm_info) {
        strftime(buf, buflen, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        snprintf(buf, buflen, "%ld", (long)now);
    }
}

static inline void stats_c_write_json_string(FILE *f, const char *s) {
    const unsigned char *p = (const unsigned char *)(s ? s : "");
    fputc('"', f);
    while (*p) {
        switch (*p) {
        case '\\':
        case '"':
            fputc('\\', f);
            fputc(*p, f);
            break;
        case '\n':
            fputs("\\n", f);
            break;
        case '\r':
            fputs("\\r", f);
            break;
        case '\t':
            fputs("\\t", f);
            break;
        default:
            if (*p < 0x20) {
                fprintf(f, "\\u%04x", (unsigned int)*p);
            } else {
                fputc(*p, f);
            }
        }
        p++;
    }
    fputc('"', f);
}

static inline void stats_c_log_outlier(stats_c_t *s, time_t now, uint64_t ms,
                                       double mean, double sigma,
                                       const char *detail) {
    FILE *f;
    char ts_buf[32];

    if (!s->file) return;
    f = fopen(s->file, "a");
    if (!f) return;

    stats_c_format_time(ts_buf, sizeof(ts_buf), now);
    fprintf(f,
            "{\"time\":\"%s\",\"module\":\"%s\",\"pid\":%ld,\"kind\":\"outlier\",\"ms\":%llu,\"mean_ms\":%.3f,\"sigma_ms\":%.3f,\"threshold_ms\":%.3f,\"detail\":",
            ts_buf,
            s->module ? s->module : "",
            (long)getpid(),
            (unsigned long long)ms,
            mean,
            sigma,
            mean + 3.0 * sigma);
    stats_c_write_json_string(f, detail);
    fputs("}\n", f);
    fclose(f);
}

static inline void stats_c_record_ex(stats_c_t *s, uint64_t ms,
                                     const char *detail);

static inline void stats_c_init(stats_c_t *s, const char *module, const char *file, int interval_min) {
    s->module = module;
    s->file = file;
    s->interval_sec = (interval_min > 0 ? interval_min * 60 : 300);
    s->last = time(NULL);
    s->count = 0;
    s->total_ms = 0;
    s->min_ms = 0;
    s->max_ms = 0;
    s->mean_ms = 0.0;
    s->m2_ms = 0.0;
    s->outlier_active = 0;
    s->outlier_count = 0;
}

static inline void stats_c_flush(stats_c_t *s, time_t now) {
    if (!s->file || s->count == 0) {
        s->last = now;
        return;
    }
    FILE *f = fopen(s->file, "a");
    if (f) {
        double avg = (double)s->total_ms / (double)s->count;
        double outlier_pct = s->count ? (100.0 * (double)s->outlier_count / (double)s->count) : 0.0;
        char ts_buf[32];
        stats_c_format_time(ts_buf, sizeof(ts_buf), now);
        fprintf(f,
                "{\"time\":\"%s\",\"module\":\"%s\",\"pid\":%ld,\"interval_sec\":%d,\"count\":%llu,\"total_ms\":%llu,\"min_ms\":%llu,\"max_ms\":%llu,\"avg_ms\":%.3f,\"outlier_count\":%llu,\"outlier_pct\":%.3f}\n",
                ts_buf,
                s->module ? s->module : "",
                (long)getpid(),
                s->interval_sec,
                (unsigned long long)s->count,
                (unsigned long long)s->total_ms,
                (unsigned long long)s->min_ms,
                (unsigned long long)s->max_ms,
                avg,
                (unsigned long long)s->outlier_count,
                outlier_pct);
        fclose(f);
    }
    s->count = 0;
    s->total_ms = 0;
    s->min_ms = 0;
    s->max_ms = 0;
    s->mean_ms = 0.0;
    s->m2_ms = 0.0;
    s->outlier_active = 0;
    s->outlier_count = 0;
    s->last = now;
}

static inline void stats_c_record(stats_c_t *s, uint64_t ms) {
    stats_c_record_ex(s, ms, NULL);
}

static inline void stats_c_record_ex(stats_c_t *s, uint64_t ms,
                                     const char *detail) {
    double delta;
    double mean_before;
    double sigma_before;
    time_t now;

    if (!s->file) return;

    now = time(NULL);
    if (s->count >= STATS_C_OUTLIER_MIN_COUNT) {
        double threshold;
        mean_before = s->mean_ms;
        sigma_before = stats_c_sqrt(s->m2_ms / (double)(s->count - 1));
        threshold = mean_before + 3.0 * sigma_before;
        if ((double)ms > threshold) {
            s->outlier_count++;
            if (!s->outlier_active) {
                stats_c_log_outlier(s, now, ms, mean_before, sigma_before, detail);
                s->outlier_active = 1;
            }
        } else {
            s->outlier_active = 0;
        }
    } else {
        s->outlier_active = 0;
    }

    s->count++;
    s->total_ms += ms;
    if (s->min_ms == 0 || ms < s->min_ms) s->min_ms = ms;
    if (ms > s->max_ms) s->max_ms = ms;
    delta = (double)ms - s->mean_ms;
    s->mean_ms += delta / (double)s->count;
    s->m2_ms += delta * ((double)ms - s->mean_ms);
    if ((now - s->last) >= s->interval_sec) {
        stats_c_flush(s, now);
    }
}

#endif
