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

TEST_F(TCPTest, TestServerClient)
{
   // server side
   char szRcvBuffer[14] = {};
   ASocket::Socket ConnectedClient;

   ASSERT_NO_THROW(m_pTCPServer.reset(new CTCPServer(PRINT_LOG, "localhost", "6669")));

   std::future<bool> futListen = std::async([&] { return m_pTCPServer->Listen(ConnectedClient); });

   // client side
   // send period between 50 and 999 ms
   srand(static_cast<unsigned>(time(nullptr)));
   unsigned iPeriod = 50 + (rand() % (999 - 50));

   // data to send to the server by the client
   const std::string strSendData = "Hello World !";
   
   Sleep(500); // to let the server object reach the accept instruction.
   ASSERT_TRUE(m_pTCPClient->Connect("localhost", "6669"));
   ASSERT_TRUE(futListen.get());

   // perform 3 checks
   unsigned uCount = 0;
   while (uCount++ < 3)
   {
      EXPECT_TRUE(m_pTCPClient->SendData(strSendData));
      EXPECT_GT(m_pTCPServer->Receive(ConnectedClient, szRcvBuffer, 13), 0);
      EXPECT_EQ(strSendData, szRcvBuffer);

      memset(szRcvBuffer, '\0', 14);

      Sleep(iPeriod);
   }

   // disconnect
   //EXPECT_TRUE(m_pTCPClient->Disconnect()); // OK
   EXPECT_TRUE(m_pTCPServer->Disconnect(ConnectedClient));
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
