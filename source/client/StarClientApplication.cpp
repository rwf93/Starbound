#include "StarClientApplication.hpp"
#include "StarConfiguration.hpp"
#include "StarJsonExtra.hpp"
#include "StarFile.hpp"
#include "StarEncode.hpp"
#include "StarLogging.hpp"
#include "StarJsonExtra.hpp"
#include "StarRoot.hpp"
#include "StarVersion.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerStorage.hpp"
#include "StarPlayerLog.hpp"
#include "StarAssets.hpp"
#include "StarWorldTemplate.hpp"
#include "StarWorldClient.hpp"
#include "StarRootLoader.hpp"

namespace Star {

Json const AdditionalAssetsSettings = Json::parseJson(R"JSON(
    {
      "missingImage" : "/assetmissing.png",
      "missingAudio" : "/assetmissing.wav"
    }
  )JSON");

Json const AdditionalDefaultConfiguration = Json::parseJson(R"JSON(
    {
      "configurationVersion" : {
        "client" : 8
      },

      "allowAssetsMismatch" : false,
      "vsync" : true,
      "limitTextureAtlasSize" : false,
      "useMultiTexturing" : true,
      "audioChannelSeparation" : [-25, 25],

      "sfxVol" : 100,
      "musicVol" : 70,
      "windowedResolution" : [1000, 600],
      "fullscreenResolution" : [1920, 1080],
      "fullscreen" : false,
      "borderless" : false,
      "maximized" : true,
      "zoomLevel" : 3.0,
      "speechBubbles" : true,

      "title" : {
        "multiPlayerAddress" : "",
        "multiPlayerPort" : "",
        "multiPlayerAccount" : ""
      },

      "bindings" : {
        "PlayerUp" :  [ { "type" : "key", "value" : "W", "mods" : [] } ],
        "PlayerDown" :  [ { "type" : "key", "value" : "S", "mods" : [] } ],
        "PlayerLeft" :  [ { "type" : "key", "value" : "A", "mods" : [] } ],
        "PlayerRight" :  [ { "type" : "key", "value" : "D", "mods" : [] } ],
        "PlayerJump" :  [ { "type" : "key", "value" : "Space", "mods" : [] } ],
        "PlayerDropItem" :  [ { "type" : "key", "value" : "Q", "mods" : [] } ],
        "PlayerInteract" :  [ { "type" : "key", "value" : "E", "mods" : [] } ],
        "PlayerShifting" :  [ { "type" : "key", "value" : "RShift", "mods" : [] }, { "type" : "key", "value" : "LShift", "mods" : [] } ],
        "PlayerTechAction1" :  [ { "type" : "key", "value" : "F", "mods" : [] } ],
        "PlayerTechAction2" :  [],
        "PlayerTechAction3" :  [],
        "EmoteBlabbering" :  [ { "type" : "key", "value" : "Right", "mods" : ["LCtrl", "LShift"] } ],
        "EmoteShouting" :  [ { "type" : "key", "value" : "Up", "mods" : ["LCtrl", "LAlt"] } ],
        "EmoteHappy" :  [ { "type" : "key", "value" : "Up", "mods" : [] } ],
        "EmoteSad" :  [ { "type" : "key", "value" : "Down", "mods" : [] } ],
        "EmoteNeutral" :  [ { "type" : "key", "value" : "Left", "mods" : [] } ],
        "EmoteLaugh" :  [ { "type" : "key", "value" : "Left", "mods" : [ "LCtrl" ] } ],
        "EmoteAnnoyed" :  [ { "type" : "key", "value" : "Right", "mods" : [] } ],
        "EmoteOh" :  [ { "type" : "key", "value" : "Right", "mods" : [ "LCtrl" ] } ],
        "EmoteOooh" :  [ { "type" : "key", "value" : "Down", "mods" : [ "LCtrl" ] } ],
        "EmoteBlink" :  [ { "type" : "key", "value" : "Up", "mods" : [ "LCtrl" ] } ],
        "EmoteWink" :  [ { "type" : "key", "value" : "Up", "mods" : ["LCtrl", "LShift"] } ],
        "EmoteEat" :  [ { "type" : "key", "value" : "Down", "mods" : ["LCtrl", "LShift"] } ],
        "EmoteSleep" :  [ { "type" : "key", "value" : "Left", "mods" : ["LCtrl", "LShift"] } ],
        "ShowLabels" :  [ { "type" : "key", "value" : "RAlt", "mods" : [] }, { "type" : "key", "value" : "LAlt", "mods" : [] } ],
        "CameraShift" :  [ { "type" : "key", "value" : "RCtrl", "mods" : [] }, { "type" : "key", "value" : "LCtrl", "mods" : [] } ],
        "TitleBack" :  [ { "type" : "key", "value" : "Esc", "mods" : [] } ],
        "CinematicSkip" :  [ { "type" : "key", "value" : "Esc", "mods" : [] } ],
        "CinematicNext" :  [ { "type" : "key", "value" : "Right", "mods" : [] }, { "type" : "key", "value" : "Return", "mods" : [] } ],
        "GuiClose" :  [ { "type" : "key", "value" : "Esc", "mods" : [] } ],
        "GuiShifting" :  [ { "type" : "key", "value" : "RShift", "mods" : [] }, { "type" : "key", "value" : "LShift", "mods" : [] } ],
        "KeybindingCancel" :  [ { "type" : "key", "value" : "Esc", "mods" : [] } ],
        "KeybindingClear" :  [ { "type" : "key", "value" : "Del", "mods" : [] }, { "type" : "key", "value" : "Backspace", "mods" : [] } ],
        "ChatPageUp" :  [ { "type" : "key", "value" : "PageUp", "mods" : [] } ],
        "ChatPageDown" :  [ { "type" : "key", "value" : "PageDown", "mods" : [] } ],
        "ChatPreviousLine" :  [ { "type" : "key", "value" : "Up", "mods" : [] } ],
        "ChatNextLine" :  [ { "type" : "key", "value" : "Down", "mods" : [] } ],
        "ChatSendLine" :  [ { "type" : "key", "value" : "Return", "mods" : [] } ],
        "ChatBegin" :  [ { "type" : "key", "value" : "Return", "mods" : [] } ],
        "ChatBeginCommand" :  [ { "type" : "key", "value" : "/", "mods" : [] } ],
        "ChatStop" :  [ { "type" : "key", "value" : "Esc", "mods" : [] } ],
        "InterfaceHideHud" :  [ { "type" : "key", "value" : "Z", "mods" : [ "LAlt" ] } ],
        "InterfaceChangeBarGroup" :  [ { "type" : "key", "value" : "X", "mods" : [] } ],
        "InterfaceDeselectHands" :  [ { "type" : "key", "value" : "Z", "mods" : [] } ],
        "InterfaceBar1" :  [ { "type" : "key", "value" : "1", "mods" : [] } ],
        "InterfaceBar2" :  [ { "type" : "key", "value" : "2", "mods" : [] } ],
        "InterfaceBar3" :  [ { "type" : "key", "value" : "3", "mods" : [] } ],
        "InterfaceBar4" :  [ { "type" : "key", "value" : "4", "mods" : [] } ],
        "InterfaceBar5" :  [ { "type" : "key", "value" : "5", "mods" : [] } ],
        "InterfaceBar6" :  [ { "type" : "key", "value" : "6", "mods" : [] } ],
        "InterfaceBar7" :  [],
        "InterfaceBar8" :  [],
        "InterfaceBar9" :  [],
        "InterfaceBar10" :  [],
        "EssentialBar1" :  [ { "type" : "key", "value" : "R", "mods" : [] } ],
        "EssentialBar2" :  [ { "type" : "key", "value" : "T", "mods" : [] } ],
        "EssentialBar3" :  [ { "type" : "key", "value" : "Y", "mods" : [] } ],
        "EssentialBar4" :  [ { "type" : "key", "value" : "N", "mods" : [] } ],
        "InterfaceRepeatCommand" :  [ { "type" : "key", "value" : "P", "mods" : [] } ],
        "InterfaceToggleFullscreen" :  [ { "type" : "key", "value" : "F11", "mods" : [] } ],
        "InterfaceReload" :  [ ],
        "InterfaceEscapeMenu" :  [ { "type" : "key", "value" : "Esc", "mods" : [] } ],
        "InterfaceInventory" :  [ { "type" : "key", "value" : "I", "mods" : [] } ],
        "InterfaceCodex" :  [ { "type" : "key", "value" : "L", "mods" : [] } ],
        "InterfaceQuest" :  [ { "type" : "key", "value" : "J", "mods" : [] } ],
        "InterfaceCrafting" :  [ { "type" : "key", "value" : "C", "mods" : [] } ]
      }
    }
  )JSON");

void ClientApplication::startup(StringList const& cmdLineArgs) {
  RootLoader rootLoader({AdditionalAssetsSettings, AdditionalDefaultConfiguration, String("starbound.log"), LogLevel::Info, false, String("starbound.config")});
  m_root = rootLoader.initOrDie(cmdLineArgs).first;

  Logger::info("Client Version %s (%s) Source ID: %s Protocol: %s", StarVersionString, StarArchitectureString, StarSourceIdentifierString, StarProtocolVersion);

  auto assets = m_root->assets();
  m_minInterfaceScale = assets->json("/interface.config:minInterfaceScale").toInt();
  m_maxInterfaceScale = assets->json("/interface.config:maxInterfaceScale").toInt();
  m_crossoverRes = jsonToVec2F(assets->json("/interface.config:interfaceCrossoverRes"));
}

void ClientApplication::shutdown() {
  m_mainInterface.reset();

  if (m_universeClient)
    m_universeClient->disconnect();

  if (m_universeServer) {
    m_universeServer->stop();
    m_universeServer->join();
    m_universeServer.reset();
  }

  if (m_statistics) {
    m_statistics->writeStatistics();
    m_statistics.reset();
  }

  m_universeClient.reset();
  m_statistics.reset();
}

void ClientApplication::applicationInit(ApplicationControllerPtr appController) {
  Application::applicationInit(appController);

  appController->setCursorVisible(false);

  AudioFormat audioFormat = appController->enableAudio();
  m_mainMixer = make_shared<MainMixer>(audioFormat.sampleRate, audioFormat.channels);

  m_mainMixer->setVolume(0.5);

  m_guiContext = make_shared<GuiContext>(m_mainMixer->mixer(), appController);

  appController->setTargetUpdateRate(1.0f / WorldTimestep);

  auto configuration = m_root->configuration();
  bool vsync = configuration->get("vsync").toBool();
  Vec2U windowedSize = jsonToVec2U(configuration->get("windowedResolution"));
  Vec2U fullscreenSize = jsonToVec2U(configuration->get("fullscreenResolution"));
  bool fullscreen = configuration->get("fullscreen").toBool();
  bool borderless = configuration->get("borderless").toBool();
  bool maximized = configuration->get("maximized").toBool();

  appController->setApplicationTitle(m_root->assets()->json("/client.config:windowTitle").toString());
  appController->setVSyncEnabled(vsync);

  if (fullscreen)
    appController->setFullscreenWindow(fullscreenSize);
  else if (borderless)
    appController->setBorderlessWindow();
  else if (maximized)
    appController->setMaximizedWindow();
  else
    appController->setNormalWindow(windowedSize);

  appController->setMaxFrameSkip(m_root->assets()->json("/client.config:maxFrameSkip").toUInt());
  appController->setUpdateTrackWindow(m_root->assets()->json("/client.config:updateTrackWindow").toFloat());
}

void ClientApplication::renderInit(RendererPtr renderer) {
  Application::renderInit(renderer);

  String rendererConfig = strf("/rendering/%s.config", renderer->rendererId());
  if (m_root->assets()->assetExists(rendererConfig))
    renderer->setEffectConfig(m_root->assets()->json(rendererConfig));
  else
    Logger::warn("No rendering config found for renderer with id '%s'", renderer->rendererId());

  if (m_root->configuration()->get("limitTextureAtlasSize").optBool().value(false))
    renderer->setSizeLimitEnabled(true);

  renderer->setMultiTexturingEnabled(m_root->configuration()->get("useMultiTexturing").optBool().value(true));

  m_guiContext->renderInit(renderer);

  m_cinematicOverlay = make_shared<Cinematic>();
  m_errorScreen = make_shared<ErrorScreen>();

  if (m_titleScreen)
    m_titleScreen->renderInit(renderer);
  if (m_worldPainter)
    m_worldPainter->renderInit(renderer);

  changeState(MainAppState::Mods);
}

void ClientApplication::windowChanged(WindowMode windowMode, Vec2U screenSize) {
  auto config = m_root->configuration();
  if (windowMode == WindowMode::Fullscreen) {
    config->set("fullscreenResolution", jsonFromVec2U(screenSize));
    config->set("fullscreen", true);
    config->set("borderless", false);
  } else if (windowMode == WindowMode::Borderless) {
    config->set("borderless", true);
    config->set("fullscreen", false);
  } else if (windowMode == WindowMode::Maximized) {
    config->set("maximized", true);
    config->set("fullscreen", false);
    config->set("borderless", false);
    config->set("windowedResolution", jsonFromVec2U(screenSize));
  } else {
    config->set("maximized", false);
    config->set("fullscreen", false);
    config->set("borderless", false);
    config->set("windowedResolution", jsonFromVec2U(screenSize));
  }
}

void ClientApplication::processInput(InputEvent const& event) {
  if (auto keyDown = event.ptr<KeyDownEvent>()) {
    m_heldKeyEvents.append(*keyDown);
    m_edgeKeyEvents.append(*keyDown);
  } else if (auto keyUp = event.ptr<KeyUpEvent>()) {
    eraseWhere(m_heldKeyEvents, [&](auto& keyEvent) {
      return keyEvent.key == keyUp->key;
    });

    Maybe<KeyMod> modKey = KeyModNames.maybeLeft(KeyNames.getRight(keyUp->key));
    if (modKey)
      m_heldKeyEvents.transform([&](auto& keyEvent) {
        return KeyDownEvent{keyEvent.key, keyEvent.mods & ~*modKey};
      });
  }

  if (m_state == MainAppState::Splash) {
    m_cinematicOverlay->handleInputEvent(event);

  } else if (m_state == MainAppState::ModsWarning || m_state == MainAppState::Error) {
    m_errorScreen->handleInputEvent(event);

  } else if (m_state == MainAppState::Title) {
    if (!m_cinematicOverlay->handleInputEvent(event))
      m_titleScreen->handleInputEvent(event);

  } else if (m_state == MainAppState::SinglePlayer || m_state == MainAppState::MultiPlayer) {
    if (!m_cinematicOverlay->handleInputEvent(event))
      m_mainInterface->handleInputEvent(event);
  }
}

void ClientApplication::update() {
  if (m_state >= MainAppState::Title) {
    if (auto p2pNetworkingService = appController()->p2pNetworkingService()) {
      if (auto join = p2pNetworkingService->pullPendingJoin()) {
        m_pendingMultiPlayerConnection = PendingMultiPlayerConnection{join.takeValue(), {}, {}};
        changeState(MainAppState::Title);
      }
      
      if (auto req = p2pNetworkingService->pullJoinRequest())
        m_mainInterface->queueJoinRequest(*req);

      p2pNetworkingService->update();
    }
  }

  if (m_state == MainAppState::Mods)
    updateMods();
  else if (m_state == MainAppState::ModsWarning)
    updateModsWarning();
  if (m_state == MainAppState::Splash)
    updateSplash();
  else if (m_state == MainAppState::Error)
    updateError();
  else if (m_state == MainAppState::Title)
    updateTitle();
  else if (m_state > MainAppState::Title)
    updateRunning();

  m_guiContext->cleanup();
  m_edgeKeyEvents.clear();
}

void ClientApplication::render() {
  auto config = m_root->configuration();
  auto assets = m_root->assets();

  if (m_guiContext->windowWidth() >= m_crossoverRes[0] && m_guiContext->windowHeight() >= m_crossoverRes[1])
    m_guiContext->setInterfaceScale(m_maxInterfaceScale);
  else
    m_guiContext->setInterfaceScale(m_minInterfaceScale);

  if (m_state == MainAppState::Mods || m_state == MainAppState::Splash) {
    m_cinematicOverlay->render();

  } else if (m_state == MainAppState::Title) {
    m_titleScreen->render();
    m_cinematicOverlay->render();

  } else if (m_state > MainAppState::Title) {
    if (auto worldClient = m_universeClient->worldClient()) {
      if (auto renderer = Application::renderer())
        renderer->setEffectParameter("lightMapEnabled", true);
      worldClient->render(m_renderData, TilePainter::BorderTileSize);
      m_worldPainter->render(m_renderData);
      m_mainInterface->renderInWorldElements();
      if (auto renderer = Application::renderer())
        renderer->setEffectParameter("lightMapEnabled", false);
    }
    m_mainInterface->render();
    m_cinematicOverlay->render();

  } else if (m_state == MainAppState::ModsWarning || m_state == MainAppState::Error) {
    m_errorScreen->render();
  }
}

void ClientApplication::getAudioData(int16_t* sampleData, size_t frameCount) {
  m_mainMixer->read(sampleData, frameCount);
}

void ClientApplication::changeState(MainAppState newState) {
  MainAppState oldState = m_state;
  m_state = newState;

  if (m_state == MainAppState::Quit)
    appController()->quit();

  if (newState == MainAppState::Mods)
    m_cinematicOverlay->load(m_root->assets()->json("/cinematics/mods/modloading.cinematic"));

  if (newState == MainAppState::Splash) {
    m_cinematicOverlay->load(m_root->assets()->json("/cinematics/splash.cinematic"));
    m_rootLoader = Thread::invoke("Async root loader", [this]() {
        m_root->fullyLoad();
      });
  }

  if (oldState > MainAppState::Title && m_state <= MainAppState::Title) {
    m_mainInterface.reset();
    if (m_universeClient)
      m_universeClient->disconnect();

    if (m_universeServer) {
      m_universeServer->stop();
      m_universeServer->join();
      m_universeServer.reset();
    }
    m_cinematicOverlay->stop();

    if (auto p2pNetworkingService = appController()->p2pNetworkingService()) {
      p2pNetworkingService->setJoinUnavailable();
      p2pNetworkingService->setAcceptingP2PConnections(false);
    }
  }

  if (oldState > MainAppState::Title && m_state == MainAppState::Title)
    m_titleScreen->resetState();

  if (oldState >= MainAppState::Title && m_state < MainAppState::Title) {
    m_playerStorage.reset();

    if (m_statistics) {
      m_statistics->writeStatistics();
      m_statistics.reset();
    }

    m_universeClient.reset();
    m_mainMixer->setUniverseClient({});
    m_titleScreen.reset();
  }

  if (oldState < MainAppState::Title && m_state >= MainAppState::Title) {
    if (m_rootLoader)
      m_rootLoader.finish();

    m_cinematicOverlay->stop();

    m_playerStorage = make_shared<PlayerStorage>(m_root->toStoragePath("player"));
    m_statistics = make_shared<Statistics>(m_root->toStoragePath("player"), appController()->statisticsService());
    m_universeClient = make_shared<UniverseClient>(m_playerStorage, m_statistics);
    m_mainMixer->setUniverseClient(m_universeClient);
    m_titleScreen = make_shared<TitleScreen>(m_playerStorage, m_mainMixer->mixer());
    if (auto renderer = Application::renderer())
      m_titleScreen->renderInit(renderer);
  }

  if (m_state == MainAppState::Title) {
    auto configuration = m_root->configuration();

    if (m_pendingMultiPlayerConnection) {
      if (auto address = m_pendingMultiPlayerConnection->server.ptr<HostAddressWithPort>()) {
        m_titleScreen->setMultiPlayerAddress(toString(address->address()));
        m_titleScreen->setMultiPlayerPort(toString(address->port()));
        m_titleScreen->setMultiPlayerAccount(configuration->getPath("title.multiPlayerAccount").toString());
        m_titleScreen->goToMultiPlayerSelectCharacter(false);
      } else {
        m_titleScreen->goToMultiPlayerSelectCharacter(true);
      }
    } else {
      m_titleScreen->setMultiPlayerAddress(configuration->getPath("title.multiPlayerAddress").toString());
      m_titleScreen->setMultiPlayerPort(configuration->getPath("title.multiPlayerPort").toString());
      m_titleScreen->setMultiPlayerAccount(configuration->getPath("title.multiPlayerAccount").toString());
    }
  }

  if (m_state > MainAppState::Title) {
    if (m_titleScreen->currentlySelectedPlayer()) {
      m_player = m_titleScreen->currentlySelectedPlayer();
    } else {
      if (auto uuid = m_playerStorage->playerUuidAt(0))
        m_player = m_playerStorage->loadPlayer(*uuid);

      if (!m_player) {
        setError("Error loading player!");
        return;
      }
    }

    m_universeClient->setMainPlayer(m_player);
    m_cinematicOverlay->setPlayer(m_player);

    auto assets = m_root->assets();
    String loadingCinematic = assets->json("/client.config:loadingCinematic").toString();
    m_cinematicOverlay->load(assets->json(loadingCinematic));
    if (!m_player->log()->introComplete()) {
      String introCinematic = assets->json("/client.config:introCinematic").toString();
      introCinematic = introCinematic.replaceTags(StringMap<String>{{"species", m_player->species()}});
      m_player->setPendingCinematic(Json(introCinematic));
    } else {
      m_player->setPendingCinematic(Json());
    }

    if (m_state == MainAppState::MultiPlayer) {
      PacketSocketUPtr packetSocket;

      auto multiPlayerConnection = m_pendingMultiPlayerConnection.take();

      if (auto address = multiPlayerConnection.server.ptr<HostAddressWithPort>()) {
        try {
          packetSocket = TcpPacketSocket::open(TcpSocket::connectTo(*address));
        } catch (StarException const& e) {
          setError(strf("Join failed! Error connecting to '%s'", *address), e);
          return;
        }

      } else {
        auto p2pPeerId = multiPlayerConnection.server.ptr<P2PNetworkingPeerId>();

        if (auto p2pNetworkingService = appController()->p2pNetworkingService()) {
          auto result = p2pNetworkingService->connectToPeer(*p2pPeerId);
          if (result.isLeft()) {
            setError(strf("Cannot join peer: %s", result.left()));
            return;
          } else {
            packetSocket = P2PPacketSocket::open(std::move(result.right()));
          }
        } else {
          setError("Internal error, no p2p networking service when joining p2p networking peer");
          return;
        }
      }

      bool allowAssetsMismatch = m_root->configuration()->get("allowAssetsMismatch").toBool();
      if (auto errorMessage = m_universeClient->connect(UniverseConnection(std::move(packetSocket)), allowAssetsMismatch,
            multiPlayerConnection.account, multiPlayerConnection.password)) {
        setError(*errorMessage);
        return;
      }

      if (auto address = multiPlayerConnection.server.ptr<HostAddressWithPort>())
        m_currentRemoteJoin = *address;
      else
        m_currentRemoteJoin.reset();

    } else {
      if (!m_universeServer) {
        try {
          m_universeServer = make_shared<UniverseServer>(m_root->toStoragePath("universe"));
          m_universeServer->start();
        } catch (StarException const& e) {
          setError("Unable to start local server", e);
          return;
        }
      }

      if (auto errorMessage = m_universeClient->connect(m_universeServer->addLocalClient(), "", "")) {
        setError(strf("Error connecting locally: %s", *errorMessage));
        return;
      }
    }

    m_titleScreen->stopMusic();

    m_worldPainter = make_shared<WorldPainter>();
    m_mainInterface = make_shared<MainInterface>(m_universeClient, m_worldPainter, m_cinematicOverlay);

    if (auto renderer = Application::renderer()) {
      m_worldPainter->renderInit(renderer);
    }
  }
}

void ClientApplication::setError(String const& error) {
  Logger::error(error.utf8Ptr());
  m_errorScreen->setMessage(error);
  changeState(MainAppState::Error);
}

void ClientApplication::setError(String const& error, std::exception const& e) {
  Logger::error("%s\n%s", error, outputException(e, true));
  m_errorScreen->setMessage(strf("%s\n%s", error, outputException(e, false)));
  changeState(MainAppState::Error);
}

void ClientApplication::updateMods() {
  m_cinematicOverlay->update();
  auto ugcService = appController()->userGeneratedContentService();
  if (ugcService) {
    if (ugcService->triggerContentDownload()) {
      StringList modDirectories;
      for (auto contentId : ugcService->subscribedContentIds()) {
        if (auto contentDirectory = ugcService->contentDownloadDirectory(contentId)) {
          Logger::info("Loading mods from user generated content with id '%s' from directory '%s'", contentId, *contentDirectory);
          modDirectories.append(*contentDirectory);
        } else {
          Logger::warn("User generated content with id '%s' is not available", contentId);
        }
      }

      if (modDirectories.empty()) {
        Logger::info("No subscribed user generated content");
        changeState(MainAppState::Splash);
      } else {
        Logger::info("Reloading to include all user generated content");
        Root::singleton().reloadWithMods(modDirectories);

        auto configuration = m_root->configuration();
        auto assets = m_root->assets();

        if (configuration->get("modsWarningShown").optBool().value()) {
          changeState(MainAppState::Splash);
        } else {
          configuration->set("modsWarningShown", true);
          m_errorScreen->setMessage(assets->json("/interface.config:modsWarningMessage").toString());
          changeState(MainAppState::ModsWarning);
        }
      }
    }
  } else {
    changeState(MainAppState::Splash);
  }
}

void ClientApplication::updateModsWarning() {
  m_errorScreen->update();

  if (m_errorScreen->accepted())
    changeState(MainAppState::Splash);
}

void ClientApplication::updateSplash() {
  m_cinematicOverlay->update();
  if (!m_rootLoader.isRunning() && (m_cinematicOverlay->completable() || m_cinematicOverlay->completed()))
    changeState(MainAppState::Title);
}

void ClientApplication::updateError() {
  m_errorScreen->update();

  if (m_errorScreen->accepted())
    changeState(MainAppState::Title);
}

void ClientApplication::updateTitle() {
  m_cinematicOverlay->update();

  m_titleScreen->update();
  m_mainMixer->update();

  appController()->setAcceptingTextInput(m_titleScreen->textInputActive());

  auto p2pNetworkingService = appController()->p2pNetworkingService();
  if (p2pNetworkingService)
    p2pNetworkingService->setActivityData("In Main Menu", {});

  if (m_titleScreen->currentState() == TitleState::StartSinglePlayer) {
    changeState(MainAppState::SinglePlayer);

  } else if (m_titleScreen->currentState() == TitleState::StartMultiPlayer) {
    if (!m_pendingMultiPlayerConnection || m_pendingMultiPlayerConnection->server.is<HostAddressWithPort>()) {
      auto addressString = m_titleScreen->multiPlayerAddress().trim();
      auto portString = m_titleScreen->multiPlayerPort().trim();
      portString = portString.empty() ? toString(m_root->configuration()->get("gameServerPort").toUInt()) : portString;
      if (auto port = maybeLexicalCast<uint16_t>(portString)) {
        auto address = HostAddressWithPort::lookup(addressString, *port);
        if (address.isLeft()) {
          setError(address.left());
        } else {
          m_pendingMultiPlayerConnection = PendingMultiPlayerConnection{
            address.right(),
            m_titleScreen->multiPlayerAccount(),
            m_titleScreen->multiPlayerPassword()
          };

          auto configuration = m_root->configuration();
          configuration->setPath("title.multiPlayerAddress", m_titleScreen->multiPlayerAddress());
          configuration->setPath("title.multiPlayerPort", m_titleScreen->multiPlayerPort());
          configuration->setPath("title.multiPlayerAccount", m_titleScreen->multiPlayerAccount());

          changeState(MainAppState::MultiPlayer);
        }
      } else {
        setError(strf("invalid port: %s", portString));
      }
    } else {
      changeState(MainAppState::MultiPlayer);
    }

  } else if (m_titleScreen->currentState() == TitleState::Quit) {
    changeState(MainAppState::Quit);
  }
}

void ClientApplication::updateRunning() {
  try {
    auto p2pNetworkingService = appController()->p2pNetworkingService();
    bool clientIPJoinable = m_root->configuration()->get("clientIPJoinable").toBool();
    bool clientP2PJoinable = m_root->configuration()->get("clientP2PJoinable").toBool();
    Maybe<pair<uint16_t, uint16_t>> party = make_pair(m_universeClient->players(), m_universeClient->maxPlayers());

    if (m_state == MainAppState::MultiPlayer) {
      if (p2pNetworkingService) {
        p2pNetworkingService->setAcceptingP2PConnections(false);
        if (clientP2PJoinable && m_currentRemoteJoin)
          p2pNetworkingService->setJoinRemote(*m_currentRemoteJoin);
        else
          p2pNetworkingService->setJoinUnavailable();
      }
    } else {
      m_universeServer->setListeningTcp(clientIPJoinable);
      if (p2pNetworkingService) {
        p2pNetworkingService->setAcceptingP2PConnections(clientP2PJoinable);
        if (clientP2PJoinable) {
          p2pNetworkingService->setJoinLocal(m_universeServer->maxClients());
        } else {
          p2pNetworkingService->setJoinUnavailable();
          party = {};
        }
      }
    }
    
    if (p2pNetworkingService)
      p2pNetworkingService->setActivityData("In Game", party);

    if (!m_mainInterface->inputFocus() && !m_cinematicOverlay->suppressInput()) {
      m_player->setShifting(isActionTaken(InterfaceAction::PlayerShifting));

      if (isActionTaken(InterfaceAction::PlayerRight))
        m_player->moveRight();
      if (isActionTaken(InterfaceAction::PlayerLeft))
        m_player->moveLeft();
      if (isActionTaken(InterfaceAction::PlayerUp))
        m_player->moveUp();
      if (isActionTaken(InterfaceAction::PlayerDown))
        m_player->moveDown();
      if (isActionTaken(InterfaceAction::PlayerJump))
        m_player->jump();

      if (isActionTaken(InterfaceAction::PlayerTechAction1))
        m_player->special(1);
      if (isActionTaken(InterfaceAction::PlayerTechAction2))
        m_player->special(2);
      if (isActionTaken(InterfaceAction::PlayerTechAction3))
        m_player->special(3);

      if (isActionTakenEdge(InterfaceAction::PlayerInteract))
        m_player->beginTrigger();
      else if (!isActionTaken(InterfaceAction::PlayerInteract))
        m_player->endTrigger();

      if (isActionTakenEdge(InterfaceAction::PlayerDropItem))
        m_player->dropItem();

      if (isActionTakenEdge(InterfaceAction::EmoteBlabbering))
        m_player->addEmote(HumanoidEmote::Blabbering);
      if (isActionTakenEdge(InterfaceAction::EmoteShouting))
        m_player->addEmote(HumanoidEmote::Shouting);
      if (isActionTakenEdge(InterfaceAction::EmoteHappy))
        m_player->addEmote(HumanoidEmote::Happy);
      if (isActionTakenEdge(InterfaceAction::EmoteSad))
        m_player->addEmote(HumanoidEmote::Sad);
      if (isActionTakenEdge(InterfaceAction::EmoteNeutral))
        m_player->addEmote(HumanoidEmote::NEUTRAL);
      if (isActionTakenEdge(InterfaceAction::EmoteLaugh))
        m_player->addEmote(HumanoidEmote::Laugh);
      if (isActionTakenEdge(InterfaceAction::EmoteAnnoyed))
        m_player->addEmote(HumanoidEmote::Annoyed);
      if (isActionTakenEdge(InterfaceAction::EmoteOh))
        m_player->addEmote(HumanoidEmote::Oh);
      if (isActionTakenEdge(InterfaceAction::EmoteOooh))
        m_player->addEmote(HumanoidEmote::OOOH);
      if (isActionTakenEdge(InterfaceAction::EmoteBlink))
        m_player->addEmote(HumanoidEmote::Blink);
      if (isActionTakenEdge(InterfaceAction::EmoteWink))
        m_player->addEmote(HumanoidEmote::Wink);
      if (isActionTakenEdge(InterfaceAction::EmoteEat))
        m_player->addEmote(HumanoidEmote::Eat);
      if (isActionTakenEdge(InterfaceAction::EmoteSleep))
        m_player->addEmote(HumanoidEmote::Sleep);
    }

    auto checkDisconnection = [this]() {
      if (!m_universeClient->isConnected()) {
        m_cinematicOverlay->stop();
        String errMessage;
        if (auto disconnectReason = m_universeClient->disconnectReason())
          errMessage = strf("You were disconnected from the server for the following reason:\n%s", *disconnectReason);
        else
          errMessage = "Client-server connection no longer valid!";
        Logger::error(errMessage.utf8Ptr());
        m_errorScreen->setMessage(errMessage);
        changeState(MainAppState::Error);
        return true;
      }

      return false;
    };

    if (checkDisconnection())
      return;

    m_universeClient->update();

    if (checkDisconnection())
      return;

    if (auto worldClient = m_universeClient->worldClient())
      worldClient->setInteractiveHighlightMode(isActionTaken(InterfaceAction::ShowLabels));

    updateCamera();

    m_cinematicOverlay->update();
    m_mainInterface->update();
    m_mainMixer->update(m_cinematicOverlay->muteSfx(), m_cinematicOverlay->muteMusic());

    appController()->setAcceptingTextInput(m_mainInterface->textInputActive());

    for (auto const& interactAction : m_player->pullInteractActions())
      m_mainInterface->handleInteractAction(interactAction);

    if (m_universeServer) {
      if (auto p2pNetworkingService = appController()->p2pNetworkingService()) {
        for (auto& p2pClient : p2pNetworkingService->acceptP2PConnections())
          m_universeServer->addClient(UniverseConnection(P2PPacketSocket::open(std::move(p2pClient))));
      }

      m_universeServer->setPause(m_mainInterface->escapeDialogOpen());
    }

    Vec2F aimPosition = m_player->aimPosition();
    LogMap::set("render_fps", appController()->renderFps());
    LogMap::set("update_rate", appController()->updateRate());
    LogMap::set("player_pos", strf("%4.2f %4.2f", m_player->position()[0], m_player->position()[1]));
    LogMap::set("player_vel", strf("%4.2f %4.2f", m_player->velocity()[0], m_player->velocity()[1]));
    LogMap::set("player_aim", strf("%4.2f %4.2f", aimPosition[0], aimPosition[1]));
    if (m_universeClient->worldClient()) {
      LogMap::set("liquid_level", strf("%d", m_universeClient->worldClient()->liquidLevel(Vec2I::floor(aimPosition)).level));
      LogMap::set("dungeonId", strf("%d", m_universeClient->worldClient()->dungeonId(Vec2I::floor(aimPosition))));
    }

    if (m_mainInterface->currentState() == MainInterface::ReturnToTitle)
      changeState(MainAppState::Title);

  } catch (std::exception& e) {
    setError("Exception caught in client main-loop", e);
  }
}

bool ClientApplication::isActionTaken(InterfaceAction action) const {
  for (auto keyEvent : m_heldKeyEvents) {
    if (m_guiContext->actions(keyEvent).contains(action))
      return true;
  }

  return false;
}

bool ClientApplication::isActionTakenEdge(InterfaceAction action) const {
  for (auto keyEvent : m_edgeKeyEvents) {
    if (m_guiContext->actions(keyEvent).contains(action))
      return true;
  }

  return false;
}

void ClientApplication::updateCamera() {
  if (!m_universeClient->worldClient())
    return;

  if (m_mainInterface->fixedCamera())
    return;

  auto assets = m_root->assets();
  auto camera = m_worldPainter->camera();

  const float triggerRadius = 100.0f;
  const float deadzone = 0.1f;
  const float smoothFactor = 30.0f;
  const float panFactor = 1.5f;

  auto playerCameraPosition = m_player->cameraPosition();

  if (isActionTaken(InterfaceAction::CameraShift)) {
    m_snapBackCameraOffset = false;
    m_cameraOffsetDownTicks++;
    Vec2F aim = m_universeClient->worldClient()->geometry().diff(m_mainInterface->cursorWorldPosition(), playerCameraPosition);

    float magnitude = aim.magnitude() / (triggerRadius / camera.pixelRatio());
    if (magnitude > deadzone) {
      float cameraXOffset = aim.x() / magnitude;
      float cameraYOffset = aim.y() / magnitude;
      magnitude = (magnitude - deadzone) / (1.0 - deadzone);
      if (magnitude > 1)
        magnitude = 1;
      cameraXOffset *= magnitude * 0.5f * camera.pixelRatio() * panFactor;
      cameraYOffset *= magnitude * 0.5f * camera.pixelRatio() * panFactor;
      m_cameraXOffset = (m_cameraXOffset * (smoothFactor - 1.0) + cameraXOffset) / smoothFactor;
      m_cameraYOffset = (m_cameraYOffset * (smoothFactor - 1.0) + cameraYOffset) / smoothFactor;
    }
  } else {
    if ((m_cameraOffsetDownTicks > 0) && (m_cameraOffsetDownTicks < 20))
      m_snapBackCameraOffset = true;
    if (m_snapBackCameraOffset) {
      m_cameraXOffset = (m_cameraXOffset * (smoothFactor - 1.0)) / smoothFactor;
      m_cameraYOffset = (m_cameraYOffset * (smoothFactor - 1.0)) / smoothFactor;
    }
    m_cameraOffsetDownTicks = 0;
  }
  Vec2F newCameraPosition;

  newCameraPosition.setX(playerCameraPosition.x());
  newCameraPosition.setY(playerCameraPosition.y());

  auto baseCamera = newCameraPosition;

  const float cameraSmoothRadius = assets->json("/interface.config:cameraSmoothRadius").toFloat();
  const float cameraSmoothFactor = assets->json("/interface.config:cameraSmoothFactor").toFloat();

  auto cameraSmoothDistance = m_universeClient->worldClient()->geometry().diff(m_cameraPositionSmoother, newCameraPosition).magnitude();
  if (cameraSmoothDistance > cameraSmoothRadius) {
    auto cameraDelta = m_universeClient->worldClient()->geometry().diff(m_cameraPositionSmoother, newCameraPosition);
    m_cameraPositionSmoother = newCameraPosition + cameraDelta.normalized() * cameraSmoothRadius;
    m_cameraSmoothDelta = {};
  }

  auto cameraDelta = m_universeClient->worldClient()->geometry().diff(m_cameraPositionSmoother, newCameraPosition);
  if (cameraDelta.magnitude() > assets->json("/interface.config:cameraSmoothDeadzone").toFloat())
    newCameraPosition = newCameraPosition + cameraDelta * (cameraSmoothFactor - 1.0) / cameraSmoothFactor;
  m_cameraPositionSmoother = newCameraPosition;

  newCameraPosition.setX(newCameraPosition.x() + m_cameraXOffset / camera.pixelRatio());
  newCameraPosition.setY(newCameraPosition.y() + m_cameraYOffset / camera.pixelRatio());

  auto smoothDelta = newCameraPosition - baseCamera;

  m_worldPainter->setCameraPosition(m_universeClient->worldClient()->geometry(), baseCamera + (smoothDelta + m_cameraSmoothDelta) * 0.5f);
  m_cameraSmoothDelta = smoothDelta;
  camera = m_worldPainter->camera();

  m_universeClient->worldClient()->setClientWindow(camera.worldTileRect());
}

}

STAR_MAIN_APPLICATION(Star::ClientApplication);
