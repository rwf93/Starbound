#include "StarNetSocket.hpp"

using namespace Star;

int main(int argc, char** argv) {
  class TestSockThread : public Thread {
  public:
    TestSockThread(NetSocketPtr ns) : Thread("TestSockThread"), m_netSocket(ns) {}

    virtual void run() {
      int count = 0;
      coutf("starting\n");

      // Just to test timing out, wait on a packet never sent.
      if (!m_netSocket->waitPacket<ClientConnectPacket>(1000))
        coutf("timed out waiting for bogus packet (expected)\n");

      while (count < 10) {
        m_netSocket->sendPacket(make_shared<ClientConnectPacket>());
        if (m_netSocket->waitPacket<ClientConnectPacket>(1000)) {
          ++count;
          coutf("packet received\n");
        } else {
          coutf("timed out\n");
        }
      }
      coutf("received all packets!\n");
    }

  private:
    NetSocketPtr m_netSocket;
  };

  auto pair = NetSocket::memorySocketPair();
  TestSockThread testThread1(pair.first);
  TestSockThread testThread2(pair.second);

  testThread1.start();
  testThread2.start();

  testThread1.join();
  testThread2.join();

  return 0;
}
