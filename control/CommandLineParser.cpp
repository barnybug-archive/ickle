/*
 * Copyright (C) 2002 Dominic Sacré <bugcreator@gmx.de>.
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

#include <string>
#include "ickle/sstream_fix.h"

#include "CommandLineParser.h"

using std::string;
using std::ostringstream;
using std::vector;

// ============================================================================
//  CommandLineParser
// ============================================================================

CommandLineParser::CommandLineParser (int argc, char ** argv)
{
  if (argc > 1 && (argv[1])[0] != '-') throw CommandLineException ("Invalid option `" + string(argv[1]) + "'");

  for (int i = 1; i < argc; i++) {
    if ((argv[i])[0] == '-') {
      string opt = argv[i];
      vector <string> args;
      while ((i+1 < argc) && ((argv[i+1])[0] != '-')) {
        args.push_back (argv[i+1]);
        i++;
      }
      m_opts.push_back (CommandLineOption (opt, args));
    }
  }
}


// ============================================================================
//  CommandLineOption
// ============================================================================

bool CommandLineOption::isOption (const string & long_opt, const string & short_opt, int min_args, int max_args) const
{
  ostringstream s;

  if ((m_opt == "--" + long_opt && !long_opt.empty()) ||
      (m_opt == "-" + short_opt && !short_opt.empty()))
  {
    if (max_args == -1 && int(m_args.size()) != min_args) {
      s << "Option `" << m_opt << "' requires " << min_args << " arguments";
      throw CommandLineException (s.str());
    }
    if (int(m_args.size()) < min_args) {
      s << "Option `" << m_opt << "' requires at least " << min_args << " arguments";
      throw CommandLineException (s.str());
    }
    if (int(m_args.size()) > max_args) {
      s << "Option `" << m_opt << "' doesn't take more than " << max_args << " arguments";
      throw CommandLineException (s.str());
    }
    return true;
  }
  else
  {
    return false;
  }
}

void CommandLineOption::invalid () const
{
  throw CommandLineException ("Invalid option `" + m_opt + "'");
}
