#!/bin/sh

DB_SOURCE='test-users.txt'
GDBM_FILE='u.gdbm'
OUT_FILE='test-out'
ERR_FILE='test-err'

rm "$GDBM_FILE"
./gdbmfile-gen -s "$DB_SOURCE" -f "$GDBM_FILE" || exit 1

cat <<'TEXT' |
str="vasya"
str="quot"
str="pap-vasya"

str="123"

str="123"
str="quot"

TEXT
./gdbmfile -f "$GDBM_FILE" -d >"$OUT_FILE" 2>"$ERR_FILE"
echo '== ERR =='
cat "$ERR_FILE" | sed 's-^-[E] -'
echo '== OUT =='
cat "$OUT_FILE" | sed 's-^-[O] -'
