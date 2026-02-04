#!/bin/sh
cat <<'DATA' |
str="10:56:36.375 UTC Wed Apr 6 2011"

str="invalid data"

DATA
./h323date -d
