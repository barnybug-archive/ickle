/*
 * EventSubstituter: stream class for substituting |'s, %'s and dirty
 * dishes in auto responses and event commands.
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef EVENTSUBSTITUTER_H
#define EVENTSUBSTITUTER_H

#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>

#include <libicq2000/Contact.h>

#include "sstream_fix.h"

using std::string;
using std::ostringstream;

using namespace ICQ2000;

class EventSubstituter : public ostringstream {
 private:
  bool line_start;
  char got_special;  // % or |
  string cmd; // command string buffer
  Contact *co;
  time_t event_time;
  bool escape_shell;

  void sanecat(const string &s);
  void execute();

 public:
  EventSubstituter(Contact *c);
  
  void set_contact(Contact *c) { co = c; }
  void set_event_time(time_t t) { event_time = t; }
  void set_escape_shell(bool e) { escape_shell = e; }

  EventSubstituter& operator<<(char c);
  EventSubstituter& operator<<(unsigned char c) { return (*this) << (char)c; }
  EventSubstituter& operator<<(signed char c) { return (*this) << (char)c; }
  EventSubstituter& operator<<(const char *s);

  // even the formatting is copied from gnu iostream.h.
  EventSubstituter& operator<<(const unsigned char *s)
      { return (*this) << (const char*)s; }
  EventSubstituter& operator<<(const signed char *s)
      { return (*this) << (const char*)s; }

  EventSubstituter& operator<<(const string &s);

  string str();
};

// This is just an auxiliary class for event substitution, but might
// as well expose it to other files.
// From Barnaby's shell.cpp:

class PipeExec {
 private:
  FILE *fStdIn, *fStdOut;
  int pid;

 public:
  PipeExec();
  ~PipeExec();

  bool Open(const char *cmd);

  // Reads up to size-1 characters into buf, and NUL-terminates it.
  void Read(char *buf, int size);
  void Write(const char *buf);
  void CloseInput();
  void Close();
};

#endif
