/*
 * Copyright (C) 2003 Barnaby Gray <barnaby@beedesign.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#include "sighandler.h"

#include <signal.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_BACKTRACE
# include <execinfo.h>
# define USE_BACKTRACE
#endif

#include <iostream>

using std::cerr;
using std::endl;

void handle_sigsegv(int s)
{
  if (s == SIGSEGV)
  {
    cerr << "ickle: Segmentation fault. Damn!" << endl;

#ifdef USE_BACKTRACE
    cerr << "Stack backtrace:" << endl;

    void * array [100];
    char ** strings = NULL;
    size_t size;

    size = backtrace(array, 100);
    strings = backtrace_symbols(array, size);
    cerr << "Obtained " << size << " stack frames" << endl;
    for (size_t i = 0; i < size; ++i)
      cerr << strings[i] << endl;

    free(strings);

    cerr << "Please follow the instructions in the README and send this information " << endl
	 << "to the developers. Thanks." << endl;
#endif

    abort();
  }
}

void sighandler_init()
{
  signal(SIGSEGV, handle_sigsegv);
}

