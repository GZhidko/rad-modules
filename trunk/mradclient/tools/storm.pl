#!/usr/local/bin/perl

use Time::HiRes qw(usleep nanosleep);

$| = 1;

foreach (0..1_000_000_000) {
   usleep(10000); # microseconds
   $x = int(rand(1)*256);
#   $x = 100;
   $c = chr(65+26*rand());
   print <<"DATA";
RAD-Identifier = $x
RAD-Authenticator = "$c$c$c$c$c$c$c$c$c$c$c$c$c$c$c$c...$_"
T = "$c$c$c"
Req: $_

DATA
}
sleep(1);
