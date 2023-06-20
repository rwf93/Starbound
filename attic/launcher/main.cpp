#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>

#include <iostream>

#include "Launcher.hpp"
#include "Commands.hpp"

void fixupDirectoryStructureKoalaToGiraffe(QDir contentDir) {
  // First, do a lot of sanity checking heuristics to make sure we are actually
  // in a starbound released content directory...

  std::cout << "Detecting old starbound directory layout and migrating from Koala to Giraffe..." << std::endl;

  if (!contentDir.exists("win32") || !contentDir.exists("win64") || !contentDir.exists("linux32")
      || !contentDir.exists("linux64")
      || !contentDir.exists("osx")
      || !contentDir.exists("assets")) {
    std::cout << "Bailing out!  It does not appear that we are running in an installed copy of starbound" << std::endl;
    return;
  }

  if (!contentDir.exists("koala_storage")) {
    std::cout << "Migrating (old) base directories to koala_storage..." << std::endl;
    contentDir.mkdir("koala_storage");
    if (contentDir.exists("mods"))
      contentDir.rename("mods", QDir::toNativeSeparators("koala_storage/mods"));
    if (contentDir.exists("player"))
      contentDir.rename("player", QDir::toNativeSeparators("koala_storage/player"));
    if (contentDir.exists("universe"))
      contentDir.rename("universe", QDir::toNativeSeparators("koala_storage/universe"));
    if (contentDir.exists("starbound.config"))
      contentDir.rename("starbound.config", QDir::toNativeSeparators("koala_storage/starbound.config"));
    if (contentDir.exists("starbound_server.config"))
      contentDir.rename("starbound_server.config", QDir::toNativeSeparators("koala_storage/starbound_server.config"));

    for (auto entry : contentDir.entryList({"starbound.log*"}, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
      contentDir.rename(entry, QDir::toNativeSeparators("koala_storage/" + entry));
    for (auto entry : contentDir.entryList({"starbound_server.log*"}, QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
      contentDir.rename(entry, QDir::toNativeSeparators("koala_storage/" + entry));
  } else {
    std::cout << "It appears we have already applied the Koala backup migration, continuing..." << std::endl;
  }

  if (contentDir.exists("storage_unstable")) {
    std::cout << "Migrating (old) storage_unstable to giraffe_storage..." << std::endl;
    if (!contentDir.exists("giraffe_storage"))
      contentDir.mkdir("giraffe_storage");
    std::cout << "storage_unstable directory from Giraffe series detected, migrating to giraffe_storage" << std::endl;
    QDir storageUnstable = contentDir;
    storageUnstable.cd("storage_unstable");
    for (auto entry : storageUnstable.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
      storageUnstable.rename(entry, QDir::toNativeSeparators(QString("../giraffe_storage/") + entry));
    contentDir.rmdir("storage_unstable");
  } else {
    std::cout << "It appears we do not have a storage_unstable directory, continuing..." << std::endl;
  }
}

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/launcher.ico"));

  QStringList arguments = a.arguments();
  arguments.pop_front();
  if (!arguments.empty()) {
    startClient(arguments);
    return 0;
  }

  QFile styleSheetFile(":/launcher.qss");
  styleSheetFile.open(QIODevice::ReadOnly);
  a.setStyleSheet(styleSheetFile.readAll());

  Launcher l;
  l.show();

#if defined Q_OS_MACX
  QDir appDir(a.applicationDirPath());
  appDir.cdUp();
  appDir.cdUp();
  appDir.cdUp();
  QDir::setCurrent(appDir.canonicalPath());
#elif defined Q_OS_WIN
  QDir appDir(a.applicationDirPath());
  appDir.cdUp();
  QDir::setCurrent(appDir.canonicalPath());
#else
  QDir appDir(a.applicationDirPath());
  QDir::setCurrent(appDir.canonicalPath());
#endif

  appDir.cdUp();
  fixupDirectoryStructureKoalaToGiraffe(appDir);

  return a.exec();
}
