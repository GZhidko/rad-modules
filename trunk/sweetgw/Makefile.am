#
# Makefile.am for the sweetgw OpenRADIUS module
#
# Written by Ilya Etingof <ilya@glas.net>, 2006
#
# $Log: not supported by cvs2svn $
# Revision 1.3  2006/04/13 23:16:01  ilya
# libsweetmisc added
#
# Revision 1.2  2006/04/13 23:00:23  ilya
# make use of SWEETSPOT_INCLUDE_DIR & SWEETSPOT_LIB_DIR
#
# Revision 1.1.1.1  2006/04/13 22:55:33  ilya
# initial revision
#
#
sbin_PROGRAMS    = sweetgw
INCLUDES         = -I@OPENRADIUS_INCLUDE_DIR@ -I@SWEETSPOT_INCLUDE_DIR@

sweetgw_SOURCES    = sweetgw.c tlog.c
sweetgw_LDADD      = -L@SWEETSPOT_LIB_DIR@ -lsweetuam -lsweetmisc
