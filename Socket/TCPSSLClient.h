/*
* @file TCPSSLClient.h
* @brief wrapper for TCP SSL client
*
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
* @date 2017-02-16
*/

#ifdef OPENSSL
#ifndef INCLUDE_TCPSSLCLIENT_H_
#define INCLUDE_TCPSSLCLIENT_H_

#include "SecureSocket.h"
#include "TCPClient.h"

class CTCPSSLClient : public ASecureSocket
{
public:
   explicit CTCPSSLClient(const LogFnCallback oLogger,
                          const OpenSSLProtocol eSSLVersion = OpenSSLProtocol::TLS,
                          const SettingsFlag eSettings = ALL_FLAGS);
   ~CTCPSSLClient() override;

   CTCPSSLClient(const CTCPSSLClient&) = delete;
   CTCPSSLClient& operator=(const CTCPSSLClient&) = delete;

   /* connect to a TCP SSL server */
   bool Connect(const std::string& strServer, const std::string& strPort);

   bool SetRcvTimeout(unsigned int timeout);
   bool SetRcvTimeout(struct timeval timeout);
   bool SetSndTimeout(unsigned int timeout);
   bool SetSndTimeout(struct timeval timeout);

   /* disconnect from the SSL TCP server */
   bool Disconnect();

   /* send data to a TCP SSL server */
   bool Send(const char* pData, const size_t uSize) const;
   bool Send(const std::string& strData) const;
   bool Send(const std::vector<char>& Data) const;

   /* receive data from a TCP SSL server */
   bool HasPending();
   int PendingBytes();

   int Receive(char* pData, const size_t uSize, bool bReadFully = true) const;

protected:
   CTCPClient  m_TCPClient;
   SSLSocket   m_SSLConnectSocket;

};

#endif
#endif
