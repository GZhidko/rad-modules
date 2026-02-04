AC_PREREQ(2.59)
AC_INIT([redisgw], [0.0.1], [elie@rol.ru])
AM_INIT_AUTOMAKE([1.9 foreign])
AC_PROG_CC
AC_CHECK_HEADERS(hiredis/hiredis.h, [], [
    AC_ERROR(require Reddis client headers (https://github.com/redis/hiredis)!)
  ]
)
AC_SEARCH_LIBS(redisConnect, hiredis, [], [
    AC_SEARCH_LIBS(redisConnect, hiredis, [], [
        AC_ERROR(require Reddis client library (https://github.com/redis/hiredis)!)
    ], [ -lsocket ])
  ]
)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
