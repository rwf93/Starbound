#ifndef STAR_LAUNCHER_HPP
#define STAR_LAUNCHER_HPP

#include <QMainWindow>
#include <QWebEngineView>

class Launcher : public QMainWindow {
  Q_OBJECT

public:
  Launcher();

private slots:
  void runClient();
  void runServer();

private:
  QWebEngineView* m_web;
};

class WebPage : public QWebEnginePage {
  Q_OBJECT

public:
  using QWebEnginePage::QWebEnginePage;

  bool acceptNavigationRequest(QUrl const& url, NavigationType type, bool isMainFrame);
};

class WebView : public QWebEngineView {
  Q_OBJECT

public:
  using QWebEngineView::QWebEngineView;

  QWebEngineView* createWindow(QWebEnginePage::WebWindowType type);
};

#endif
