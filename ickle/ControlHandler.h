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

#ifndef CONTROLHANDLER_H
#define CONTROLHANDLER_H

#include <config.h>

#ifdef CONTROL_SOCKET

#include <sigc++/signal_system.h>

#include "ControlSocket.h"

// ============================================================================
//  ControlHandler
// ============================================================================

class ControlHandler : public SigC::Object
{
 public:
  ControlHandler (class IckleClient & i)
    : ickle (i) { }

  void init ();
  void quit ();

private:
  void input_cb (int, GdkInputCondition);
  void connection_input_cb (int, GdkInputCondition);

  void command (ControlSocket &);

  void cmdSetStatus (ControlSocket &);
  void cmdGetStatus (ControlSocket &);
  void cmdSetInvisible (ControlSocket &);
  void cmdGetInvisible (ControlSocket &);
  void cmdSetAwayMessage (ControlSocket &);
  void cmdGetAwayMessage (ControlSocket &);
  void cmdAddContact (ControlSocket &);
  void cmdSendMessage (ControlSocket &);
  void cmdQuit (ControlSocket &);

  void addTimeout (int, SigC::Connection, SigC::Connection);
  void endTimeout (int, bool, const string &);
  int timeout_cb (int);

  void self_status_change_cb (ICQ2000::StatusChangeEvent *, int, ICQ2000::Status);
  void messageack_cb         (ICQ2000::MessageEvent *, int, ICQ2000::MessageEvent *);

  IckleClient & ickle;
  ControlSocketServer m_socket;

  map <int, SigC::Connection> m_connections;
  map <int, pair <SigC::Connection, SigC::Connection> > m_timeouts;
};

#endif // CONTROL_SOCKET

#endif // CONTROLHANDLER_H
