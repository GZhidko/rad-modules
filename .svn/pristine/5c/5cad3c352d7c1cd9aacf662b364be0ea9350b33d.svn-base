#! /bin/sh

if test "a$1" = "a-build"
then
  # if you do not want to install net-snmp library
  # to your base system, you cat use those variables
  # to customize compilling in linking process:
  # export PATH="$PREFIX/bin:$PATH"
  # export C_INCLUDE_PATH="$PREFIX/include"
  # export LD_LIBRARY_PATH="$PREFIX/lib"
  app=snmpgw
  gcc `net-snmp-config --cflags` -c -o $app.o $app.c
  gcc -o $app $app.o `net-snmp-config --libs`
else
  echo "*** NOTE:"
  echo "*** You can build this software without Autoconf."
  echo "*** Run"
  echo "*** sh ./autogen.sh -build"
  echo "*** to do this."
  aclocal  &&
  autoconf &&
  automake --add-missing
fi
