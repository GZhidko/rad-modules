#! /bin/sh
set -xa
libtoolize &&
aclocal  &&
autoheader &&
automake --add-missing -i &&
autoconf &&
set +x
