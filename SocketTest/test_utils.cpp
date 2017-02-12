#include "test_utils.h"

// Test configuration constants (to be loaded from an INI file)
//bool HTTP_PROXY_TEST_ENABLED;

//std::string PROXY_SERVER;
//std::string PROXY_SERVER_FAKE;

//std::mutex g_mtxConsoleMutex;

bool GlobalTestInit(const std::string& strConfFile)
{
   //CSimpleIniA ini;
   //ini.LoadFile(strConfFile.c_str());

   /*strTmp = ini.GetValue("tests", "http-proxy", "");
   std::transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::toupper);
   HTTP_PROXY_TEST_ENABLED = (strTmp == "YES") ? true : false;*/

   //PROXY_SERVER = ini.GetValue("http-proxy", "host", "");
   //PROXY_SERVER_FAKE = ini.GetValue("http-proxy", "host_invalid", "");

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
