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

string IPtoString(unsigned int ip) {
  ostringstream ostr;
  ostr << (ip >> 24) << "."
       << ((ip >> 16) & 0xff) << "."
       << ((ip >> 8) & 0xff) << "."
       << (ip & 0xff);
  return ostr.str();
}

TCPSocket::TCPSocket() {
  socketDescriptor = -1;
}

TCPSocket::TCPSocket( int fd, struct sockaddr_in addr )
  : socketDescriptor(fd), remoteAddr(addr)
{
  socklen_t localLen = sizeof(struct sockaddr_in);
  getsockname( socketDescriptor, (struct sockaddr *)&localAddr, &localLen );
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

  socklen_t localLen = sizeof(struct sockaddr_in);
  getsockname( socketDescriptor, (struct sockaddr *)&localAddr, &localLen );
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
  if (ret == 0) throw SocketException( "Other end closed connection" );
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

  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return false;
    else throw SocketException( strerror(errno) );
  } else {
    if (ret == 0) throw SocketException( "Other end closed connection" );
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

unsigned short TCPSocket::getRemotePort() const {
  return ntohs( remoteAddr.sin_port );
}

unsigned int TCPSocket::getRemoteIP() const {
  return ntohl( remoteAddr.sin_addr.s_addr );
}

unsigned short TCPSocket::getLocalPort() const {
  return ntohs( localAddr.sin_port );
}

unsigned int TCPSocket::getLocalIP() const {
  return ntohl( localAddr.sin_addr.s_addr );
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
 * TCPServer class
 */
TCPServer::TCPServer() {
  socketDescriptor = -1;
}

TCPServer::~TCPServer() {
  Disconnect();
}

void TCPServer::StartServer() {

  if (socketDescriptor != -1) throw SocketException("Already listening");
  
  socketDescriptor = socket( AF_INET, SOCK_STREAM, 0 );
  if (socketDescriptor <= 0) throw SocketException("Couldn't create socket");
  
  /*
   * don't bother with bind, we don't care which port
   * it picks to listen on, just so long as we can find
   * out which it is
   *   localAddr.sin_family = AF_INET;
   *   localAddr.sin_addr.s_addr = INADDR_ANY;
   *   localAddr.sin_port = 0;
   *
   *   if ( bind( socketDescriptor,
   *   (struct sockaddr *)&localAddr,
   *   sizeof(struct sockaddr) ) < 0 ) throw SocketException("Couldn't bind socket");
  */

  listen( socketDescriptor, 5 );
  // queue size of 5 should be sufficient
  
  socklen_t localLen = sizeof(struct sockaddr_in);
  getsockname( socketDescriptor, (struct sockaddr *)&localAddr, &localLen );
}

unsigned short TCPServer::getPort() const {
  return ntohs( localAddr.sin_port );
}

unsigned int TCPServer::getIP() const {
  return ntohl( localAddr.sin_addr.s_addr );
}

TCPSocket* TCPServer::Accept() {
  int newsockfd;
  socklen_t remoteLen;
  struct sockaddr_in remoteAddr;

  if (socketDescriptor == -1) throw SocketException("Not connected");

  remoteLen = sizeof(remoteAddr);
  newsockfd = accept( socketDescriptor,
		      (struct sockaddr *) &remoteAddr, 
		      &remoteLen );
  if (newsockfd < 0) throw SocketException("Error on accept");

  return new TCPSocket( newsockfd, remoteAddr );
}

int TCPServer::getSocketHandle() { return socketDescriptor; }

void TCPServer::Disconnect() {
  if (socketDescriptor != -1) {
    close(socketDescriptor);
    socketDescriptor = -1;
  }
}

/**
 * SocketException class
 */
SocketException::SocketException(const string& text) : m_errortext(text) { }

const char* SocketException::what() const {
  return m_errortext.c_str();
}
