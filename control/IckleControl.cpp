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

#include <stdlib.h>

#include "CommandLineParser.h"
#include "IckleControl.h"

#include <iostream>

using std::cout;
using std::cerr;
using std::string;
using std::endl;

using std::vector;


// ============================================================================
//  IckleControl
// ============================================================================

// --- main ---

int IckleControl::main (int argc, char ** argv)
{
  try
  {
    CommandLineParser p (argc, argv);
    string config_dir;

    if (argc == 1) {
      printUsage ();
      return 0;
    }

    // first handle options which don't require ickle to be running...
    for (CommandLineParser::iterator o = p.begin(); o != p.end(); ++o) {
      if (o->isOption ("help", "h")) {
        printUsage ();
        return 0;
      }
      else if (o->isOption ("version", "v")) {
        printVersion ();
        return 0;
      }
      else if (o->isOption ("configdir", "b", 1)) {
        config_dir = o->argument();
      }
    }

    // determine the config directory to be used
    if (config_dir.empty()) {
      char * dir = getenv ("HOME");
      if (dir == NULL) dir = getenv ("PWD");
      if (dir != NULL) {
        config_dir = string(dir) + "/.ickle/";
      } else {
        config_dir = ".ickle/";
      }
    }

    for (CommandLineParser::iterator o = p.begin(); o != p.end(); ++o) {
      if (o->isOption ("running", "r")) {
        if (m_socket.init (config_dir, true)) {
          m_socket.quit ();
          return 0;
        } else {
          return 1;
        }
      }
    }

    if (!m_socket.init (config_dir)) {
      cerr << "Is ickle running?" << endl;
      return 1;
    }
    bool b = runCommands (p);
    m_socket.quit ();

    return b ? 0 : 1;
  }
  catch (CommandLineException & e)
  {
    cerr << e.what() << endl;
    return 1;
  }
}


bool IckleControl::runCommands (CommandLineParser & p)
{
  for (CommandLineParser::iterator o = p.begin(); o != p.end(); ++o) {
    if      (o->isOption ("timeout",    "t", 0, 1)) { setTimeout (o->argument()); }
    else if (o->isOption ("status",     "s", 0, 1)) { if (!cmdStatus (o->argument())) return false; }
    else if (o->isOption ("invisible",  "i", 0, 1)) { if (!cmdInvisible (o->argument())) return false; }
    else if (o->isOption ("away",       "a", 0, 1)) { cmdAwayMessage (o->argument()); }
    else if (o->isOption ("addcontact", "c", 1))    { if (!cmdAddContact (o->argument())) return false; }
    else if (o->isOption ("send",       "m", 2))    { if (!cmdSendMessage (o->argument(0), o->argument(1), MESSAGE_Normal)) return false; }
    else if (o->isOption ("sms",        "o", 2))    { if (!cmdSendMessage (o->argument(0), o->argument(1), MESSAGE_SMS)) return false; }
    else if (o->isOption ("setting",    "S", 1, 2)) { if (!cmdSetting (o->argument(0), o->argument(1))) return false; }
    else if (o->isOption ("quit",       "q", 0))    { cmdQuit (); }
    else if (o->isOption ("configdir",  "b", 1))    { /* nop here */ }
    else {
      o->invalid ();
    }
  }

  return true;
}


// --- usage ---

void IckleControl::printUsage ()
{
  cout  << "Usage: ickle_control OPTIONS..." << endl
        << endl
        << "Options:" << endl
        << "  -r, --running             Determine whether ickle is running" << endl
        << "  -b, --configdir DIR       Use DIR as the ickle configuration directory." << endl
        << "                            Default is ~/.ickle/" << endl
        << "  -t, --timeout [TIMEOUT]   Wait at most TIMEOUT seconds for the next command" << endl
        << "                            to succeed. Omit TIMEOUT to return immediately" << endl
        << "  -s, --status [STATUS]     Set/get status. STATUS can be one of: online, away," << endl
        << "                            na, occupied, dnd, freeforchat, offline" << endl
        << "  -i, --invisible [BOOL]    Set/get invisibility" << endl
        << "  -a, --away [\"MESSAGE\"]    Set/get away message" << endl
        << "  -c, --addcontact UIN      Add a new contact" << endl
        << "  -m, --send UIN \"MESSAGE\"  Send MESSAGE to user UIN" << endl
        << "  -o, --sms UIN \"MESSAGE\"   Send SMS to user UIN" << endl
        << "  -S, --setting KEY [VALUE] Set/get the setting KEY. >>Use with caution!<<" << endl
        << "  -q, --quit                Terminate ickle" << endl
        << "  -h, --help                Display this help and exit" << endl
        << "  -v, --version             Display ickle version and exit" << endl
        << endl;
}

// --- version ---

void IckleControl::printVersion ()
{
  cout  << "ickle " << ICKLE_VERSION << endl;
}


// --- timeout ---

bool IckleControl::setTimeout (const string & param)
{
  if (!param.empty()) {
    char * end;
    m_timeout = strtol (param.c_str(), &end, 10);
    if (*end != '\0') {
      cerr << "Invalid timeout value `" << param << "'" << endl;
      return false;
    }
  }
  else {
    m_timeout = 0;
  }

  return true;
}


// --- status ---

bool IckleControl::cmdStatus (const string & param)
{
  if (!param.empty ()) {
    // set status
    ICQ2000::Status s;

    if      (param == "online")      s = ICQ2000::STATUS_ONLINE;
    else if (param == "away")        s = ICQ2000::STATUS_AWAY;
    else if (param == "na")          s = ICQ2000::STATUS_NA;
    else if (param == "occupied")    s = ICQ2000::STATUS_OCCUPIED;
    else if (param == "dnd")         s = ICQ2000::STATUS_DND;
    else if (param == "freeforchat") s = ICQ2000::STATUS_FREEFORCHAT;
    else if (param == "offline")     s = ICQ2000::STATUS_OFFLINE;
    else {
      cerr << "Invalid status `" << param << "'" << endl;
      return false;
    }

    m_socket << CMD_SET_STATUS << s << m_timeout*1000;

    if (m_timeout > 0) {
      bool r;
      string str;
      m_socket >> r >> str;
      if (!r) {
        cerr << "Failed setting status to `" << param << "'";
        if (!str.empty()) cerr << " (" << str << ")";
        cerr << endl;
        return false;
      }
    }
    return true;
  }
  else {
    // get status
    m_socket << CMD_GET_STATUS;
    ICQ2000::Status s;
    m_socket >> s;
    string str;

    if      (s == ICQ2000::STATUS_ONLINE)      str = "online";
    else if (s == ICQ2000::STATUS_AWAY)        str = "away";
    else if (s == ICQ2000::STATUS_NA)          str = "na";
    else if (s == ICQ2000::STATUS_OCCUPIED)    str = "occupied";
    else if (s == ICQ2000::STATUS_DND)         str = "dnd";
    else if (s == ICQ2000::STATUS_FREEFORCHAT) str = "freeforchat";
    else if (s == ICQ2000::STATUS_OFFLINE)     str = "offline";

    cout << str << endl;
    return true;
  }
}

// --- invisible ---

bool IckleControl::cmdInvisible (const string & param)
{
  if (!param.empty ()) {
    // set invisible
    bool inv;

    if      (param == "true")  inv = true;
    else if (param == "false") inv = false;
    else {
      cerr << "Invalid boolean value `" << param << "'" << endl;
      return false;
    }

    m_socket << CMD_SET_INVISIBLE << inv;
    return true;
  }
  else {
    // get invisible
    m_socket << CMD_GET_INVISIBLE;
    bool inv;
    m_socket >> inv;
    if (inv) cout << "true" << endl;
        else cout << "false" << endl;
    return true;
  }
}

// --- away message ---

void IckleControl::cmdAwayMessage (const string & param)
{
  if (!param.empty ()) {
    // set away message
    m_socket << CMD_SET_AWAY_MESSAGE << param;
  }
  else {
    // get away message
    m_socket << CMD_GET_AWAY_MESSAGE;
    string str;
    m_socket >> str;
    cout << str << endl;
  }
}

// --- add contact ---

bool IckleControl::cmdAddContact (const string & param)
{
  unsigned int uin;
  char * end;
  uin = strtol (param.c_str(), &end, 10);
  if (*end != '\0') {
    cerr << "Invalid UIN `" << param << "'" << endl;
    return false;
  }
  m_socket << CMD_ADD_CONTACT << uin;
  return true;
}

// --- send message ---

bool IckleControl::cmdSendMessage (const string & param1, const string & param2, CommandMessageType type)
{
  unsigned int uin;
  char * end;
  uin = strtol (param1.c_str(), &end, 10);
  if (*end != '\0') {
    cerr << "Invalid UIN `" << param1 << "'" << endl;
    return false;
  }
  m_socket << CMD_SEND_MESSAGE << uin << type << param2 << m_timeout*1000;

  if (m_timeout > 0) {
    bool r;
    string str;
    m_socket >> r >> str;
    if (!r) {
      cerr << "Failed sending message to `" << param1 << "'";
      if (!str.empty()) cerr << " (" << str << ")";
      cerr << endl;
      return false;
    }
  }
  return true;
}

// --- setting ---

bool IckleControl::cmdSetting (const string & key, const string & value)
{
  if (!value.empty()) {
    // set setting
    m_socket << CMD_SET_SETTING << key << value;
    bool r;
    m_socket >> r;
    if (!r) {
      cerr << "No such setting `" << key << "'" << endl;
    }
  }
  else {
    // get setting
    m_socket << CMD_GET_SETTING << key;
    bool r;
    m_socket >> r;
    if (r) {
      string v;
      m_socket >> v;
      cout << v << endl;
    } else {
      cerr << "No such setting `" << key << "'" << endl;
    }
  }
  return true;
}

// --- quit ---

void IckleControl::cmdQuit ()
{
  m_socket << CMD_QUIT;
}
