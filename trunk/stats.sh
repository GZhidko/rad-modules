# Common stats helpers for OpenRADIUS modules (shell).
# Usage: stats_init "module" "/path/to/file" interval_minutes
# Then call stats_record <elapsed_ms> per request.

stats_init() {
    STAT_MODULE="$1"
    STAT_FILE="$2"
    STAT_INTERVAL_SEC=$(( ${3:-5} * 60 ))
    STAT_COUNT=0
    STAT_TOTAL=0
    STAT_MIN=0
    STAT_MAX=0
    STAT_LAST=$(date +%s)
}

stats_now_ms() {
    local t
    t=$(date +%s%3N 2>/dev/null)
    # Fallback if %3N unsupported
    if [ ${#t} -le 10 ]; then
        t=$((t * 1000))
    fi
    echo "$t"
}

stats_flush() {
    local now_s
    now_s="$1"
    if [ -z "$STAT_FILE" ] || [ $STAT_COUNT -eq 0 ]; then
        STAT_LAST="$now_s"
        return
    fi
    local avg
    avg=$(awk "BEGIN {printf \"%.3f\", $STAT_TOTAL / $STAT_COUNT}")
    printf '{"ts":%s,"module":"%s","pid":%s,"interval_sec":%s,"count":%s,"total_ms":%s,"min_ms":%s,"max_ms":%s,"avg_ms":%s}\n' \
        "$now_s" "$STAT_MODULE" "$$" "$STAT_INTERVAL_SEC" \
        "$STAT_COUNT" "$STAT_TOTAL" "$STAT_MIN" "$STAT_MAX" "$avg" >> "$STAT_FILE"
    STAT_COUNT=0
    STAT_TOTAL=0
    STAT_MIN=0
    STAT_MAX=0
    STAT_LAST="$now_s"
}

stats_record() {
    local ms now_s
    if [ -z "$STAT_FILE" ]; then
        return
    fi
    ms="$1"
    STAT_COUNT=$((STAT_COUNT + 1))
    STAT_TOTAL=$((STAT_TOTAL + ms))
    if [ $STAT_MIN -eq 0 ] || [ $ms -lt $STAT_MIN ]; then
        STAT_MIN=$ms
    fi
    if [ $ms -gt $STAT_MAX ]; then
        STAT_MAX=$ms
    fi
    now_s=$(date +%s)
    if [ $((now_s - STAT_LAST)) -ge $STAT_INTERVAL_SEC ]; then
        stats_flush "$now_s"
    fi
}
