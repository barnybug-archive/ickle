# Process this file with autoconf to produce a configure script.
AC_INIT(ickle, 0.1-pre1, barnaby@beedesign.co.uk)
AM_INIT_AUTOMAKE(ickle,0.1-pre1)

AC_CONFIG_SRCDIR([src/Client.cpp])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

AC_PROG_INSTALL

AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for libraries.
AM_PATH_SIGC(1.0.3, [],
 [AM_PATH_SIGC(1.0.0,
  [AC_MSG_WARN(You have libsigc++ libraries version < 1.0.3, these
have not been tested, but it will probably work)]
  [AC_MSG_ERROR(You need to have the libsigc++ libraries installed)])
 ])

AM_PATH_GTKMM(1.2.5, [],
 [AM_PATH_GTKMM(1.2.0,
  [AC_MSG_WARN(You have libsigc++ libraries version < 1.2.5, these
have not been tested)]
  [AC_MSG_ERROR(You need to have the gtkmm libraries installed of at
least version 1.2.0, preferrably 1.2.5 and above)])
])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_HEADER_TIME

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname gethostname select socket])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile ickle/Makefile])
AC_OUTPUT
