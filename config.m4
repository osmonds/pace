dnl $Id$
dnl config.m4 for extension pace

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(pace, for pace support,
dnl Make sure that the comment is aligned:
dnl [  --with-pace             Include pace support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(pace, whether to enable Php access control extension support,
[  --enable-pace           Enable PHP access control extension support])

if test "$PHP_PACE" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-pace -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/pace.h"  # you most likely want to change this
  dnl if test -r $PHP_PACE/$SEARCH_FOR; then # path given as parameter
  dnl   PACE_DIR=$PHP_PACE
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for pace files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PACE_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PACE_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the pace distribution])
  dnl fi

  dnl # --with-pace -> add include path
  dnl PHP_ADD_INCLUDE($PACE_DIR/include)

  dnl # --with-pace -> check for lib and symbol presence
  dnl LIBNAME=pace # you may want to change this
  dnl LIBSYMBOL=pace # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PACE_DIR/lib, PACE_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PACELIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong pace lib version or lib not found])
  dnl ],[
  dnl   -L$PACE_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PACE_SHARED_LIBADD)

  CFLAGS="$CFLAGS -Wall -fvisibility=hidden"

  # Check libselinux
  PHP_CHECK_LIBRARY(selinux, is_selinux_enabled,
  [
    PHP_ADD_LIBRARY(selinux, 1, PACE_SHARED_LIBADD)
  ], [
    AC_MSG_ERROR("libselinux not found!")
  ], [
    -lselinux
  ])

  # Check libaudit
  PHP_CHECK_LIBRARY(audit, audit_open,
  [
    PHP_ADD_LIBRARY(audit, 1, PACE_SHARED_LIBADD)
  ], [
    AC_MSG_ERROR("libaudit not found!")
  ], [
    -laudit
  ])


  pace_sources="pace.c \
                pace_selinux.c"

  PHP_NEW_EXTENSION(pace, $pace_sources, $ext_shared)
  PHP_SUBST(PACE_SHARED_LIBADD)
fi
