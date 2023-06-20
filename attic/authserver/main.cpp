#include "StarAuthenticationServer.hpp"
#include "StarRandom.hpp"
#include "StarLexicalCast.hpp"
#include "StarLogging.hpp"
#include "StarSignalHandler.hpp"

using namespace Star;
using namespace Star::Auth;

int main(int argc, char **argv) {
  _unused(argc);
  _unused(argv);
  SignalHandler signalHandler;
  try {

    Logger::addSink(make_shared<FileLogSink>("starbound_auth.log", Logger::Debug));

    Thread::prepareThread();
    Auth::initializeKeyLogic();
    StarException::initialize();

    Logger::info("Auth server starting.");

    auto authServer = make_shared<AuthenticationServer>();
    signalHandler.setInterruptHandler([&authServer]() {
        Logger::info("Interrupt received.");
        if (authServer)
          authServer->stop();
      });

    authServer->run();
    authServer.reset();

    Logger::info("Server shutdown gracefully");
  } catch (std::exception const& e) {
    fatalException(e);
  }

  return 0;
}
