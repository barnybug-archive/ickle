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

#include "CommandLineParser.h"

using std::string;
using std::vector;

// ============================================================================
//  CommandLineParser
// ============================================================================

CommandLineParser::CommandLineParser (int argc, char ** argv)
{
  if (argc > 1 && (argv[1])[0] != '-') throw CommandLineException ("Invalid option `" + string(argv[1]) + '\'');

  for (int i = 1; i < argc; i++) {
    if ((argv[i])[0] == '-') {
      string opt = argv[i];
      vector <string> args;
      while ((i+1 < argc) && ((argv[i+1])[0] != '-')) {
        args.push_back (argv[i+1]);
        i++;
      }
      push_back (CommandLineOption (opt, args));
    }
  }
}


// ============================================================================
//  CommandLineOption
// ============================================================================

bool CommandLineOption::isOption (const string & long_opt, const string & short_opt, int num_args)
{
  if ((m_opt == "--" + long_opt && !long_opt.empty()) ||
      (m_opt == "-" + short_opt && !short_opt.empty())) {
    if (num_args == 0) {
      if (m_args.size() == 0) return true;
      else throw CommandLineException ("Option `" + m_opt + "' doesn't take an argument");
    }
    else if (num_args == 1) {
      if (m_args.size() == 1) return true;
      else throw CommandLineException ("Option `" + m_opt + "' requires one argument");
    }
    else if (num_args == 2) {
      if (m_args.size() == 2) return true;
      else throw CommandLineException ("Option `" + m_opt + "' requires two arguments");
    }
    else if (num_args == -1) {
      if (m_args.size() < 2) return true;
      else throw CommandLineException ("Option `" + m_opt + "' used with invalid arguments");
    }
  }
  else {
    return false;
  }
}

void CommandLineOption::invalid ()
{
  throw CommandLineException ("Invalid option `" + m_opt + "'");
}
