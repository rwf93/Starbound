#include "StarString.hpp"
#include "StarThread.hpp"
#include "StarRandom.hpp"
#include "StarAuthenticationKey.hpp"
#include "StarAuthenticationService.hpp"
#include "StarAuthenticationDatabase.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

using namespace std;
using namespace Star;

int usage(int rc) {
  std::cout
  << "USAGE: " << std::endl
      << " keygen rootkey <rootPrivateFile> <rootPublicFile>" << std::endl
      << " keygen authkey <rootPrivateFile> <authPrivateFile>" << std::endl
      << " keygen hashpass <username> <password>" << std::endl
      << " keygen insertuser <username> <passhash>" << std::endl
      << " keygen enableuser <username>" << std::endl
      << " keygen disableuser <username>" << std::endl;
  return rc;
}

int main(int argc, char ** argv) {
  try {
    Random::randu32();
    StringList args;
    for (int i = 1; i < argc; i++)
      args.append(argv[i]);

    String connectionString = Variant::parseJson(File::readFileString("connectionstring.config")).getString("connectionString");

    if (argc == 4 && !String("rootkey").compare(args[0])) {
      Star::Auth::Key rootKey(true);
      std::ofstream oFile;
      oFile.open(args[1].utf8());
      oFile << rootKey.privateKey();
      oFile.close();
      oFile.open(args[2].utf8());
      oFile << rootKey.publicKey();
      oFile.close();
      return 0;
    } else if (argc == 4 && !String("authkey").compare(args[0])) {
      std::ifstream iFile(args[1].utf8());
      String rootPrivateKey;
      iFile >> rootPrivateKey;
      iFile.close();
      std::ofstream oFile(args[2].utf8());
      oFile << Star::Auth::AuthenticationService::generateAuthenticationConfig(rootPrivateKey, Thread::currentTime() - 1LL * 25LL * 60LL * 60LL * 1000LL, Thread::currentTime() + 38LL * 24LL * 60LL * 60LL * 1000LL).repr(0, true);
      oFile.close();
      return 0;
    } else if (argc == 4 && !String("hashpass").compare(args[0])) {
      cout << Auth::preHashPassword(args[1], args[2]);
    } else if (argc == 4 && !String("insertuser").compare(args[0])) {
      Auth::AuthenticationDatabase database(connectionString);
      database.setUserRecord(args[1], args[2], true);
      cout << "Enabled user: " << args[1] << "\n";
    } else if (argc == 3 && !String("disableuser").compare(args[0])) {
      Auth::AuthenticationDatabase database(connectionString);
      if (database.activateUserRecord(args[1], false))
        cout << "Disabled user: " << args[1] << "\n";
      else
        cout << "No such user\n";
    } else if (argc == 3 && !String("enableuser").compare(args[0])) {
      Auth::AuthenticationDatabase database(connectionString);
      if (database.activateUserRecord(args[1], true))
        cout << "Enabled user: " << args[1] << "\n";
      else
        cout << "No such user\n";
    } else {
      return usage(-1);
    }
  } catch (std::exception const& e) {
    cerr << e.what() << "\n";
    return -1;
  }
}
