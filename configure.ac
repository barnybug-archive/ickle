# Process this file with autoconf to produce a configure script.
AC_INIT(ickle, 0.2.0, barnaby@beedesign.co.uk)
AM_INIT_AUTOMAKE(ickle,0.2.0)

AC_CONFIG_SRCDIR([src/Client.cpp])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_AWK

AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h fcntl.h netdb.h netinet/in.h arpa/inet.h stdlib.h string.h sys/socket.h ctype.h sys/time.h sys/wait.h sys/types.h unistd.h signal.h],, AC_MSG_ERROR([Required header not found]) )
# doesn't matter if getopt is missing it's a gnu thing
AC_CHECK_HEADERS([getopt.h],,)

dnl Checks for library functions.
AC_CHECK_LIB(socket, socket)
AC_CHECK_FUNC(gethostbyname, ,
  AC_CHECK_LIB(nsl, gethostbyname,, AC_MSG_ERROR([You do not have gethostbyname - check you have libc installed properly])) )

dnl optional GNOME support
ICKLE_GNOME_INIT(applets)

if test "x$want_gnome" = xyes; then
   ICKLE_PROGRAM=ickle_applet
   ICKLE_MANPAGES=ickle_applet.1
   CORBA_FILES=ickle_applet.gnorba
   DESKTOP_FILES=ickle_applet.desktop
   AC_DEFINE(GNOME_ICKLE,,[Define if GNOME applet should be built])
else
   ICKLE_PROGRAM=ickle
   ICKLE_MANPAGES=ickle.1
fi

AC_SUBST(ICKLE_PROGRAM)
AC_SUBST(ICKLE_MANPAGES)
AC_SUBST(CORBA_FILES)
AC_SUBST(DESKTOP_FILES)

dnl Switch to C++ mode and check for needed C++ headers
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_CHECK_HEADERS([vector iostream fstream string algorithm list memory iterator exception utility],,
  AC_MSG_ERROR([You need to have the libstdc++ headers installed]))

AC_CHECK_HEADERS(ext/hash_map,,
  AC_CHECK_HEADERS(hash_map,, AC_MSG_ERROR([Your libstdc++ installation does not have the extra hash classes])))

AC_CHECK_HEADERS(sstream,,
  AC_CHECK_HEADERS(strstream,, AC_MSG_ERROR([Your libstdc++ doesn't have the sstream or strstream classes])))

AC_LANG_RESTORE

# Checks for libraries.
AM_PATH_SIGC(1.0.0, [], [AC_MSG_ERROR([You need to have the libsigc++ libraries installed - see the README for more information])])

AM_PATH_GTKMM(1.2.5, [], [AC_MSG_ERROR([You need to have the gtkmm libraries installed of at least version 1.2.5 and above - see the README for more information])])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_C_INLINE
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MKTIME
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_FORK
AC_FUNC_SETVBUF_REVERSED
AC_HEADER_SYS_WAIT
AC_CHECK_FUNCS([dup2 mkdir select socket strerror],,AC_MSG_ERROR([You do not have one of the standard C functions required - check you have libc installed properly]))

AC_CONFIG_FILES([Makefile src/Makefile ickle/Makefile examples/Makefile
share/Makefile share/icons/Makefile share/icons/ickle/Makefile
share/icons/doors/Makefile share/icons/icq/Makefile share/icons/new/Makefile
share/icons/gnomeicu/Makefile share/translations/Makefile])
AC_OUTPUT

$ECHO
$ECHO "-----------------------------------------------------------------"
if test "x$want_gnome" = xyes; then
   $ECHO "Ickle will be built with the GNOME applet."
   $ECHO "After installing, run ickle by rightclicking the"
   $ECHO "panel and choosing Applets->Network->Ickle ICQ."
   $ECHO "If you would rather ickle is not built as an applet,"
   $ECHO "for example if you don't use GNOME as a window manager,"
   $ECHO "then re-run configure with the option --with-gnome=no"
else
   $ECHO "Ickle will be built without the GNOME applet."
   $ECHO "After installing, run ickle by typing 'ickle' at the prompt."
fi
$ECHO "-----------------------------------------------------------------"
$ECHO
