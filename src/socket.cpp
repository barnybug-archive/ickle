#include "socket.h"

/*
 * TCPSocket class
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


TCPSocket::TCPSocket() {
  socketDescriptor = -1;
}

TCPSocket::~TCPSocket() {
  Disconnect();
}

void TCPSocket::Connect() {
  if (socketDescriptor != -1) {
    throw SocketException("Already connected");
  } else {
    socketDescriptor = socket(AF_INET,SOCK_STREAM,0);
    if (socketDescriptor <= 0) throw SocketException("Couldn't create socket");
  }      
  remoteAddr.sin_family = AF_INET;

  if (connect(socketDescriptor,(struct sockaddr *)&remoteAddr,sizeof(struct sockaddr)) < 0)
    throw SocketException("Couldn't connect socket");
}

void TCPSocket::Disconnect() {
  if (socketDescriptor != -1) {
    close(socketDescriptor);
    socketDescriptor = -1;
  }
}

int TCPSocket::getSocketHandle() { return socketDescriptor; }

bool TCPSocket::connected() {
  return (socketDescriptor != -1);
}

void TCPSocket::Send(Buffer& b) {
  if (!connected()) throw SocketException("Not connected");

  int ret;
  unsigned int sent = 0;

  while (sent < b.size())
  {
    ret = send(socketDescriptor, b.Data() + sent, b.size() - sent, 0);
    if (ret == -1) throw SocketException("Sending on socket");
    sent += ret;
  }
}

void TCPSocket::RecvBlocking(Buffer& b) {
  if (!connected()) throw SocketException("Not connected");

  unsigned char buffer[max_receive_size];

  int ret = recv(socketDescriptor, buffer, max_receive_size, 0);
  if (ret == 0) throw SocketException("Socket closed at other end");
  if (ret < 0) throw SocketException( strerror(errno) );

  b.Pack(buffer,ret);
}

bool TCPSocket::RecvNonBlocking(Buffer& b) {
  if (!connected()) throw SocketException("Not connected");

  unsigned char buffer[max_receive_size];

  int f = fcntl(socketDescriptor, F_GETFL);
  fcntl(socketDescriptor, F_SETFL, f | O_NONBLOCK);
  int ret = recv(socketDescriptor, buffer, max_receive_size, 0);
  fcntl(socketDescriptor, F_SETFL, f & ~O_NONBLOCK);

  if (ret == 0) throw SocketException("Socket closed at other end");
  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return false;
    else throw SocketException( strerror(errno) );
  }

  b.Pack(buffer,ret);
  return true;
}
  
void TCPSocket::setRemoteHost(const char *host) {
  remoteAddr.sin_addr.s_addr = gethostname(host);
}

void TCPSocket::setRemotePort(unsigned short port) {
  remoteAddr.sin_port = htons(port);
}

unsigned long TCPSocket::gethostname(const char *hostname) {

  // try and resolve hostname
  struct hostent *hostEnt;
  if ((hostEnt = gethostbyname(hostname)) == NULL || hostEnt->h_addrtype != AF_INET) {
    throw SocketException("DNS lookup failed");
  } else {
    return *((unsigned long *)(hostEnt->h_addr));
  }
}

/**
 * SocketException class
 */
SocketException::SocketException(const string& text) : m_errortext(text) { }

const char* SocketException::what() const {
  return m_errortext.c_str();
}

