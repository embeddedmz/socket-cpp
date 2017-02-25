#include "test_utils.h"

// Test configuration constants (to be loaded from an INI file)
bool TCP_TEST_ENABLED;
bool SECURE_TCP_TEST_ENABLED;
bool HTTP_PROXY_TEST_ENABLED;

// TCP
std::string TCP_SERVER_PORT;

// TCP over SSL/TLS
std::string SECURE_TCP_SERVER_PORT;
std::string CERT_AUTH_FILE;
std::string SSL_CERT_FILE;
std::string SSL_KEY_FILE;

// PROXY TUNNELING
std::string PROXY_SERVER;
std::string PROXY_SERVER_FAKE;

// STDOUT MUTEX
std::mutex g_mtxConsoleMutex;

bool GlobalTestInit(const std::string& strConfFile)
{
   CSimpleIniA ini;
   ini.LoadFile(strConfFile.c_str());

   std::string strTmp;

   strTmp = ini.GetValue("tests", "tcp", "");
   std::transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::toupper);
   TCP_TEST_ENABLED = (strTmp == "YES") ? true : false;

   strTmp = ini.GetValue("tests", "tcp-ssl", "");
   std::transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::toupper);
   SECURE_TCP_TEST_ENABLED = (strTmp == "YES") ? true : false;

   strTmp = ini.GetValue("tests", "http-proxy", "");
   std::transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::toupper);
   HTTP_PROXY_TEST_ENABLED = (strTmp == "YES") ? true : false;

   TCP_SERVER_PORT = ini.GetValue("tcp", "server_port", "");

   // build must be generated with the macro OPENSSL to enable SSL tests
   SECURE_TCP_SERVER_PORT = ini.GetValue("tcp-ssl", "server_port", "");
   CERT_AUTH_FILE = ini.GetValue("tcp-ssl", "ca_file", "");
   SSL_CERT_FILE = ini.GetValue("tcp-ssl", "ssl_cert_file", "");
   SSL_KEY_FILE = ini.GetValue("tcp-ssl", "ssl_key_file", "");

   PROXY_SERVER = ini.GetValue("http-proxy", "host", "");
   PROXY_SERVER_FAKE = ini.GetValue("http-proxy", "host_invalid", "");

   if (TCP_TEST_ENABLED && TCP_SERVER_PORT.empty())
   {
      std::clog << "[ERROR] Check your INI file TCP's parameter(s)." << std::endl;
      return false;
   }

   /* according to my tests :
    *
    * For Server:
    * Cert and Key files mandatory.
    * 
    * For Client : nothing is needed
    */
   if (SECURE_TCP_TEST_ENABLED &&
      (/*CERT_AUTH_FILE.empty() || */SSL_CERT_FILE.empty() || SSL_KEY_FILE.empty()))
   {
      std::clog << "[ERROR] Check your INI file SSL's parameter(s)." << std::endl;
      return false;
   }

   if (HTTP_PROXY_TEST_ENABLED && (PROXY_SERVER.empty() || PROXY_SERVER_FAKE.empty()))
   {
      std::clog << "[ERROR] Check your INI file proxy's parameter(s)." << std::endl;
      return false;
   }

   return true;
}

void GlobalTestCleanUp(void)
{

   return;
}

void TimeStampTest(std::ostringstream& ssTimestamp)
{
   time_t tRawTime;
   tm * tmTimeInfo;
   time(&tRawTime);
   tmTimeInfo = localtime(&tRawTime);

   ssTimestamp << (tmTimeInfo->tm_year) + 1900
      << "/" << tmTimeInfo->tm_mon + 1 << "/" << tmTimeInfo->tm_mday << " at "
      << tmTimeInfo->tm_hour << ":" << tmTimeInfo->tm_min << ":" << tmTimeInfo->tm_sec;
}

// for uplaod prog. callback, just use DL one and inverse download parameters with upload ones...
int TestUPProgressCallback(void* ptr, double dTotalToDownload, double dNowDownloaded, double dTotalToUpload, double dNowUploaded)
{
   return TestDLProgressCallback(ptr, dTotalToUpload, dNowUploaded, dTotalToDownload, dNowDownloaded);
}
int TestDLProgressCallback(void* ptr, double dTotalToDownload, double dNowDownloaded, double dTotalToUpload, double dNowUploaded)
{
   // ensure that the file to be downloaded is not empty
   // because that would cause a division by zero error later on
   if (dTotalToDownload <= 0.0)
      return 0;

   // how wide you want the progress meter to be
   const int iTotalDots = 20;
   double dFractionDownloaded = dNowDownloaded / dTotalToDownload;
   // part of the progressmeter that's already "full"
   int iDots = static_cast<int>(round(dFractionDownloaded * iTotalDots));

   // create the "meter"
   int iDot = 0;
   std::cout << static_cast<unsigned>(dFractionDownloaded * 100) << "% [";

   // part  that's full already
   for (; iDot < iDots; iDot++)
      std::cout << "=";

   // remaining part (spaces)
   for (; iDot < iTotalDots; iDot++)
      std::cout << " ";

   // and back to line begin - do not forget the fflush to avoid output buffering problems!
   std::cout << "]           \r" << std::flush;

   // if you don't return 0, the transfer will be aborted - see the documentation
   return 0;
}

long GetGMTOffset()
{
   time_t now = time(nullptr);

   struct tm gm = *gmtime(&now);
   time_t gmt = mktime(&gm);

   struct tm loc = *localtime(&now);
   time_t local = mktime(&loc);

   return static_cast<long>(difftime(local, gmt));
}

bool GetFileTime(const char* const & pszFilePath, time_t& tLastModificationTime)
{
   FILE* pFile = fopen(pszFilePath, "rb");

   if (pFile != nullptr)
   {
      struct stat file_info;

#ifndef LINUX
      if (fstat(_fileno(pFile), &file_info) == 0)
#else
      if (fstat(fileno(pFile), &file_info) == 0)
#endif
      {
         tLastModificationTime = file_info.st_mtime;
         return true;
      }
   }

   return false;
}

void SleepMs(int iMilisec)
{
   std::this_thread::sleep_for(std::chrono::milliseconds(iMilisec));
}
