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

#include <iostream>

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "ControlSocket.h"

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

using std::cerr;
using std::string;
using std::endl;

// ============================================================================
//  ControlSocket
// ============================================================================

ControlSocket & ControlSocket::operator<< (unsigned int v)
{
  send (m_sd, &v, sizeof(unsigned int), 0);
  return *this;
}

ControlSocket & ControlSocket::operator<< (bool v)
{
  send (m_sd, &v, sizeof(bool), 0);
  return *this;
}

ControlSocket & ControlSocket::operator<< (const string & v)
{
  size_t s = v.size() + 1;
  send (m_sd, &s, sizeof(size_t), 0);
  send (m_sd, v.c_str(), s, 0);
  return *this;
}


ControlSocket & ControlSocket::operator>> (unsigned int & v)
{
  recv (m_sd, &v, sizeof(unsigned int), 0);
  return *this;
}

ControlSocket & ControlSocket::operator>> (bool & v)
{
  recv (m_sd, &v, sizeof(bool), 0);
  return *this;
}

ControlSocket & ControlSocket::operator>> (string & v)
{
  size_t s;
  recv (m_sd, &s, sizeof(size_t), 0);
  char *p = new char[s];
  if (p != NULL) {
    recv (m_sd, p, s, 0);
    v = string (p);
    delete [] p;
  }

  return *this;
}


ControlSocket::ReadStatus ControlSocket::readStatus ()
{
  char foo;
  fcntl (m_sd, F_SETFL, O_NONBLOCK);
  int r = recv (m_sd, &foo, sizeof(foo), MSG_PEEK);
  fcntl (m_sd, F_SETFL, 0);

  if (r > 0) return READ;
  else if (r == 0) return CLOSED;
  return WAITING;
}


// ============================================================================
//  ControlSocketServer
// ============================================================================

bool ControlSocketServer::init (const string & socket_path)
{
  m_saddr.sun_family = AF_UNIX;
  snprintf (m_saddr.sun_path, UNIX_PATH_MAX, "%s/ctrlsocket", socket_path.c_str());

  // try to create the directory where the socket should be created
  if ((mkdir (socket_path.c_str(), 0700) == -1) && errno != EEXIST) {
    cerr << "Failed to create directory `" << socket_path.c_str() << "' (Error: " << strerror(errno) << ")" << endl;
    return false;
  }
  // try to delete the socket if it already exists
  if ((unlink (m_saddr.sun_path) == -1) && errno != ENOENT) {
    cerr << "Failed to unlink `" << m_saddr.sun_path << "' (Error: " << strerror(errno) << ")" << endl;
    return false;
  }

  // create the socket
  if ((m_sd = socket (AF_UNIX, SOCK_STREAM, 0)) == -1) {
    cerr << "Failed to open socket" << endl;
    return false;
  }
  if (bind (m_sd, (struct sockaddr *) &m_saddr, sizeof(m_saddr)) == -1) {
    cerr << "Failed to assign `" << m_saddr.sun_path << "' to a socket (Error: " << strerror(errno) << ")" << endl;
    close (m_sd);
    return false;
  }
  chmod (m_saddr.sun_path, 0700);
  listen (m_sd, 5);

  return true;
}

void ControlSocketServer::quit ()
{
  close (m_sd);
  unlink (m_saddr.sun_path);
}


int ControlSocketServer::getConnection ()
{
  fd_set set;
  timeval tv;

  FD_ZERO (&set);
  FD_SET (m_sd, &set);
  tv.tv_sec = 0;
  tv.tv_usec = 0;

  // see whether someone connected to our socket
  if (select (m_sd + 1, &set, NULL, NULL, &tv) > 0) {
    int sd = accept (m_sd, (sockaddr *) NULL, NULL);
    return sd;
  }

  return -1;
}

void ControlSocketServer::closeConnection (int sd)
{
  close (sd);
}


// ============================================================================
//  ControlSocketClient
// ============================================================================

bool ControlSocketClient::init (const string & socket_path, bool quiet)
{
  sockaddr_un saddr;

  saddr.sun_family = AF_UNIX;
  snprintf (saddr.sun_path, UNIX_PATH_MAX, "%s/ctrlsocket", socket_path.c_str());

  if ((m_sd = socket (AF_UNIX, SOCK_STREAM, 0)) == -1) {
    if (!quiet) cerr << "Failed to open socket" << endl;
    return false;
  }
  if (connect (m_sd, (sockaddr *) &saddr, sizeof (saddr)) == -1) {
    if (!quiet) cerr << "Couldn't connect to socket `" << saddr.sun_path << "'" << endl;
    close (m_sd);
    return false;
  }

  return true;
}

void ControlSocketClient::quit ()
{
  close (m_sd);
}
