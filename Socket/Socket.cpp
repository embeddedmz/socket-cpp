/**
* @file Socket.cpp
* @brief implementation of the Socket class
* @author Mohamed Amine Mzoughi <mohamed-amine.mzoughi@laposte.net>
*/

#include "Socket.h"

// Static members initialization
volatile int    ASocket::s_iSocketCount = 0;
std::mutex      ASocket::s_mtxCount;

#ifdef WINDOWS
WSADATA         ASocket::s_wsaData;
#endif

/**
* @brief constructor of the Socket
*
* @param Logger - a callabck to a logger function void(const std::string&)
*
*/
ASocket::ASocket(const LogFnCallback& oLogger) :
   m_oLog(oLogger)
{
   s_mtxCount.lock();
   if (s_iSocketCount++ == 0)
   {
      // In windows, this will init the winsock DLL stuff
#ifdef WINDOWS
      int iWinSockInitResult = WSAStartup(MAKEWORD(2, 2), &s_wsaData);
      
      // MAKEWORD(2,2) version 2.2 of Winsock
      if (iWinSockInitResult != 0)
      {
         m_oLog(StringFormat("[TCPClient][Error] WSAStartup failed : %d", iWinSockInitResult));
      }
#endif
   }
   s_mtxCount.unlock();
}

/**
* @brief destructor of the socket object
* It's a pure virtual destructor but an implementation is provided below.
* this to avoid creating a dummy pure virtual method to transform the class
* to an abstract one.
*/
ASocket::~ASocket()
{
   s_mtxCount.lock();
   if (--s_iSocketCount <= 0)
   {
#ifdef WINDOWS
      /* call WSACleanup when done using the Winsock dll */
      WSACleanup();
#endif
   }
   s_mtxCount.unlock();
}

/**
* @brief returns a formatted string
*
* @param [in] strFormat string with one or many format specifiers
* @param [in] parameters to be placed in the format specifiers of strFormat
*
* @retval string formatted string
*/
std::string ASocket::StringFormat(const std::string strFormat, ...)
{
   int n = (static_cast<int>(strFormat.size())) * 2; // Reserve two times as much as the length of the strFormat

   std::unique_ptr<char[]> pFormatted;

   va_list ap;

   while (true)
   {
      pFormatted.reset(new char[n]); // Wrap the plain char array into the unique_ptr
      strcpy(&pFormatted[0], strFormat.c_str());

      va_start(ap, strFormat);
      int iFinaln = vsnprintf(&pFormatted[0], n, strFormat.c_str(), ap);
      va_end(ap);

      if (iFinaln < 0 || iFinaln >= n)
      {
         n += abs(iFinaln - n + 1);
      }
      else
      {
         break;
      }
   }

   return std::string(pFormatted.get());
}
