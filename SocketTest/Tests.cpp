#include "gtest/gtest.h"   // Google Test Framework
#include "test_utils.h"    // Helpers for tests

// Test subject (SUT)
#include "TCPClient.h"
#include "TCPServer.h"
#include "TCPSSLServer.h"
#include "TCPSSLClient.h"

#define PRINT_LOG [](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl;  }

// Test parameters
extern bool TCP_TEST_ENABLED;
extern bool SECURE_TCP_TEST_ENABLED;
extern bool HTTP_PROXY_TEST_ENABLED;

extern std::string TCP_SERVER_PORT;
extern std::string SECURE_TCP_SERVER_PORT;

extern std::string CERT_AUTH_FILE;
extern std::string SSL_CERT_FILE;
extern std::string SSL_KEY_FILE;

extern std::string PROXY_SERVER;
extern std::string PROXY_SERVER_FAKE;

extern std::mutex g_mtxConsoleMutex;

namespace
{
// fixture for TCP tests
class TCPTest : public ::testing::Test
{
protected:
   std::unique_ptr<CTCPClient> m_pTCPClient;
   std::unique_ptr<CTCPServer> m_pTCPServer;

   TCPTest() :
      m_pTCPClient(nullptr),
      m_pTCPServer(nullptr)
   {

   }

   virtual ~TCPTest()
   {
   }

   virtual void SetUp()
   {
      m_pTCPClient.reset(new CTCPClient(PRINT_LOG));
   }

   virtual void TearDown()
   {
      if (m_pTCPClient.get() != nullptr)
      {
         m_pTCPClient.reset();
      }
      if (m_pTCPServer.get() != nullptr)
      {
         m_pTCPServer.reset();
      }
   }
};

#ifdef OPENSSL
// fixture for TCP SSL tests
class SSLTCPTest : public ::testing::Test
{
protected:
   std::unique_ptr<CTCPSSLClient> m_pSSLTCPClient;
   std::unique_ptr<CTCPSSLServer> m_pSSLTCPServer;

   SSLTCPTest() :
      m_pSSLTCPClient(nullptr),
      m_pSSLTCPServer(nullptr)
   {
   }

   virtual ~SSLTCPTest()
   {
   }

   virtual void SetUp()
   {
      m_pSSLTCPClient.reset(new CTCPSSLClient(PRINT_LOG));
   }

   virtual void TearDown()
   {
      if (m_pSSLTCPClient.get() != nullptr)
      {
         m_pSSLTCPClient.reset();
      }
      if (m_pSSLTCPServer.get() != nullptr)
      {
         m_pSSLTCPServer.reset();
      }
   }
};
#endif

// Unit tests
/*
TEST_F(TCPTest, TestServer)
{
   const std::string strSendData = "Hello World !";
   char szRcvBuffer[14] = {};
   ASocket::Socket ConnectedClient;

   ASSERT_NO_THROW(m_pTCPServer.reset(new CTCPServer(PRINT_LOG, "6669")));

   ASSERT_TRUE(m_pTCPServer->Listen(ConnectedClient));

   int iCount = 0;
   while (iCount++ < 3)
   {
      EXPECT_GT(m_pTCPServer->Receive(ConnectedClient, szRcvBuffer, 13), 0);
      EXPECT_TRUE(m_pTCPServer->Send(ConnectedClient, strSendData));
      EXPECT_EQ(strSendData, szRcvBuffer);
      bzero(szRcvBuffer, sizeof(szRcvBuffer));
   }
   
   // disconnect
   EXPECT_TRUE(m_pTCPServer->Disconnect(ConnectedClient));
}
*/

/*
TEST_F(TCPTest, TestClient)
{
   const std::string strSendData = "Hello World !";
   char szRcvBuffer[14] = {};

   ASSERT_TRUE(m_pTCPClient->Connect("localhost", "6669"));

   // perform 3 checks
   unsigned uCount = 0;
   while (uCount++ < 3)
   {
      EXPECT_GT(m_pTCPClient->Receive(szRcvBuffer, 13), 0);
      EXPECT_TRUE(m_pTCPClient->Send(strSendData));
      EXPECT_EQ(strSendData, szRcvBuffer);
      memset(szRcvBuffer, '\0', 14);
   }

   // disconnect
   EXPECT_TRUE(m_pTCPClient->Disconnect());
}
*/

TEST_F(TCPTest, TestLoopback)
{
   if (TCP_TEST_ENABLED)
   {
      const std::string strSendData = "Hello World !";
      char szRcvBuffer[14] = {};
      ASocket::Socket ConnectedClient;

      ASSERT_NO_THROW(m_pTCPServer.reset(new CTCPServer(PRINT_LOG, TCP_SERVER_PORT)));

      #ifdef WINDOWS
      // Not always starts a new thread, std::launch::async must be passed to force it.
      std::future<bool> futListen = std::async(std::launch::async,
                                               [&] { return m_pTCPServer->Listen(ConnectedClient); });
      #else
      auto ListenTask = [&] { return m_pTCPServer->Listen(ConnectedClient); };
      std::packaged_task< bool(void) > packageListen { ListenTask };
      std::future<bool> futListen = packageListen.get_future();
      std::thread ListenThread { std::move(packageListen) }; // pack. task is not copyable
      #endif

      // client side
      // send period between 50 and 999 ms
      srand(static_cast<unsigned>(time(nullptr)));
      unsigned iPeriod = 50 + (rand() % (999 - 50));
 
      // give time to let the server object reach the accept instruction.
      SleepMs(500);

      ASSERT_TRUE(m_pTCPClient->Connect("localhost", "6669"));
      #ifdef WINDOWS
      ASSERT_TRUE(futListen.get());
      #else
      /* with std::thread we can't easily get the result of Listen
       * unlike std::async/promise/packaged_task
       */
      ASSERT_TRUE(futListen.get());
      ListenThread.join();
      #endif
      ASSERT_FALSE(ConnectedClient == INVALID_SOCKET);

      // perform 3 checks
      unsigned uCount = 0;
      while (uCount++ < 3)
      {
         // server -> client
         EXPECT_TRUE(m_pTCPServer->Send(ConnectedClient, strSendData));
         EXPECT_GT(m_pTCPClient->Receive(szRcvBuffer, 13), 0);
         EXPECT_EQ(strSendData, szRcvBuffer);
         memset(szRcvBuffer, '\0', 14);

         // client -> server
         EXPECT_TRUE(m_pTCPClient->Send(strSendData));
         EXPECT_GT(m_pTCPServer->Receive(ConnectedClient, szRcvBuffer, 13), 0);
         EXPECT_EQ(strSendData, szRcvBuffer);
         memset(szRcvBuffer, '\0', 14);

         SleepMs(iPeriod);
      }

      // disconnect
      EXPECT_TRUE(m_pTCPClient->Disconnect());
      //EXPECT_TRUE(m_pTCPServer->Disconnect(ConnectedClient)); // OK tested
   }
   else
      std::cout << "TCP tests are disabled !" << std::endl;
}

TEST_F(TCPTest, TestLoopbackWithSelect)
{
   if (TCP_TEST_ENABLED)
   {
      srand(static_cast<unsigned>(time(nullptr)));

      const size_t oneMeg = 1024*1024;
      std::vector<char> OneMbData(oneMeg);
      std::vector<char> RcvBuffer(oneMeg);
      std::generate (OneMbData.begin(), OneMbData.end(), []{ return (std::rand() % 256); });

      ASocket::Socket ConnectedClient;

      ASSERT_NO_THROW(m_pTCPServer.reset(new CTCPServer(PRINT_LOG, TCP_SERVER_PORT)));

      #ifdef WINDOWS
      // Not always starts a new thread, std::launch::async must be passed to force it.
      std::future<bool> futListen = std::async(std::launch::async,
                                               [&]
                                               {
                                                  return m_pTCPServer->Listen(ConnectedClient,
                                                                              1000);
                                               });
      #else
      auto ListenTask = [&] { return m_pTCPServer->Listen(ConnectedClient, 1000); }; // 1 sec max wait
      std::packaged_task< bool(void) > packageListen { ListenTask };
      std::future<bool> futListen = packageListen.get_future();
      std::thread ListenThread { std::move(packageListen) }; // pack. task is not copyable
      #endif

      // client side
      // give time to let the server object reach the accept instruction.
      SleepMs(500);

      ASSERT_TRUE(m_pTCPClient->Connect("localhost", "6669"));
      #ifdef WINDOWS
      ASSERT_TRUE(futListen.get());
      #else
      /* with std::thread we can't easily get the result of Listen
       * unlike std::async/promise/packaged_task
       */
      ASSERT_TRUE(futListen.get());
      ListenThread.join();
      #endif
      ASSERT_FALSE(ConnectedClient == INVALID_SOCKET);

      // server -> client
      EXPECT_TRUE(m_pTCPServer->Send(ConnectedClient, OneMbData));
      EXPECT_GT(m_pTCPClient->Receive(RcvBuffer.data(), oneMeg), 0);
      EXPECT_TRUE(std::equal(OneMbData.begin(), OneMbData.end(), RcvBuffer.begin()));

      std::fill(RcvBuffer.begin(), RcvBuffer.end(), 0);

      // client -> server
      EXPECT_TRUE(m_pTCPClient->Send(OneMbData));
      EXPECT_GT(m_pTCPServer->Receive(ConnectedClient, RcvBuffer.data(), oneMeg), 0);
      EXPECT_TRUE(std::equal(OneMbData.begin(), OneMbData.end(), RcvBuffer.begin()));

      // disconnect
      EXPECT_TRUE(m_pTCPClient->Disconnect());
      //EXPECT_TRUE(m_pTCPServer->Disconnect(ConnectedClient)); // OK tested
   }
   else
      std::cout << "TCP tests are disabled !" << std::endl;
}

TEST_F(TCPTest, TestLoopbackWithSelectFailure)
{
   if (TCP_TEST_ENABLED)
   {
      ASocket::Socket ConnectedClient;

      ASSERT_NO_THROW(m_pTCPServer.reset(new CTCPServer(PRINT_LOG, TCP_SERVER_PORT)));

      ASSERT_FALSE(m_pTCPServer->Listen(ConnectedClient, 3000)); // 3 sec max wait

      ASSERT_TRUE(ConnectedClient == INVALID_SOCKET);
   }
   else
      std::cout << "TCP tests are disabled !" << std::endl;
}

#ifdef OPENSSL
/*
TEST_F(SSLTCPTest, TestServer)
{
   const std::string strSendData = "Hello World !";
   char szRcvBuffer[14] = {};
   ASecureSocket::SSLSocket ConnectedClient;

   ASSERT_NO_THROW(m_pSSLTCPServer.reset(new CTCPSSLServer(PRINT_LOG, "4242")));
   m_pSSLTCPServer->SetSSLCertFile(SSL_CERT_FILE);
   m_pSSLTCPServer->SetSSLKeyFile(SSL_KEY_FILE);

   ASSERT_TRUE(m_pSSLTCPServer->Listen(ConnectedClient));

   int iCount = 0;
   while (iCount++ < 3)
   {
      EXPECT_GT(m_pSSLTCPServer->Receive(ConnectedClient, szRcvBuffer, 13), 0);
      EXPECT_TRUE(m_pSSLTCPServer->Send(ConnectedClient, strSendData));
      EXPECT_EQ(strSendData, szRcvBuffer);
      memset(szRcvBuffer, '\0', sizeof(szRcvBuffer));
   }

   // disconnect
   EXPECT_TRUE(m_pSSLTCPServer->Disconnect(ConnectedClient));
}
*/

TEST_F(SSLTCPTest, TestLoopback)
{
   if (SECURE_TCP_TEST_ENABLED)
   {
      srand(static_cast<unsigned>(time(nullptr)));

      const size_t oneMeg = 1024*1024;
      std::vector<char> OneMbData(oneMeg);
      std::vector<char> RcvBuffer(oneMeg);
      std::generate (OneMbData.begin(), OneMbData.end(), []{ return (std::rand() % 256); });

      ASecureSocket::SSLSocket ConnectedClient;

      ASSERT_NO_THROW(m_pSSLTCPServer.reset(new CTCPSSLServer(PRINT_LOG, SECURE_TCP_SERVER_PORT)));

      m_pSSLTCPServer->SetSSLCertFile(SSL_CERT_FILE);
      m_pSSLTCPServer->SetSSLKeyFile(SSL_KEY_FILE);
      //m_pSSLTCPServer->SetSSLCerthAuth(CERT_AUTH_FILE); // not mandatory

#ifdef WINDOWS
      // Not always starts a new thread, std::launch::async must be passed to force it.
      std::future<bool> futListen = std::async(std::launch::async,
      [&]() -> bool
      {
         // give time to let the server object reach the accept instruction.
         SleepMs(1000);

         //m_pSSLTCPClient->SetSSLCerthAuth(CERT_AUTH_FILE); // not mandatory
         //m_pSSLTCPClient->SetSSLKeyFile(SSL_KEY_FILE); // not mandatory
         return m_pSSLTCPClient->Connect("localhost", SECURE_TCP_SERVER_PORT);
      });
#else
      auto ConnectTask = [&]() -> bool
      {
         // give time to let the server object reach the accept instruction.
         std::cout << "** Connect task : delay 2 seconds\n";
         SleepMs(2000);
         std::cout << "** Connect task : begin connect\n";

         //m_pSSLTCPClient->SetSSLKeyFile(SSL_KEY_FILE); // not mandatory
         //m_pSSLTCPClient->SetSSLCerthAuth(CERT_AUTH_FILE); // not mandatory

         bool bRet = m_pSSLTCPClient->Connect("localhost", "4242");
         std::cout << "** Connect task : end connect\n";
         return bRet;
      };
      std::packaged_task< bool(void) > packageConnect { ConnectTask };
      std::future<bool> futConnect = packageConnect.get_future();
      std::thread ConnectThread { std::move(packageConnect) }; // pack. task is not copyable
#endif

      m_pSSLTCPServer->Listen(ConnectedClient);

#ifdef WINDOWS
      ASSERT_TRUE(futListen.get());
#else
      /* with std::thread we can't easily get the result of Listen
       * contrary to std::async/promise/packaged_task
       */
      ASSERT_TRUE(futConnect.get());
      ConnectThread.join();
#endif

      ASSERT_FALSE(ConnectedClient.m_pSSL == nullptr);
      ASSERT_FALSE(ConnectedClient.m_pCTXSSL == nullptr);
      ASSERT_FALSE(ConnectedClient.m_SockFd == INVALID_SOCKET);
      
      // server -> client
      EXPECT_TRUE(m_pSSLTCPServer->Send(ConnectedClient, OneMbData));
      EXPECT_GT(m_pSSLTCPClient->Receive(RcvBuffer.data(), oneMeg), 0);
      EXPECT_TRUE(std::equal(OneMbData.begin(), OneMbData.end(), RcvBuffer.begin()));

      std::fill(RcvBuffer.begin(), RcvBuffer.end(), 0);

      // client -> server
      EXPECT_TRUE(m_pSSLTCPClient->Send(OneMbData));
      EXPECT_GT(m_pSSLTCPServer->Receive(ConnectedClient, RcvBuffer.data(), oneMeg), 0);
      EXPECT_TRUE(std::equal(OneMbData.begin(), OneMbData.end(), RcvBuffer.begin()));

      // disconnect
      EXPECT_TRUE(m_pSSLTCPClient->Disconnect());
      EXPECT_TRUE(m_pSSLTCPServer->Disconnect(ConnectedClient));
   }
   else
      std::cout << "SECURE TCP tests are disabled !" << std::endl;
}

#endif

} // namespace

int main(int argc, char **argv)
{
   if (argc > 1 && GlobalTestInit(argv[1])) // loading test parameters from the INI file...
   {
      ::testing::InitGoogleTest(&argc, argv);
      int iTestResults = RUN_ALL_TESTS();

      ::GlobalTestCleanUp();

      return iTestResults;
   }
   else
   {
      std::cerr << "[Error] Encountered an error while loading test parameters from provided INI file !" << std::endl;
      return 1;
   }
}
