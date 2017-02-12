/*
* @file Socket.h
* @brief Abstract class to perform API global operations
*
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
* @date 2017-02-10
*/

#ifndef INCLUDE_ASOCKET_H_
#define INCLUDE_ASOCKET_H_

#include <cstdio>         // snprintf
#include <exception>
#include <functional>
#include <mutex>
#include <stdarg.h>       // va_start, etc.
#include <stdexcept>

#ifdef WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment(lib,"WS2_32.lib")
#endif

class ASocket
{
public:
   // Public definitions
   //typedef std::function<int(void*, double, double, double, double)> ProgressFnCallback;
   typedef std::function<void(const std::string&)>                   LogFnCallback;

   // socket file descriptor id
   #ifdef WINDOWS
   typedef SOCKET Socket;
   #else

   #endif

   enum SettingsFlag
   {
      NO_FLAGS = 0x00,
      ENABLE_LOG = 0x01,
      // ENABLE_SSL = 0x02,  // OpenSSL
      ALL_FLAGS = 0xFF
   };

   /* Please provide your logger thread-safe routine, otherwise, you can turn off
   * error log messages printing by not using the flag ALL_FLAGS or ENABLE_LOG */
   explicit ASocket(LogFnCallback oLogger);
   virtual ~ASocket() = 0;

   inline static int GetSocketCount() { return s_iSocketCount; }

protected:
   // String Helpers
   static std::string StringFormat(const std::string strFormat, ...);

   // Log printer callback
   /*mutable*/ LogFnCallback         m_oLog;

   volatile static int   s_iSocketCount;  // Count of the actual socket sessions
   static std::mutex     s_mtxCount;      // mutex used to sync API global operations

   #ifdef WINDOWS
   static WSADATA s_wsaData;
   #endif

};

class EResolveError : public std::logic_error
{
public:
   explicit EResolveError(const std::string &strMsg) : std::logic_error(strMsg) {}
};

#endif
