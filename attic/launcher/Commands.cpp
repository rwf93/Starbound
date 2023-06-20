#include "Commands.hpp"

#include <QProcess>
#include <QFileInfo>
#include <QMessageBox>

#if defined Q_OS_WIN
#include <windows.h>
#endif

void startClient(QStringList arguments) {
  auto startStarbound = [](QString const& program, QStringList const& arguments, QString const& workingDirectory = QString()) {
    if (!QProcess::startDetached(program, arguments, workingDirectory))
      QMessageBox::warning(nullptr, "", "Error starting starbound process");
  };

#if defined Q_OS_WIN
  SYSTEM_INFO systemInfo;
  GetNativeSystemInfo(&systemInfo);

  if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    startStarbound("..\\win64\\starbound.exe", arguments, "..\\win64\\");
  else
    startStarbound(".\\starbound.exe", arguments);

#elif defined Q_OS_MACX
  if (!arguments.empty())
    arguments.prepend("--args");
  arguments.prepend("./Starbound.app");

  startStarbound("/usr/bin/open", arguments);

#else
  startStarbound("./starbound", arguments);

#endif
}

void startServer() {
  auto startStarboundServer = [](QString const& program, QStringList const& arguments = QStringList(), QString const& workingDirectory = QString()) {
    if (!QProcess::startDetached(program, arguments, workingDirectory))
      QMessageBox::warning(nullptr, "", "Error starting starbound server process");
  };


#if defined Q_OS_WIN
  startStarboundServer("cmd.exe", {"/C", ".\\starbound_server.exe"});

#elif defined Q_OS_MACX
  startStarboundServer("/usr/bin/open", {"-a", "/Applications/Utilities/Terminal.app", "./run-server.sh"});

#else
  std::vector<QString> xTermCandidates({
    "/usr/bin/x-terminal-emulator",
    "/usr/bin/konsole",
    "/usr/bin/gnome-terminal.wrapper",
    "/usr/bin/xfce4-terminal.wrapper",
    "/usr/bin/koi8rxterm",
    "/usr/bin/lxterm",
    "/usr/bin/uxterm",
    "/usr/bin/xterm"});
  QString destinationXTerm = "";
  for (auto const& candidate : xTermCandidates) {
    QFileInfo fileInfo(candidate);
    if (fileInfo.exists()) {
      if (fileInfo.isSymLink())
        fileInfo.setFile(fileInfo.symLinkTarget());

      if (fileInfo.isExecutable()) {
        destinationXTerm = candidate;
        break;
      }
    }
  }
  if (destinationXTerm.empty()) {
    QMessageBox::warning(nullptr, "", 
        "Could not find a valid graphical terminal emulator, starting in the background instead, executable name is "
        "starbound_server, to shut down the server, use killall starbound_server");
    startStarboundServer("./starbound_server");
  } else {
    startStarboundServer(destinationXTerm, {"-e", "./starbound_server"});
  }

#endif
}
