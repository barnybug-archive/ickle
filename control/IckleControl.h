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

#ifndef ICKLECONTROL_H
#define ICKLECONTROL_H

#include <string>
#include "ControlSocket.h"

// ============================================================================
//  IckleControl
// ============================================================================

class IckleControl
{
 public:
  IckleControl () : m_timeout (0) { }

  int main (int, char **);

 private:
  void printUsage ();
  void printVersion ();

  bool runCommands (class CommandLineParser &);

  bool setTimeout (const std::string &);

  bool cmdStatus (const std::string &);
  bool cmdInvisible (const std::string &);
  void cmdAwayMessage (const std::string &);
  bool cmdAddContact (const std::string &);
  bool cmdSendMessage (const std::string &, const std::string &, CommandMessageType);
  void cmdQuit ();

  ControlSocketClient m_socket;

  int m_timeout;
};

#endif
