/*
 * General sockets class wrapper
 *
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

#ifndef SOCKET_H
#define SOCKET_H

#include <exception>

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <string>

#include "buffer.h"

using namespace std;

class TCPSocket {
 private:
  static const unsigned int max_receive_size = 4096;
  
  unsigned long gethostname(const char *hostname);

  int socketDescriptor;
  struct sockaddr_in remoteAddr;

 public:
  TCPSocket();
  ~TCPSocket();

  void Connect();
  void Disconnect();
  
  int getSocketHandle();

  void Send(Buffer& b);

  void RecvBlocking(Buffer& b);
  bool RecvNonBlocking(Buffer& b);

  bool connected();

  void setRemoteHost(const char *host);
  void setRemotePort(unsigned short port);

};

class SocketException : exception {
 private:
  string m_errortext;

 public:
  SocketException(const string& text);

  const char* what() const;
};

#endif
