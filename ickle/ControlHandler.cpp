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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef CONTROL_SOCKET

#include "sstream_fix.h"

#include <gtk--/main.h>

#include <libicq2000/Client.h>

#include "IckleClient.h"
#include "ControlHandler.h"

using namespace SigC;

// ============================================================================
//  ControlHandler
// ============================================================================

void ControlHandler::init ()
{
  if (!m_socket.init (BASE_DIR)) {
    cerr << "Disabling control socket" << endl;
    return;
  }

  // connect input callback
  Gtk::Main::input.connect ( slot (this, &ControlHandler::input_cb), m_socket.sd(),
                             GdkInputCondition (GDK_INPUT_READ | GDK_INPUT_EXCEPTION) );
}

void ControlHandler::quit ()
{
  m_socket.quit ();
}


void ControlHandler::input_cb (int, GdkInputCondition)
{
  // look for new connections
  int sd = m_socket.getConnection ();
  if (sd != -1) {
    // connect an input callback for the new socket
    Connection c = Gtk::Main::input.connect ( slot (this, &ControlHandler::connection_input_cb), sd,
                                              GdkInputCondition (GDK_INPUT_READ | GDK_INPUT_EXCEPTION) );
    m_connections.insert (make_pair (sd, c));
  }
}

void ControlHandler::connection_input_cb (int sd, GdkInputCondition)
{
  ControlSocket c (sd);

  ControlSocket::ReadStatus s = c.readStatus ();

  if (s == ControlSocket::READ) {
    command (c);
  }
  else if (s == ControlSocket::CLOSED) {
    // disconnect input callback
    m_connections[sd].disconnect ();
    m_connections.erase (sd);
    // close socket
    m_socket.closeConnection (sd);
  }
}


void ControlHandler::command (ControlSocket & s)
{
  CommandType t;
  s >> t;

  switch (t) {
    case CMD_SET_STATUS:        cmdSetStatus (s); break;
    case CMD_GET_STATUS:        cmdGetStatus (s); break;
    case CMD_SET_INVISIBLE:     cmdSetInvisible (s); break;
    case CMD_GET_INVISIBLE:     cmdGetInvisible (s); break;
    case CMD_SET_AWAY_MESSAGE:  cmdSetAwayMessage (s); break;
    case CMD_GET_AWAY_MESSAGE:  cmdGetAwayMessage (s); break;
    case CMD_ADD_CONTACT:       cmdAddContact (s); break;
    case CMD_SEND_MESSAGE:      cmdSendMessage (s); break;
    case CMD_QUIT:              cmdQuit (s); break;
  }
}

// --- timeout ---

void ControlHandler::addTimeout (int sd, Connection c1, Connection c2)
{
  m_timeouts.insert (make_pair (sd, make_pair (c1, c2)));
}

void ControlHandler::endTimeout (int sd, bool success, const string & reason)
{
  ControlSocket s (sd);
  if (success) {
    s << 1 << string("");
  }
  else {
    s << 0 << reason;
  }

  if (m_timeouts.find (sd) != m_timeouts.end ()) {
    m_timeouts[sd].first.disconnect ();
    m_timeouts[sd].second.disconnect ();
    m_timeouts.erase (sd);
  }
}

int ControlHandler::timeout_cb (int sd)
{
  endTimeout (sd, false, "timeout reached");
  return false;
}

// --- status ---

void ControlHandler::cmdSetStatus (ControlSocket & s)
{
  Status status;
  int timeout;
  s >> status >> timeout;

  if (icqclient.getStatus() == status) {
    endTimeout (s.sd(), true, "");
    return;
  }

  if (timeout != 0) {
    addTimeout ( s.sd(),
                 Gtk::Main::timeout.connect (bind (slot (this, &ControlHandler::timeout_cb), s.sd()), timeout),
                 icqclient.self_event.connect (bind (slot (this, &ControlHandler::self_event_cb), s.sd(), status)) );
  }

  icqclient.setStatus (status);
}

void ControlHandler::cmdGetStatus (ControlSocket & s)
{
  s << icqclient.getStatus ();
}

void ControlHandler::self_event_cb (SelfEvent * ev, int sd, Status status)
{
  if (ev->getType() == SelfEvent::MyStatusChange) {
    if ((static_cast<MyStatusChangeEvent*>(ev))->getStatus() == status) {
      endTimeout (sd, true, "");
    }
  }
}

// --- invisible ---

void ControlHandler::cmdSetInvisible (ControlSocket & s)
{
  bool inv;
  s >> inv;
  icqclient.setInvisible (inv);
}

void ControlHandler::cmdGetInvisible (ControlSocket & s)
{
  s << icqclient.getInvisible ();
}

// --- away message ---

void ControlHandler::cmdSetAwayMessage (ControlSocket & s)
{
  string msg;
  s >> msg;
  ickle.gui.setAutoResponse (msg); // hmm...
}

void ControlHandler::cmdGetAwayMessage (ControlSocket & s)
{
  s << ickle.gui.getAutoResponse (); // hmm...
}

// --- add contact ---

void ControlHandler::cmdAddContact (ControlSocket & s)
{
  unsigned int uin;
  s >> uin;

  Contact * c = icqclient.getContact (uin);
  if (c == NULL) {
    Contact nc (uin);
    icqclient.addContact(nc);
  }
}

// --- send message ---

void ControlHandler::cmdSendMessage (ControlSocket & s)
{
  unsigned int uin;
  string msg;
  int timeout;
  s >> uin >> msg >> timeout;

  if (!icqclient.isConnected ()) {
    endTimeout (s.sd(), false, "not connected");
    return;
  }
  if (!icqclient.getContact (uin)) {
    ostringstream ostr;
    ostr << "UIN " << uin << " is not on your contact list";
    endTimeout (s.sd(), false, ostr.str());
    return;
  }

  MessageEvent * ev = new NormalMessageEvent (icqclient.getContact (uin), msg);

  if (timeout != 0) {
    addTimeout ( s.sd(),
                 Gtk::Main::timeout.connect (bind (slot (this, &ControlHandler::timeout_cb), s.sd()), timeout),
                 icqclient.messageack.connect (bind (slot (this, &ControlHandler::messageack_cb), s.sd(), ev)) );
  }

  icqclient.SendEvent (ev);
}

void ControlHandler::messageack_cb (MessageEvent * ack, int sd, MessageEvent * ev)
{
  if (ack == ev) {
    endTimeout (sd, true, "");
  }
}

// --- quit ---

void ControlHandler::cmdQuit (ControlSocket &)
{
  ickle.exit_cb (); // hmm...
  ickle.quit ();
}

#endif // CONTROL_SOCKET
