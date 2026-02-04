#
# Makefile.am for the absgw OpenRADIUS module
#
# Written by Ilya Etingof <ilya@glas.net>, 2004
#
# $Log: not supported by cvs2svn $
#
sbin_PROGRAMS    = absgw
INCLUDES         = -I@OPENRADIUS_INCLUDE_DIR@ -I@ABSWARE_INCLUDE_DIR@

absgw_SOURCES    = absgw.c
absgw_LDADD      = -labs_io -labs_v3 -labs_msg -labs_ber
