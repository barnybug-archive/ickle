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

#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <string>
#include <vector>

// ============================================================================
//  CommandLineParser
// ============================================================================

class CommandLineParser : public std::vector <class CommandLineOption>
{
 public:
  CommandLineParser (int, char **);
};


// ============================================================================
//  CommandLineOption
// ============================================================================

class CommandLineOption
{
 public:
  CommandLineOption (const std::string & o, const std::vector<std::string> & a)
    : m_opt (o), m_args (a) { }

  bool isOption (const std::string & long_opt, const std::string & short_opt, int min_args = 0, int max_args = -1);
  std::string argument (int n = 0) const { return (m_args.size() > n) ? m_args[n] : ""; }
  void invalid ();

 private:
  std::string m_opt;
  std::vector <std::string> m_args;
};


// ============================================================================
//  CommandLineException
// ============================================================================

class CommandLineException
{
 public:
  CommandLineException (const std::string & w) : m_what (w) { }
  std::string what () const { return m_what; }
 private:
  std::string m_what;
};

#endif
