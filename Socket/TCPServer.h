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

class CTCPServer : public ASocket
{	
public:
   explicit CTCPServer(LogFnCallback oLogger,
                       const std::string& strAddr,
                       const std::string& strPort) throw (EResolveError);
   
   ~CTCPServer() override;

   // copy constructor and assignment operator are disabled
   CTCPServer(const CTCPServer&) = delete;
   CTCPServer& operator=(const CTCPServer&) = delete;

   /* returns the socket of the accepted client */
   bool Listen(Socket& ClientSocket);
   
   int Receive(Socket ClientSocket,
               char* pData,
               size_t uSize) const;
   
   bool Disconnect(Socket ClientSocket);

protected:
   Socket m_ListenSocket;

   std::string m_strHost;
   std::string m_strPort;

   #ifdef WINDOWS
   struct addrinfo* m_pResultAddrInfo;
   struct addrinfo  m_HintsAddrInfo;
   #else

   #endif

};

#endif
