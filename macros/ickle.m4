# ickle configure macros

dnl Test for std::locale() with a simple C++ test program
dnl
AC_DEFUN([ICKLE_CXX_HAVE_STD_LOCALE],
[AC_CACHE_CHECK(whether the compiler has std::locale support,
ac_cv_cxx_have_std_locale,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <locale>],[std::locale("");],
 ac_cv_cxx_have_std_locale=yes, ac_cv_cxx_have_std_locale=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_std_locale" = yes; then
 AC_DEFINE(HAVE_STD_LOCALE,,[define if the compiler has the std::locale support])
fi
])dnl

dnl Test for backtrace functions in execinfo.h header
dnl
AC_DEFUN([ICKLE_HAVE_BACKTRACE],
[AC_CACHE_CHECK(for backtrace support,
ac_cv_have_backtrace,
[AC_LANG_SAVE
 AC_LANG_C
 AC_TRY_COMPILE([#include <stdlib.h>
#include <execinfo.h>],[
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace (array, 10);
  strings = backtrace_symbols (array, size);
  free (strings);
],
 ac_cv_have_backtrace=yes, ac_cv_have_backtrace=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_have_backtrace" = yes; then
 AC_DEFINE(HAVE_BACKTRACE,,[define if backtrace is supported])
fi
])dnl