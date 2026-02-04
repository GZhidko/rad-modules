# Common stats helpers for OpenRADIUS modules (Perl).
# Usage: stats_init($module, $file, $interval_minutes);
# Then call stats_record($elapsed_ms) per request.

use strict;
use Time::HiRes qw(time);

our ($STAT_MODULE, $STAT_FILE, $STAT_INTERVAL_SEC);
our ($STAT_COUNT, $STAT_TOTAL, $STAT_MIN, $STAT_MAX, $STAT_LAST);

sub stats_init {
    my ($module, $file, $interval_minutes) = @_;
    $STAT_MODULE = $module;
    $STAT_FILE = $file;
    $STAT_INTERVAL_SEC = ($interval_minutes && $interval_minutes > 0) ? $interval_minutes * 60 : 300;
    $STAT_COUNT = 0;
    $STAT_TOTAL = 0;
    $STAT_MIN = 0;
    $STAT_MAX = 0;
    $STAT_LAST = time();
}

sub _stats_flush {
    my ($now) = @_;
    return if (!$STAT_FILE);
    if ($STAT_COUNT == 0) {
        $STAT_LAST = $now;
        return;
    }
    my $avg = $STAT_TOTAL / $STAT_COUNT;
    if (open(my $fh, '>>', $STAT_FILE)) {
        printf $fh '{"ts":%d,"module":"%s","pid":%d,"interval_sec":%d,"count":%d,"total_ms":%d,"min_ms":%d,"max_ms":%d,"avg_ms":%.3f}\n',
            int($now), $STAT_MODULE, $$, $STAT_INTERVAL_SEC,
            $STAT_COUNT, $STAT_TOTAL, $STAT_MIN, $STAT_MAX, $avg;
        close($fh);
    }
    $STAT_COUNT = 0;
    $STAT_TOTAL = 0;
    $STAT_MIN = 0;
    $STAT_MAX = 0;
    $STAT_LAST = $now;
}

sub stats_record {
    my ($ms) = @_;
    return if (!$STAT_FILE);
    $STAT_COUNT++;
    $STAT_TOTAL += $ms;
    if ($STAT_MIN == 0 || $ms < $STAT_MIN) { $STAT_MIN = $ms; }
    if ($ms > $STAT_MAX) { $STAT_MAX = $ms; }
    my $now = time();
    if (($now - $STAT_LAST) >= $STAT_INTERVAL_SEC) {
        _stats_flush($now);
    }
}

1;
