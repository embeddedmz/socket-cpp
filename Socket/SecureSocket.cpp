/**
* @file SecureSocket.cpp
* @brief implementation of the Secure Socket class
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
*/

#ifdef OPENSSL

#include "SecureSocket.h"

#ifndef LINUX
// to avoid link problems in prod/test program
// Update : with the newer versions of OpenSSL, there's no need to include it
//#include <openssl/applink.c>
#endif

// Static members initialization
volatile int    ASecureSocket::s_iSecureSocketCount = 0;
std::mutex      ASecureSocket::s_mtxSecureCount;

/**
* @brief constructor of the Secure Socket
*
* @param oLogger - a callabck to a logger function void(const std::string&)
* @param eSSLVersion - SSL/TLS protocol version
*
*/
ASecureSocket::ASecureSocket(const LogFnCallback& oLogger,
                             const OpenSSLProtocol eSSLVersion,
                             const SettingsFlag eSettings /*= ALL_FLAGS*/) :
   ASocket(oLogger, eSettings),
   m_eOpenSSLProtocol(eSSLVersion)
{
   s_mtxSecureCount.lock();
   if (s_iSecureSocketCount++ == 0)
   {
      // Initialize OpenSSL
      InitializeSSL();
   }
   s_mtxSecureCount.unlock();
}

/**
* @brief destructor of the secure socket object
* It's a pure virtual destructor but an implementation is provided below.
* this to avoid creating a dummy pure virtual method to transform the class
* to an abstract one.
*/
ASecureSocket::~ASecureSocket()
{
   s_mtxSecureCount.lock();
   if (--s_iSecureSocketCount <= 0)
   {
      DestroySSL();
   }
   s_mtxSecureCount.unlock();
}

void ASecureSocket::SetUpCtxClient(SSLSocket& Socket)
{
   switch (m_eOpenSSLProtocol)
   {
      default:
      case OpenSSLProtocol::TLS:
         // Standard Protocol as of 11/2018, OpenSSL will choose highest possible TLS standard between peers
         Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(TLS_client_method());
         break;

      case OpenSSLProtocol::SSL_V23:
         Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(SSLv23_client_method());
         break;

      #ifndef LINUX
      // deprecated in newer versions of OpenSSL
      //case OpenSSLProtocol::SSL_V2:
         //Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(SSLv2_client_method());
         //break;
      #endif

      // deprecated
      /*case OpenSSLProtocol::SSL_V3:
         Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(SSLv3_client_method());
         break;*/

      case OpenSSLProtocol::TLS_V1:
         Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(TLSv1_client_method());
         break;
   }
   Socket.m_pCTXSSL = SSL_CTX_new(Socket.m_pMTHDSSL);
}

void ASecureSocket::SetUpCtxServer(SSLSocket& Socket)
{
   switch (m_eOpenSSLProtocol)
   {
      default:
      case OpenSSLProtocol::TLS:
         // Standard Protocol as of 11/2018, OpenSSL will choose highest possible TLS standard between peers
         Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(TLS_server_method());
         break;

      #ifndef LINUX
      //case OpenSSLProtocol::SSL_V2:
         //Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(SSLv2_server_method());
         //break;
      #endif

      // deprecated
      /*case OpenSSLProtocol::SSL_V3:
         Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(SSLv3_server_method());
         break;*/

      case OpenSSLProtocol::TLS_V1:
         Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(TLSv1_server_method());
         break;

      case OpenSSLProtocol::SSL_V23:
         Socket.m_pMTHDSSL = const_cast<SSL_METHOD*>(SSLv23_server_method());
         break;
   }
   Socket.m_pCTXSSL = SSL_CTX_new(Socket.m_pMTHDSSL);
}

void ASecureSocket::InitializeSSL()
{
   /* Initialize malloc, free, etc for OpenSSL's use. */
   //CRYPTO_malloc_init();

   /* Initialize OpenSSL's SSL libraries: load encryption & hash algorithms for SSL */
   SSL_library_init();

   /* Load the error strings for good error reporting */
   SSL_load_error_strings();

   /* Load BIO error strings. */
   //ERR_load_BIO_strings();

   /* Load all available encryption algorithms. */
   OpenSSL_add_all_algorithms();
}

void ASecureSocket::DestroySSL()
{
   ERR_free_strings();
   EVP_cleanup();
}

void ASecureSocket::ShutdownSSL(SSLSocket& SSLSock)
{
   if (SSLSock.m_pSSL != nullptr)
   {
      /* send the close_notify alert to the peer. */
      SSL_shutdown(SSLSock.m_pSSL); // must be called before SSL_free
      SSL_free(SSLSock.m_pSSL);
      SSL_CTX_free(SSLSock.m_pCTXSSL);

      SSLSock.m_pSSL = nullptr;
   }
}

const char* ASecureSocket::GetSSLErrorString(int iErrorCode)
{
   switch (iErrorCode)
   {
   case SSL_ERROR_NONE:
      return "The TLS/SSL I/O operation completed.";
      break;

   case SSL_ERROR_ZERO_RETURN:
      return "The TLS/SSL connection has been closed.";
      break;

   case SSL_ERROR_WANT_READ:
      return "The read operation did not complete; "
         "the same TLS/SSL I/O function should be called again later.";
      break;

   case SSL_ERROR_WANT_WRITE:
      return "The write operation did not complete; "
      "the same TLS/SSL I/O function should be called again later.";
      break;

   case SSL_ERROR_WANT_CONNECT:
      return "The connect operation did not complete; "
         "the same TLS/SSL I/O function should be called again later.";
      break;

   case SSL_ERROR_WANT_ACCEPT:
      return "The accept operation did not complete; "
      "the same TLS/SSL I/O function should be called again later.";
      break;

   case SSL_ERROR_WANT_X509_LOOKUP:
      return "The operation did not complete because an application callback set"
         " by SSL_CTX_set_client_cert_cb() has asked to be called again. "
         "The TLS/SSL I/O function should be called again later.";
      break;

   case SSL_ERROR_SYSCALL:
      return "Some I/O error occurred. The OpenSSL error queue may contain"
         " more information on the error.";
      break;

   case SSL_ERROR_SSL:
      return "A failure in the SSL library occurred, usually a protocol error. "
         "The OpenSSL error queue contains more information on the error.";
      break;

   default:
      return "Unknown error !";
      break;
   }
}

int ASecureSocket::AlwaysTrueCallback(X509_STORE_CTX* pCTX, void* pArg)
{
   return 1;
}
#endif
