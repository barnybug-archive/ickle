# Process this file with autoconf to produce a configure script.
AC_INIT(ickle, 0.1.1, barnaby@beedesign.co.uk)
AM_INIT_AUTOMAKE(ickle,0.1.1)

AC_CONFIG_SRCDIR([src/Client.cpp])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CC
AC_PROG_CPP

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h sys/wait.h sys/types.h unistd.h signal.h getopt.h],, AC_MSG_ERROR([Required header not found]) )

dnl Switch to C++ mode and check for needed C++ headers
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_CHECK_HEADERS([vector iostream fstream string algorithm list memory iterator exception utility],,
  AC_MSG_ERROR([You need to have the libstdc++ headers installed]))

AC_CHECK_HEADERS(ext/hash_map,,
  AC_CHECK_HEADERS(hash_map,, AC_MSG_ERROR([Your libstdc++ installation does not have the extra hash classes])))

AC_CHECK_HEADER(sstream,, AC_MSG_ERROR([Although you appear to have libstdc++ headers installed you will need a newer version such as those which come with gcc 2.95.3 or later]))

AC_LANG_RESTORE

# Checks for libraries.
AM_PATH_SIGC(1.0.0, [], [AC_MSG_ERROR([You need to have the libsigc++ libraries installed - see the README for more information])])

AM_PATH_GTKMM(1.2.0, [], [AC_MSG_ERROR([You need to have the gtkmm libraries installed of at least version 1.2.0 preferably 1.2.5 and above - see the README for more information])])

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
AC_CHECK_FUNCS([gethostbyname gethostname mkdir select socket strerror])

AC_CONFIG_FILES([Makefile src/Makefile ickle/Makefile examples/Makefile])
AC_OUTPUT
