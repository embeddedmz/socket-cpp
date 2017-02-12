/**
* @file TCPServer.cpp
* @brief implementation of the TCP server class
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
*/

#include "TCPServer.h"


CTCPServer::CTCPServer(LogFnCallback oLogger,
                       const std::string& strAddr,
                       const std::string& strPort) throw (EResolveError) :
   ASocket(oLogger),
   m_ListenSocket(INVALID_SOCKET),
   m_pResultAddrInfo(nullptr),
   m_strHost(strAddr),
   m_strPort(strPort)
{
   // Resolve the server address and port
   ZeroMemory(&m_HintsAddrInfo, sizeof(m_HintsAddrInfo));
   /* AF_INET is used to specify the IPv4 address family. */
   m_HintsAddrInfo.ai_family = AF_INET;
   /* SOCK_STREAM is used to specify a stream socket. */
   m_HintsAddrInfo.ai_socktype = SOCK_STREAM;
   /* IPPROTO_TCP is used to specify the TCP protocol. */
   m_HintsAddrInfo.ai_protocol = IPPROTO_TCP;
   /* AI_PASSIVE flag indicates the caller intends to use the returned socket
   * address structure in a call to the bind function.*/
   m_HintsAddrInfo.ai_flags = AI_PASSIVE;

   int iResult = getaddrinfo(strAddr.c_str(), strPort.c_str(), &m_HintsAddrInfo, &m_pResultAddrInfo);
   if (iResult != 0)
   {
      if (m_pResultAddrInfo != nullptr)
      {
         freeaddrinfo(m_pResultAddrInfo);
         m_pResultAddrInfo = nullptr;
      }

      throw EResolveError(StringFormat("[TCPServer][Error] getaddrinfo failed : %d", iResult));
   }
}

// Renvoie le socket du client autorisé à se connecter au serveur
bool CTCPServer::Listen(ASocket::Socket& ClientSocket)
{
   ClientSocket = INVALID_SOCKET;

   // creates a socket to listen for incoming client connections if it doesn't already exist
   if (m_ListenSocket == INVALID_SOCKET)
   {
      m_ListenSocket = socket(m_pResultAddrInfo->ai_family,
                              m_pResultAddrInfo->ai_socktype,
                              m_pResultAddrInfo->ai_protocol);
      
      if (m_ListenSocket == INVALID_SOCKET)
      {
         m_oLog(StringFormat("[TCPServer][Error] socket failed : %d", WSAGetLastError()));
         freeaddrinfo(m_pResultAddrInfo);
         m_pResultAddrInfo = nullptr;
         return false;
      }

      // bind the listen socket to the host address:port
      int iResult = bind(m_ListenSocket,
                         m_pResultAddrInfo->ai_addr,
                         static_cast<int>(m_pResultAddrInfo->ai_addrlen));
      
      freeaddrinfo(m_pResultAddrInfo);	// free memory allocated by getaddrinfo
      m_pResultAddrInfo = nullptr;

      if (iResult == SOCKET_ERROR)
      {
         m_oLog(StringFormat("[TCPServer][Error] bind failed : %d", WSAGetLastError()));
      }
   }

   sockaddr addrClient;
   int iResult;
   /* SOMAXCONN = allow max number of connexions in waiting */
   iResult = listen(m_ListenSocket, SOMAXCONN);
   if (iResult == SOCKET_ERROR)
   {
      m_oLog(StringFormat("[TCPServer][Error] listen failed : %d", WSAGetLastError()));
      return false;
   }

   // accept client connection, the returned socket will be used for I/O operations
   int iAddrLen = sizeof(addrClient);
   ClientSocket = accept(m_ListenSocket, &addrClient, &iAddrLen);
   if (ClientSocket == INVALID_SOCKET)
   {
      m_oLog(StringFormat("[TCPServer][Error] accept failed : %d", WSAGetLastError()));
      return false;
   }
   
   //char buf1[256];
   //unsigned long len2 = 256UL;
   //if (!WSAAddressToStringA(&addrClient, lenAddr, NULL, buf1, &len2))
      //m_oLog(StringFormat("[TCPServer][Info] Connection from %s", buf1));
   
   return true;
}

/* ret > 0   : bytes received
 * ret == 0  : connection closed
 * ret < 0   : recv failed
 */
int CTCPServer::Receive(CTCPServer::Socket ClientSocket, char* pData, size_t uSize) const
{
   int iResult = recv(ClientSocket, pData, uSize, 0);

   if (iResult < 0)
   {
      m_oLog(StringFormat("[TCPClient][Error] recv failed : %d", WSAGetLastError()));
   }

   return iResult;
}

bool CTCPServer::Disconnect(CTCPServer::Socket ClientSocket)
{
   // The shutdown function disables sends or receives on a socket.
   int iResult = shutdown(ClientSocket, SD_RECEIVE);
   
   if (iResult == SOCKET_ERROR)
   {
      m_oLog(StringFormat("[TCPServer][Error] shutdown failed : %d", WSAGetLastError()));
      return false;
   }

   closesocket(ClientSocket);

   m_ListenSocket = INVALID_SOCKET;
   m_pResultAddrInfo = nullptr;

   return true;
}

CTCPServer::~CTCPServer()
{

}
