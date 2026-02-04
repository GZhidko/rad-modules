#! /bin/sh
aclocal &&
autoconf &&
automake --add-missing &&
echo 'DONE.'

