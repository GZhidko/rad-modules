#!/usr/bin/env bash
set -euo pipefail

# Solaris toolchain paths (OpenCSW + system tools)
export PATH="/opt/csw/bin:/usr/gnu/bin:/usr/sfw/bin:/usr/ccs/bin:/usr/bin:${PATH}"

if ! command -v gmake >/dev/null 2>&1; then
  echo "ERROR: gmake not found in PATH"
  exit 1
fi
MAKE_BIN=gmake

OPENRADIUS_SRC="${OPENRADIUS_SRC:-/export/home/tt/packaging/openradius-0.9.12c}"
SWEETSPOT_SRC="${SWEETSPOT_SRC:-/export/home/tt/openradius-stuff/sweetspot/src}"
ABSWARE_INCLUDE="${ABSWARE_INCLUDE:-/opt/bwf/include}"
ABSWARE_LIB="${ABSWARE_LIB:-/opt/bwf/lib}"
HIREDIS_INCLUDE="${HIREDIS_INCLUDE:-/opt/bwf/include}"
HIREDIS_LIB="${HIREDIS_LIB:-/opt/bwf/lib}"

build_mod() {
  local d="$1"
  echo "=== BUILD $d ==="
  cd "$d"
  sh ./autogen.sh
  ./configure
  "$MAKE_BIN"
  cd ..
}

build_absgw() {
  echo "=== BUILD absgw ==="
  if [ -f "${OPENRADIUS_SRC}/common/constants.h" ]; then
    cd absgw
    sh ./autogen.sh
    LDFLAGS="-L${ABSWARE_LIB} -R${ABSWARE_LIB}" \
    ./configure --with-openradius-source-dir="${OPENRADIUS_SRC}" --with-absware-include-dir="${ABSWARE_INCLUDE}"
    "$MAKE_BIN"
    cd ..
  else
    echo "SKIP absgw: OPENRADIUS_SRC invalid (${OPENRADIUS_SRC})"
  fi
}

build_sweetgw() {
  echo "=== BUILD sweetgw ==="
  if [ ! -f "${OPENRADIUS_SRC}/common/constants.h" ]; then
    echo "SKIP sweetgw: OPENRADIUS_SRC invalid (${OPENRADIUS_SRC})"
    return
  fi
  if [ -z "${SWEETSPOT_SRC}" ] || [ ! -f "${SWEETSPOT_SRC}/uamclt.h" ]; then
    echo "SKIP sweetgw: set SWEETSPOT_SRC with uamclt.h"
    return
  fi
  cd sweetgw
  sh ./autogen.sh
  LDFLAGS="-L${ABSWARE_LIB} -R${ABSWARE_LIB}" \
  ./configure --with-openradius-source-dir="${OPENRADIUS_SRC}" --with-sweetspot-source-dir="${SWEETSPOT_SRC}"
  "$MAKE_BIN"
  cd ..
}

build_redisgw() {
  echo "=== BUILD redisgw ==="
  local hiredis_link=""
  if [ ! -f "${HIREDIS_INCLUDE}/hiredis/hiredis.h" ]; then
    echo "SKIP redisgw: hiredis headers not found (${HIREDIS_INCLUDE}/hiredis/hiredis.h)"
    return
  fi
  if [ -f "${HIREDIS_LIB}/libhiredis.so.0.13" ]; then
    hiredis_link="${HIREDIS_LIB}/libhiredis.so.0.13"
  elif [ -f "${HIREDIS_LIB}/libhiredis.so" ]; then
    hiredis_link="${HIREDIS_LIB}/libhiredis.so"
  elif [ -f "${HIREDIS_LIB}/libhiredis.a" ]; then
    hiredis_link="${HIREDIS_LIB}/libhiredis.a"
  else
    echo "SKIP redisgw: hiredis library not found in ${HIREDIS_LIB}"
    return
  fi

  cd redisgw
  sh ./autogen.sh
  CPPFLAGS="-I${HIREDIS_INCLUDE}" \
  LDFLAGS="-L${HIREDIS_LIB} -R${HIREDIS_LIB}" \
  LIBS="${hiredis_link} -lsocket -lnsl" \
  ./configure
  "$MAKE_BIN"
  cd ..
}

build_snmpgw() {
  echo "=== BUILD snmpgw ==="
  if ! command -v net-snmp-config >/dev/null 2>&1; then
    echo "SKIP snmpgw: net-snmp-config not found"
    return
  fi
  cd snmpgw
  sh ./autogen.sh
  ./configure
  "$MAKE_BIN"
  cd ..
}

for d in bcgw kvfile mradclient multi_radclient radb64 radmppe radre; do
  build_mod "$d"
done

build_snmpgw
build_redisgw
build_absgw
build_sweetgw

chmod +x stats.sh \
  dbmsdate/dbmsdate gdbmauth/gdbmauth gdbmfile/gdbmfile h323date/h323date mschap/mschap \
  radacctlogger/radacctlogger radacctlogger/radacctwriter \
  radclasspacker/radclasspacker radecho/radecho radeval/radeval \
  radinfologger/radinfologger radinfologger/radlogwriter unhex/unhex

for f in dbmsdate/dbmsdate gdbmauth/gdbmauth gdbmfile/gdbmfile h323date/h323date mschap/mschap \
  radacctlogger/radacctwriter radclasspacker/radclasspacker radecho/radecho radeval/radeval \
  radinfologger/radlogwriter unhex/unhex; do
  perl -c "$f"
done

echo "ALL DONE"
