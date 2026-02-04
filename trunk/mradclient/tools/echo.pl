#!/usr/local/bin/perl

$|=1;

$n=0;
$s = '';
while (<>) {
    $s .= $_;
    $i = index($s, "\n\n");
    next if ($i < 0);
    $i += 2;
    $_ = "Child-PID = " . $$ . "\n" . "Child-Time = \"" . scalar(localtime()) . "\"\n" . substr($s, 0, $i);
    $s = substr($s, $i);
    s-OUT-=CHILD=-g;
#    print "asdf\n\n";
    print;
    print STDERR "ERROR MESSAGE: PID=$$ (n=$n)\n";
    $n++;
}
