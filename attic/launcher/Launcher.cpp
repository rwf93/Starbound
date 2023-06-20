#include <QApplication>
#include <QGridLayout>
#include <QPushButton>
#include <QProcess>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QFileInfo>

#if defined Q_OS_WIN
#include <windows.h>
#endif

#include "Launcher.hpp"
#include "Commands.hpp"

Launcher::Launcher() : QMainWindow() {
  setWindowTitle("Starbound Launcher");
  setFixedSize(1200, 700);

  auto* centralWidget = new QWidget(this);
  centralWidget->setObjectName("background");
  setCentralWidget(centralWidget);

  auto* layout = new QGridLayout(centralWidget);
  m_web = new WebView(this);
  layout->addWidget(m_web, 0, 0, 1, 5);

  auto* launchClient = new QPushButton(this);
  launchClient->setObjectName("launchClientButton");
  launchClient->setText("Launch Starbound");
  layout->addWidget(launchClient, 1, 0);
  connect(launchClient, SIGNAL(pressed()), this, SLOT(runClient()));

  auto* launchServer = new QPushButton(this);
  launchServer->setText("Launch Starbound Server");
  launchServer->setObjectName("launchServerButton");
  layout->addWidget(launchServer, 1, 4);
  connect(launchServer, SIGNAL(pressed()), this, SLOT(runServer()));

  m_web->setHtml("<div align=\"center\"><i>Loading, please wait.</i></div>");
  m_web->load(QUrl("http://playstarbound.com/launcher/"));
}

void Launcher::runClient() {
  startClient();
  QApplication::quit();
}

void Launcher::runServer() {
  startServer();
  QApplication::quit();
}

bool WebPage::acceptNavigationRequest(QUrl const& url, NavigationType, bool) {
  QDesktopServices::openUrl(url);
  return false;
}

QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType) {
  auto res = new WebView(this);
  auto page = new WebPage(res);
  res->setPage(page);
  return res;
}
