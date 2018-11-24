/*
* @file SecureSocket.h
* @brief Abstract class to perform OpenSSL API global operations
*
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
* @date 2017-02-16
*/

#ifdef OPENSSL
#ifndef INCLUDE_ASECURESOCKET_H_
#define INCLUDE_ASECURESOCKET_H_

#ifdef OPENSSL
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "Socket.h"

class ASecureSocket : public ASocket
{
public:
   enum class OpenSSLProtocol
   {
      #ifndef LINUX
      //SSL_V2, // deprecated
      #endif
      //SSL_V3, // deprecated
      TLS_V1,
      SSL_V23, /* There is no SSL protocol version named SSLv23. The SSLv23_method() API
              and its variants choose SSLv2, SSLv3, or TLSv1 for compatibility with the peer. */
      TLS // Standard Protocol as of 11/2018, OpenSSL will choose highest possible TLS standard between peers
   };

   struct SSLSocket
   {
      SSLSocket() :
         m_SockFd(INVALID_SOCKET),
         m_pSSL(nullptr),
         m_pCTXSSL(nullptr),
         m_pMTHDSSL(nullptr)
      {
      }

      // copy constructor and assignment operator are disabled
      SSLSocket(const SSLSocket&) = delete;
      SSLSocket& operator=(const SSLSocket&) = delete;

      // move constructor
      SSLSocket(SSLSocket&& Sockother) :
        m_SockFd(Sockother.m_SockFd),
        m_pSSL(Sockother.m_pSSL),
        m_pCTXSSL(Sockother.m_pCTXSSL),
        m_pMTHDSSL(Sockother.m_pMTHDSSL)
      {
        Sockother.m_SockFd = INVALID_SOCKET;
        Sockother.m_pSSL = nullptr;
        Sockother.m_pCTXSSL = nullptr;
        Sockother.m_pMTHDSSL = nullptr;
      }

      // move assignment operator
      SSLSocket& operator=(SSLSocket&& Sockother)
      {
         if (this != &Sockother)
         {
            m_SockFd = Sockother.m_SockFd;
            m_pSSL = Sockother.m_pSSL;
            m_pCTXSSL = Sockother.m_pCTXSSL;
            m_pMTHDSSL = Sockother.m_pMTHDSSL;

            // reset Sockother
            Sockother.m_SockFd = INVALID_SOCKET;
            Sockother.m_pSSL = nullptr;
            Sockother.m_pCTXSSL = nullptr;
            Sockother.m_pMTHDSSL = nullptr;
         }
         return *this;
      }

      Socket       m_SockFd;
      SSL*         m_pSSL;
      SSL_CTX*     m_pCTXSSL; // SSL Context Structure
      SSL_METHOD*  m_pMTHDSSL; // used to create an SSL_CTX
   };

   /* Please provide your logger thread-safe routine, otherwise, you can turn off
   * error log messages printing by not using the flag ALL_FLAGS or ENABLE_LOG */
   explicit ASecureSocket(const LogFnCallback& oLogger,
                          const OpenSSLProtocol eSSLVersion = OpenSSLProtocol::TLS,
                          const SettingsFlag eSettings = ALL_FLAGS);
   virtual ~ASecureSocket() = 0;

   inline static int GetSSLSocketCount() { return s_iSecureSocketCount; }

   /*
    * For the SSL server:
    * Server's own certificate (mandatory)
    * CA certificate (optional)
    *
    * For the SSL client:
    * CA certificate (mandatory)
    * Client's own certificate (optional)
    */
   inline const std::string& GetSSLCertAuth() { return m_strCAFile; }
   inline void SetSSLCerthAuth(const std::string& strPath) { m_strCAFile = strPath; }

   inline void SetSSLCertFile(const std::string& strPath) { m_strSSLCertFile = strPath; }
   inline const std::string& GetSSLCertFile() const { return m_strSSLCertFile; }

   inline void SetSSLKeyFile(const std::string& strPath) { m_strSSLKeyFile = strPath; }
   inline const std::string& GetSSLKeyFile() const { return m_strSSLKeyFile; }

   //void SetSSLKeyPassword(const std::string& strPwd) { m_strSSLKeyPwd = strPwd; }
   //const std::string& GetSSLKeyPwd() const { return m_strSSLKeyPwd; }

protected:
   // object methods
   void SetUpCtxClient(SSLSocket& Socket);
   void SetUpCtxServer(SSLSocket& Socket);
   //void SetUpCtxCombined(SSLSocket& Socket);

   // class methods
   static void ShutdownSSL(SSLSocket& SSLSocket);
   static const char* GetSSLErrorString(int iErrorCode);
   static int AlwaysTrueCallback(X509_STORE_CTX* pCTX, void* pArg);

   // non-static/object members
   OpenSSLProtocol      m_eOpenSSLProtocol;
   std::string          m_strCAFile;
   std::string          m_strSSLCertFile;
   std::string          m_strSSLKeyFile;
   //std::string          m_strSSLKeyPwd;

   // static/class members
   volatile static int   s_iSecureSocketCount;  // Count of the actual secure socket sessions
   static std::mutex     s_mtxSecureCount;      // mutex used to sync OpenSSL API global operations

private:
   static void InitializeSSL();
   static void DestroySSL();
};

#endif
#endif
