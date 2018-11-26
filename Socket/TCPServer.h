/*
* @file TCPServer.h
* @brief wrapper for TCP server
*
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
* @date 2013-05-11
*/

#ifndef INCLUDE_TCPSERVER_H_
#define INCLUDE_TCPSERVER_H_

#include <algorithm>
#include <cstddef>   // size_t
#include <cstdlib>
#include <cstring>   // strerror, strlen, memcpy, strcpy
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "Socket.h"

#ifdef WINDOWS
#undef min
#undef max
#endif

class CTCPServer : public ASocket
{
public:
   explicit CTCPServer(const LogFnCallback oLogger,
                       /*const std::string& strAddr,*/
                       const std::string& strPort,
                       const SettingsFlag eSettings = ALL_FLAGS)
                       /*throw (EResolveError)*/;
   
   ~CTCPServer() override;

   // copy constructor and assignment operator are disabled
   CTCPServer(const CTCPServer&) = delete;
   CTCPServer& operator=(const CTCPServer&) = delete;

   /* returns the socket of the accepted client, the waiting period can be set */
   bool Listen(Socket& ClientSocket, size_t msec = ACCEPT_WAIT_INF_DELAY);

   int Receive(const Socket ClientSocket,
               char* pData,
               const size_t uSize,
               bool bReadFully = true) const;

   bool Send(const Socket ClientSocket, const char* pData, const size_t uSize) const;
   bool Send(const Socket ClientSocket, const std::string& strData) const;
   bool Send(const Socket ClientSocket, const std::vector<char>& Data) const;

   bool Disconnect(const Socket ClientSocket) const;

   bool SetRcvTimeout(ASocket::Socket& ClientSocket, unsigned int msec_timeout);
   bool SetRcvTimeout(ASocket::Socket& ClientSocket, struct timeval Timeout);
   bool SetSndTimeout(ASocket::Socket& ClientSocket, unsigned int msec_timeout);
   bool SetSndTimeout(ASocket::Socket& ClientSocket, struct timeval Timeout);

protected:
   Socket m_ListenSocket;

   //std::string m_strHost;
   std::string m_strPort;

   #ifdef WINDOWS
   struct addrinfo* m_pResultAddrInfo;
   struct addrinfo  m_HintsAddrInfo;
   #else
   struct sockaddr_in m_ServAddr;
   #endif

};

#endif
