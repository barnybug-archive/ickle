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

class CommandLineParser : public vector <class CommandLineOption>
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
  CommandLineOption (const string & o, const vector<string> & a)
    : m_opt (o), m_args (a) { }

  bool isOption (const string &, const string &, int = 0);
  string argument (int n = 0) const { return (m_args.size() > n) ? m_args[n] : ""; }
  void invalid ();

 private:
  string m_opt;
  vector <string> m_args;
};


// ============================================================================
//  CommandLineException
// ============================================================================

class CommandLineException
{
 public:
  CommandLineException (const string & w) : m_what (w) { }
  string what () const { return m_what; }
 private:
  string m_what;
};

#endif
