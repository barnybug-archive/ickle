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

#ifndef CONTROLSOCKET_H
#define CONTROLSOCKET_H

#include <sys/socket.h>
#include <sys/un.h>

#include <string>
#include <set>

#include <libicq2000/constants.h>

#include "ControlCommands.h"

// ============================================================================
//  ControlSocketBase
// ============================================================================

class ControlSocketBase
{
 public:
  ControlSocketBase (int sd = 0) : m_sd (sd) { }

  int sd () const { return m_sd; }

 protected:
  int m_sd;
};


// ============================================================================
//  ControlSocket
// ============================================================================

class ControlSocket : public ControlSocketBase
{
 public:
  ControlSocket (int sd = 0) : ControlSocketBase (sd) { }

  ControlSocket & operator<< (unsigned int);
  ControlSocket & operator<< (bool);
  ControlSocket & operator<< (const std::string &);
  ControlSocket & operator<< (int v) { return operator<< ((unsigned int)v); }
  ControlSocket & operator<< (const CommandType & v) { return operator<< ((unsigned int&)v); }
  ControlSocket & operator<< (const CommandMessageType & v) { return operator<< ((unsigned int&)v); }
  ControlSocket & operator<< (const ICQ2000::Status & v) { return operator<< ((unsigned int&)v); }

  ControlSocket & operator>> (unsigned int &);
  ControlSocket & operator>> (bool &);
  ControlSocket & operator>> (std::string &);
  ControlSocket & operator>> (int & v) { return operator>> ((unsigned int&)v); }
  ControlSocket & operator>> (CommandMessageType & v) { return operator>> ((unsigned int&)v); }
  ControlSocket & operator>> (CommandType & v) { return operator>> ((unsigned int&)v); }
  ControlSocket & operator>> (ICQ2000::Status & v) { return operator>> ((unsigned int&)v); }

  enum ReadStatus {
    WAITING,
    READ,
    CLOSED
  };

  ReadStatus readStatus ();
};


// ============================================================================
//  ControlSocketServer
// ============================================================================

class ControlSocketServer : public ControlSocketBase
{
 public:
  bool init (const std::string & socket_path);
  void quit ();

  int getConnection ();
  void closeConnection (int sd);

 private:
  sockaddr_un m_saddr;
};


// ============================================================================
//  ControlSocketClient
// ============================================================================

class ControlSocketClient : public ControlSocket
{
 public:
  bool init (const std::string & socket_path, bool quiet = false);
  void quit ();
};

#endif
