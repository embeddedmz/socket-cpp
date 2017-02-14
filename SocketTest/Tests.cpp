#include "gtest/gtest.h"   // Google Test Framework
#include "test_utils.h"    // Helpers for tests

// Test subject (SUT)
#include "TCPClient.h"
#include "TCPServer.h"

#define PRINT_LOG [](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl;  }

//extern std::mutex g_mtxConsoleMutex;

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
   const std::string strSendData = "Hello World !";
   char szRcvBuffer[14] = {};
   ASocket::Socket ConnectedClient;

   ASSERT_NO_THROW(m_pTCPServer.reset(new CTCPServer(PRINT_LOG, "6669")));

   #ifdef WINDOWS
   std::future<bool> futListen = std::async([&] { return m_pTCPServer->Listen(ConnectedClient); });
   #else
   auto ListenTask = [&] { return m_pTCPServer->Listen(ConnectedClient); };
   std::thread ListenThread(ListenTask);
   #endif

   // client side
   // send period between 50 and 999 ms
   srand(static_cast<unsigned>(time(nullptr)));
   unsigned iPeriod = 50 + (rand() % (999 - 50));
 
   // give time to let the server object reach the accept instruction.
   #ifdef LINUX
   usleep(500000);
   #else
   Sleep(500);
   #endif

   ASSERT_TRUE(m_pTCPClient->Connect("localhost", "6669"));
   #ifdef WINDOWS
   ASSERT_TRUE(futListen.get());
   #else
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
      bzero(szRcvBuffer, sizeof(szRcvBuffer));

      // client -> server
      EXPECT_TRUE(m_pTCPClient->Send(strSendData));
      EXPECT_GT(m_pTCPServer->Receive(ConnectedClient, szRcvBuffer, 13), 0);
      EXPECT_EQ(strSendData, szRcvBuffer);
      memset(szRcvBuffer, '\0', 14);
      
      #ifdef LINUX
      usleep(iPeriod*1000);
      #else
      Sleep(iPeriod);
      #endif
   }

   // disconnect
   EXPECT_TRUE(m_pTCPClient->Disconnect());
   //EXPECT_TRUE(m_pTCPServer->Disconnect(ConnectedClient)); // OK tested
}

} // namespace

int main(int argc, char **argv)
{
   //if (argc > 1 && GlobalTestInit(argv[1])) // loading test parameters from the INI file...
   {
      ::testing::InitGoogleTest(&argc, argv);
      int iTestResults = RUN_ALL_TESTS();

      ::GlobalTestCleanUp();

      return iTestResults;
   }
   /*else
   {
      std::cerr << "[ERROR] Encountered an error while loading test parameters from provided INI file !" << std::endl;
      return 1;
   }*/
}
