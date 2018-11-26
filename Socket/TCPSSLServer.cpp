/**
* @file TCPSSLServer.cpp
* @brief implementation of the TCP SSL server class
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
*/

#ifdef OPENSSL
#include "TCPSSLServer.h"

CTCPSSLServer::CTCPSSLServer(const LogFnCallback oLogger,
                             const std::string& strPort,
                             const OpenSSLProtocol eSSLVersion,
                             const SettingsFlag eSettings /*= ALL_FLAGS*/)
                             /*throw (EResolveError)*/ :
   ASecureSocket(oLogger, eSSLVersion, eSettings),
   m_TCPServer(oLogger, strPort, eSettings)
{

}

bool CTCPSSLServer::SetRcvTimeout(SSLSocket& ClientSocket, unsigned int msec_timeout){
   return m_TCPServer.SetRcvTimeout(ClientSocket.m_SockFd, msec_timeout);
}

bool CTCPSSLServer::SetRcvTimeout(SSLSocket& ClientSocket, struct timeval timeout){
   return m_TCPServer.SetRcvTimeout(ClientSocket.m_SockFd, timeout);
}

bool CTCPSSLServer::SetSndTimeout(SSLSocket& ClientSocket, unsigned int msec_timeout){
   return m_TCPServer.SetSndTimeout(ClientSocket.m_SockFd, msec_timeout);
}

bool CTCPSSLServer::SetSndTimeout(SSLSocket& ClientSocket, struct timeval timeout){
   return m_TCPServer.SetSndTimeout(ClientSocket.m_SockFd, timeout);
}

// returns the socket of the accepted client
bool CTCPSSLServer::Listen(SSLSocket& ClientSocket, size_t msec /*= ACCEPT_WAIT_INF_DELAY*/)
{
   if (m_TCPServer.Listen(ClientSocket.m_SockFd, msec))
   {
      SetUpCtxServer(ClientSocket);

      if (ClientSocket.m_pCTXSSL == nullptr)
      {
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog("[TCPSSLServer][Error] SSL CTX failed.");
         //ERR_print_errors_fp(stdout);
         return false;
      }

      //SSL_CTX_set_options(ClientSocket.m_pCTXSSL, SSL_OP_SINGLE_DH_USE);
      //SSL_CTX_set_cert_verify_callback(ClientSocket.m_pCTXSSL, AlwaysTrueCallback, nullptr);

      /* Load server certificate into the SSL context. */
      if (!m_strSSLCertFile.empty())
      {
         if (SSL_CTX_use_certificate_file(ClientSocket.m_pCTXSSL,
            m_strSSLCertFile.c_str(), SSL_FILETYPE_PEM) <= 0)
         {
            if (m_eSettingsFlags & ENABLE_LOG)
               m_oLog("[TCPSSLServer][Error] Loading cert file failed.");
            //ERR_print_errors_fp(stdout);
            return false;
         }
      }
      /* Load trusted CA file. */
      if (!m_strCAFile.empty())
      {
         if (!SSL_CTX_load_verify_locations(ClientSocket.m_pCTXSSL, m_strCAFile.c_str(), nullptr))
         {
            if (m_eSettingsFlags & ENABLE_LOG)
               m_oLog("[TCPSSLServer][Error] Loading CA file failed.");

            return false;
         }
         /* Set to require peer (client) certificate verification. */
         //SSL_CTX_set_verify(m_SSLConnectSocket.m_pCTXSSL, SSL_VERIFY_PEER, VerifyCallback);
         /* Set the verification depth to 1 */
         SSL_CTX_set_verify_depth(ClientSocket.m_pCTXSSL, 1);
      }
      /* Load the server private-key into the SSL context. */
      if (!m_strSSLKeyFile.empty())
      {
         if (SSL_CTX_use_PrivateKey_file(ClientSocket.m_pCTXSSL,
            m_strSSLKeyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
         {
            if (m_eSettingsFlags & ENABLE_LOG)
               m_oLog("[TCPSSLServer][Error] Loading key file failed.");
            //ERR_print_errors_fp(stdout);
            return false;
         }

         // verify private key
         /*if (!SSL_CTX_check_private_key(ClientSocket.m_pCTXSSL))
         {
            if (m_eSettingsFlags & ENABLE_LOG)
               m_oLog("[TCPSSLServer][Error] Private key does not match the public certificate.");

            return false;
         }*/
      }

      ClientSocket.m_pSSL = SSL_new(ClientSocket.m_pCTXSSL);
      // set the socket directly into the SSL structure or we can use a BIO structure
      SSL_set_fd(ClientSocket.m_pSSL, ClientSocket.m_SockFd);

      /* wait for a TLS/SSL client to initiate a TLS/SSL handshake */
      int iSSLErr = SSL_accept(ClientSocket.m_pSSL);
      if (iSSLErr <= 0)
      {
         //Error occurred, log and close down ssl
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog(StringFormat("[TCPSSLServer][Error] accept failed. (Error=%d | %s)",
                             iSSLErr, GetSSLErrorString(SSL_get_error(ClientSocket.m_pSSL, iSSLErr))));

         //if (iSSLErr < 0)
         // under Windows it creates problems
            #ifdef LINUX
            ERR_print_errors_fp(stdout);
            #endif

         ShutdownSSL(ClientSocket);

         return false;
      }

      /* The TLS/SSL handshake is successfully completed and  a TLS/SSL connection
       * has been established. Now all reads and writes must use SSL. */
      // peer_cert = SSL_get_peer_certificate(ClientSocket.m_pSSL);
      return true;
   }

   if (m_eSettingsFlags & ENABLE_LOG)
      m_oLog("[TCPSSLServer][Error] Unable to accept an incoming TCP connection with a client.");

   return false;
}

bool CTCPSSLServer::HasPending(const SSLSocket& ClientSocket)
{
   int pend;

   pend = SSL_has_pending(ClientSocket.m_pSSL);

   return pend == 1;
}

int CTCPSSLServer::PendingBytes(const SSLSocket& ClientSocket)
{
   int nPend;

   nPend = SSL_pending(ClientSocket.m_pSSL);

   return nPend;
}

/* When an SSL_read() operation has to be repeated because of SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE,
 * it must be repeated with the same arguments.*/
int CTCPSSLServer::Receive(const SSLSocket& ClientSocket,
                           char* pData,
                           const size_t uSize,
                           bool bReadFully /*= true*/) const
{
   int total = 0;
   do
   {
      int nRecvd = SSL_read(ClientSocket.m_pSSL, pData + total, uSize - total);

      if (nRecvd <= 0)
      {
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog(StringFormat("[TCPSSLServer][Error] SSL_read failed (Error=%d | %s)",
                  nRecvd, GetSSLErrorString(SSL_get_error(ClientSocket.m_pSSL, nRecvd))));

          //ERR_print_errors_fp(stdout);

          break;
      }

      total += nRecvd;

   } while(bReadFully && (total < uSize));

   return total;
}

/* When an SSL_write() operation has to be repeated because of SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE,
 * it must be repeated with the same arguments.
 * When calling SSL_write() with uSize=0 bytes to be sent the behaviour is undefined. */
bool CTCPSSLServer::Send(const SSLSocket& ClientSocket, const char* pData, const size_t uSize) const
{
   int total = 0;
   do
   {
      int nSent;

      nSent = SSL_write(ClientSocket.m_pSSL, pData + total, uSize - total);

      if (nSent <= 0)
      {
         if (m_eSettingsFlags & ENABLE_LOG)
            m_oLog(StringFormat("[TCPSSLServer][Error] SSL_write failed (Error=%d | %s).",
            nSent, GetSSLErrorString(SSL_get_error(ClientSocket.m_pSSL, nSent))));

         return false;
      }
      total += nSent;
   } while (total < uSize);

   return true;
}

bool CTCPSSLServer::Send(const SSLSocket& ClientSocket, const std::string& strData) const
{
   bool ret;

   ret = Send(ClientSocket, strData.c_str(), strData.length());

   return ret;
}

bool CTCPSSLServer::Send(const SSLSocket& ClientSocket, const std::vector<char>& Data) const
{
   bool ret;

   ret = Send(ClientSocket, Data.data(), Data.size());

   return ret;
}

bool CTCPSSLServer::Disconnect(SSLSocket& ClientSocket) const
{
   // send close_notify message to notify peer of the SSL closure.
   ShutdownSSL(ClientSocket);

   return m_TCPServer.Disconnect(ClientSocket.m_SockFd);
}

CTCPSSLServer::~CTCPSSLServer()
{

}
#endif
