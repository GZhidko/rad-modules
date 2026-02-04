#!/usr/local/bin/perl

foreach (<../src/*.h>) {
    $l = $_;
    $l =~ s-.*/--;
    $l =~ s-\..*--;
    $l = 'MRADCLIENT_' . uc($l) . '_H';
    $f = $_;
    print "file $f ... ($l)\n";
    open FH, "<$f" or die;
    $_ = join('', <FH>);
    close FH;
    unless (/$l/) {
        open FH, ">$f" or die;
        print FH "#ifndef $l\n#define $l\n\n$_\n#endif // $l\n";
        close FH;
    }
}
