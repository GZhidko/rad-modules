#!/usr/local/bin/perl

$|=1;

$s = '';
while (<>) {
    $s .= $_;
    $i = index($s, "\n\n");
    next if ($i < 0);
    $i += 2;
    $_ = "Child-PID = " . $$ . "\n" . "Child-Time = \"" . scalar(localtime()) . "\"\n" . substr($s, 0, $i);
    $s = substr($s, $i);
    s-OUT-=CHILD=-g;
#    if (rand() < .1) {
        s-(Identifier.+)-${1}9-;
#    }
#    s-(Identifier.+)-II=9-;
    print;
#    print STDERR "\033[1mERROR MESSAGE: PID=$$\033[0m\n";
}
