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

#include <gtkmm/main.h>

#include <libicq2000/Client.h>

#include "IckleClient.h"
#include "ControlHandler.h"

#include "ickle.h"
#include "ucompose.h"
#include "utils.h"

using std::make_pair;
using std::string;
using std::cerr;
using std::endl;

// ============================================================================
//  ControlHandler
// ============================================================================

void ControlHandler::init ()
{
  if (!m_socket.init (BASE_DIR)) {
    cerr << Utils::console( Glib::ustring( _("Disabling control socket") ) ) << endl;
    return;
  }

  // connect input callback
  Glib::signal_io().connect ( SigC::slot (*this, &ControlHandler::input_cb), m_socket.sd(),
			      (Glib::IO_IN | Glib::IO_ERR) );
}

void ControlHandler::quit ()
{
  m_socket.quit ();
}


bool ControlHandler::input_cb (Glib::IOCondition cond)
{
  // look for new connections
  int sd = m_socket.getConnection ();
  if (sd != -1)
  {
    // connect an input callback for the new socket
    SigC::Connection c = Glib::signal_io().connect ( SigC::bind( SigC::slot (*this, &ControlHandler::connection_input_cb), sd), sd,
						     (Glib::IO_IN | Glib::IO_ERR) );
    m_connections.insert (make_pair (sd, c));
  }

  return true;
}

bool ControlHandler::connection_input_cb (Glib::IOCondition cond, int sd)
{
  ControlSocket c (sd);

  ControlSocket::ReadStatus s = c.readStatus ();

  if (s == ControlSocket::READ)
  {
    command (c);
  }
  else if (s == ControlSocket::CLOSED)
  {
    // disconnect input callback
    m_connections[sd].disconnect ();
    m_connections.erase (sd);
    // close socket
    m_socket.closeConnection (sd);
  }

  return true;
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
    case CMD_SET_SETTING:       cmdSetSetting (s); break;
    case CMD_GET_SETTING:       cmdGetSetting (s); break;
    case CMD_QUIT:              cmdQuit (s); break;
  }
}

// --- timeout ---

void ControlHandler::addTimeout (int sd, SigC::Connection c1, SigC::Connection c2)
{
  m_timeouts.insert (make_pair (sd, make_pair (c1, c2)));
}

void ControlHandler::endTimeout (int sd, bool success, const string & info)
{
  ControlSocket (sd) << success << info;

  if (m_timeouts.find (sd) != m_timeouts.end ())
  {
    m_timeouts[sd].first.disconnect ();
    m_timeouts[sd].second.disconnect ();
    m_timeouts.erase (sd);
  }
}

bool ControlHandler::timeout_cb (int sd)
{
  endTimeout (sd, false, _("timeout reached") );
  return false;
}

// --- status ---

void ControlHandler::cmdSetStatus (ControlSocket & s)
{
  ICQ2000::Status status;
  int timeout;
  s >> status >> timeout;

  if (icqclient.getStatus() == status)
  {
    endTimeout (s.sd(), true);
    return;
  }

  /* TODO
  if (timeout != 0)
  {
    addTimeout ( s.sd(),
                 Glib::signal_timeout().connect (SigC::bind (SigC::slot (*this, &ControlHandler::timeout_cb), s.sd()), timeout),
                 icqclient.self_contact_status_change_signal.connect (SigC::bind (SigC::slot (*this, &ControlHandler::self_status_change_cb),
									    s.sd(), status)) );
  }
  */
  icqclient.setStatus (status);

  endTimeout(s.sd(), true); // TODO remove!
}

void ControlHandler::cmdGetStatus (ControlSocket & s)
{
  s << icqclient.getStatus ();
}

void ControlHandler::self_status_change_cb (ICQ2000::StatusChangeEvent *ev, int sd, ICQ2000::Status status)
{
  if (ev->getStatus() == status)
    endTimeout (sd, true);
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

  ICQ2000::ContactRef c = icqclient.getContact (uin);
  if (c.get() == NULL) {
    ICQ2000::ContactRef nc( new ICQ2000::Contact(uin) );
    /* TODO
    icqclient.addContact(nc);
    */
  }
}

// --- send message ---

void ControlHandler::cmdSendMessage (ControlSocket & s)
{
  unsigned int uin;
  string msg;
  int timeout;
  CommandMessageType type;
  ICQ2000::MessageEvent *ev;

  s >> uin >> type >> msg >> timeout;

  if (!icqclient.isConnected ()) {
    endTimeout (s.sd(), false, _("not connected") );
    return;
  }
  if (icqclient.getContact(uin).get() == NULL)
  {
    endTimeout (s.sd(), false, String::ucompose( _("UIN %1 is not on your contact list"), uin ) );
    return;
  }
  
  switch (type) {
   case MESSAGE_SMS:	
    ev = new ICQ2000::SMSMessageEvent (icqclient.getContact(uin), msg, true);
    break;
   case MESSAGE_Normal:
    ev = new ICQ2000::NormalMessageEvent (icqclient.getContact(uin), msg);
    break;
  }
  
  /* TODO
  if (timeout != 0)
  {
    addTimeout ( s.sd(),
                 Glib::signal_timeout().connect (SigC::bind (SigC::slot (*this, &ControlHandler::timeout_cb), s.sd()), timeout),
                 icqclient.messageack.connect (SigC::bind (SigC::slot (*this, &ControlHandler::messageack_cb), s.sd(), ev)) );
  }
  */

  icqclient.SendEvent (ev);
  endTimeout(s.sd(), true); // TODO remove!
}

void ControlHandler::messageack_cb (ICQ2000::MessageEvent *ack, int sd, ICQ2000::MessageEvent *ev)
{
  if (ack == ev)
  {
    endTimeout (sd, true);
  }
}

// --- setting ---

void ControlHandler::cmdSetSetting (ControlSocket & s)
{
  string key, value;

  s >> key >> value;

  if (!(g_settings.getValueString (key).empty()))
  {
    g_settings.setValue (key, value);
    s << true;
  }
  else {
    s << false;
  }
}

void ControlHandler::cmdGetSetting (ControlSocket & s)
{
  string key, value;

  s >> key;
  value = g_settings.getValueString (key);

  if (!value.empty()) {
    s << true << value;
  } else {
    s << false;
  }
}

// --- quit ---

void ControlHandler::cmdQuit (ControlSocket &)
{
  ickle.exit_cb (); // hmm...
  ickle.quit ();
}

#endif // CONTROL_SOCKET
