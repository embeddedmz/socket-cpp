/**
* @file TCPClient.cpp
* @brief implementation of the TCP client class
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
*/

#include "TCPClient.h"

CTCPClient::CTCPClient(LogFnCallback oLogger) :
   ASocket(oLogger),
   m_eStatus(DISCONNECTED),
   #ifdef WINDOWS
   m_ConnectSocket(INVALID_SOCKET),
   m_pResultAddrInfo(nullptr)
   #else
   m_pServer(nullptr)
   #endif
   //m_uRetryCount(0),
   //m_uRetryPeriod(0)
{

}

// Connexion au serveur
bool CTCPClient::Connect(const std::string& strServer, const std::string& strPort)
{
   if (m_eStatus == CONNECTED)
   {
      Disconnect();
      m_oLog("[TCPClient][Warning] Opening a new connexion. The last one was automatically closed.");
   }

   #ifdef WINDOWS
   ZeroMemory(&m_HintsAddrInfo, sizeof(m_HintsAddrInfo));
   /* AF_INET is used to specify the IPv4 address family. */
   m_HintsAddrInfo.ai_family = AF_INET;			
   /* SOCK_STREAM is used to specify a stream socket. */
   m_HintsAddrInfo.ai_socktype = SOCK_STREAM;
   /* IPPROTO_TCP is used to specify the TCP protocol. */
   m_HintsAddrInfo.ai_protocol = IPPROTO_TCP;

   /* Resolve the server address and port */
   int iResult = getaddrinfo(strServer.c_str(), strPort.c_str(), &m_HintsAddrInfo, &m_pResultAddrInfo);
   if (iResult != 0)
   {
      m_oLog(StringFormat("[TCPClient][Error] getaddrinfo failed : %d", iResult));

      if (m_pResultAddrInfo != nullptr)
      {
         freeaddrinfo(m_pResultAddrInfo);
         m_pResultAddrInfo = nullptr;
      }

      return false;
   }

   // socket creation
   m_ConnectSocket = socket(m_pResultAddrInfo->ai_family,   // AF_INET
                            m_pResultAddrInfo->ai_socktype, // SOCK_STREAM
                            m_pResultAddrInfo->ai_protocol);// IPPROTO_TCP

   if (m_ConnectSocket == INVALID_SOCKET)
   {
      m_oLog(StringFormat("[TCPClient][Error] socket failed : %d", WSAGetLastError()));
      freeaddrinfo(m_pResultAddrInfo);
      m_pResultAddrInfo = nullptr;
      return false;
   }
   
   /*
   SOCKET ConnectSocket = INVALID_SOCKET;
   struct sockaddr_in clientService; 

   ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (ConnectSocket == INVALID_SOCKET) {
      printf("Error at socket(): %ld\n", WSAGetLastError());
      WSACleanup();
      return 1;
   }

   // The sockaddr_in structure specifies the address family,
   // IP address, and port of the server to be connected to.
   clientService.sin_family = AF_INET;
   clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
   clientService.sin_port = htons(27015);
   */

   // connexion to the server
   //unsigned uRetry = 0;
   //do
   //{
      iResult = connect(m_ConnectSocket,
                        m_pResultAddrInfo->ai_addr,
                        static_cast<int>(m_pResultAddrInfo->ai_addrlen));
//iResult = connect(m_ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));

      //if (iResult != SOCKET_ERROR)
         //break;

      // retry mechanism
      //if (uRetry < m_uRetryCount)
         //m_oLog(StringFormat("[TCPClient][Error] connect retry %u after %u second(s)", m_uRetryCount + 1, m_uRetryPeriod));

      //if (m_uRetryPeriod > 0)
      //{
         //for (unsigned uSec = 0; uSec < m_uRetryPeriod; uSec++)
            //Sleep(1000);
      //}
   //} while (iResult == SOCKET_ERROR && ++uRetry < m_uRetryCount);
   
   freeaddrinfo(m_pResultAddrInfo);
   m_pResultAddrInfo = nullptr;

   if (iResult != SOCKET_ERROR)
   {
      m_eStatus = CONNECTED;
      return true;
   }
   m_oLog(StringFormat("[TCPClient][Error] Unable to connect to server : %d", WSAGetLastError()));

   #else
   // socket creation
   m_ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_ConnectSocket < 0)
   {
      m_oLog(StringFormat("[TCPClient][Error] opening socket: %s", strerror(errno)));
      return false;
   }
   
   int iPort = atoi(strPort.c_str());
   m_pServer = gethostbyname(strServer.c_str());
   if (m_pServer == nullptr)
   {
      m_oLog("[TCPClient][Error] no such host.");
      return false;
   }
   
   bzero(reinterpret_cast<char *>(&m_ServAddr), sizeof(m_ServAddr));
   
   // copy address
   bcopy(reinterpret_cast<char *>(m_pServer->h_addr), // src
         reinterpret_cast<char *>(&m_ServAddr.sin_addr.s_addr), // dst
         m_pServer->h_length); // len
   
   m_ServAddr.sin_family = AF_INET;
   m_ServAddr.sin_port = htons(iPort);
   

   // connexion to the server
   int iResult = connect(m_ConnectSocket,
                         reinterpret_cast<struct sockaddr *>(&m_ServAddr),
                         sizeof(m_ServAddr));
   if (iResult >= 0)
   {
      m_eStatus = CONNECTED;
      return true;
   }
   m_oLog(StringFormat("[TCPClient][Error] connecting : %s", strerror(errno)));
   #endif

   return false;
}

bool CTCPClient::Send(const char* pData, size_t uSize) const
{
   if (m_eStatus != CONNECTED)
   {
      m_oLog("[TCPClient][Error] send failed : not connected to a server.");
      return false;
   }

   int iResult = 0;
   #ifdef WINDOWS
   iResult = send(m_ConnectSocket, pData, uSize, 0);
   if (iResult == SOCKET_ERROR)
   {
      m_oLog(StringFormat("[TCPClient][Error] send failed : %d", WSAGetLastError()));
      //Disconnect();
      return false;
   }
   #else
   iResult = write(m_ConnectSocket, pData, uSize);
   if (iResult < 0) 
   {
      m_oLog(StringFormat("[TCPClient][Error] writing to socket : %s", strerror(errno)));
      return false;
   }
   #endif
   
   return true;
}

bool CTCPClient::Send(const std::string& strData) const
{
   return Send(strData.c_str(), strData.length());
}

bool CTCPClient::Send(const std::vector<char>& Data) const
{
   return Send(Data.data(), Data.size());
}

/* ret > 0   : bytes received
 * ret == 0  : connection closed
 * ret < 0   : recv failed
 */
int CTCPClient::Receive(char* pData, size_t uSize) const
{
   if (m_eStatus != CONNECTED)
   {
      m_oLog("[TCPClient][Error] recv failed : not connected to a server.");
      return -1;
   }

   int iBytesRcvd = 0;
   
   #ifdef WINDOWS
   iBytesRcvd = recv(m_ConnectSocket, pData, uSize, 0);
   if (iBytesRcvd < 0)
   {
      m_oLog(StringFormat("[TCPClient][Error] recv failed : %d", WSAGetLastError()));
   }
   #else
   //bzero(pData, uSize);
   iBytesRcvd = read(m_ConnectSocket, pData, uSize);
   if (iBytesRcvd < 0)
   {
      m_oLog(StringFormat("[TCPClient][Error] reading from socket : %s", strerror(errno)));
   }
   #endif

   return iBytesRcvd;
}

bool CTCPClient::Disconnect()
{
   if (m_eStatus != CONNECTED)
      return true;

   m_eStatus = DISCONNECTED;

   #ifdef WINDOWS
   // shutdown the connection since no more data will be sent
   int iResult = shutdown(m_ConnectSocket, SD_SEND);
   if (iResult == SOCKET_ERROR)
   {
      m_oLog(StringFormat("[TCPClient][Error] shutdown failed : %d", WSAGetLastError()));
      return false;
   }
   closesocket(m_ConnectSocket);

   if (m_pResultAddrInfo != nullptr)
   {
      freeaddrinfo(m_pResultAddrInfo);
      m_pResultAddrInfo = nullptr;
   }
   #else
   close(m_ConnectSocket);
   #endif

   m_ConnectSocket = INVALID_SOCKET;

   return true;
}

CTCPClient::~CTCPClient()
{
   if (m_eStatus == CONNECTED)
      Disconnect();


}
