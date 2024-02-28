#include "StarClientCommandProcessor.hpp"
#include "StarItem.hpp"
#include "StarAssets.hpp"
#include "StarItemDatabase.hpp"
#include "StarPlayer.hpp"
#include "StarPlayerTech.hpp"
#include "StarPlayerInventory.hpp"
#include "StarPlayerLog.hpp"
#include "StarWorldClient.hpp"
#include "StarAiInterface.hpp"
#include "StarQuestInterface.hpp"
#include "StarStatistics.hpp"

namespace Star {

ClientCommandProcessor::ClientCommandProcessor(UniverseClientPtr universeClient, CinematicPtr cinematicOverlay,
    MainInterfacePaneManager* paneManager, StringMap<StringList> macroCommands)
  : m_universeClient(std::move(universeClient)), m_cinematicOverlay(std::move(cinematicOverlay)),
    m_paneManager(paneManager), m_macroCommands(std::move(macroCommands)) {
  m_builtinCommands = {
    {"reload", bind(&ClientCommandProcessor::reload, this)},
    {"whoami", bind(&ClientCommandProcessor::whoami, this)},
    {"gravity", bind(&ClientCommandProcessor::gravity, this)},
    {"debug", bind(&ClientCommandProcessor::debug, this)},
    {"boxes", bind(&ClientCommandProcessor::boxes, this)},
    {"fullbright", bind(&ClientCommandProcessor::fullbright, this)},
    {"setGravity", bind(&ClientCommandProcessor::setGravity, this, _1)},
    {"resetGravity", bind(&ClientCommandProcessor::resetGravity, this)},
    {"fixedCamera", bind(&ClientCommandProcessor::fixedCamera, this)},
    {"monochromeLighting", bind(&ClientCommandProcessor::monochromeLighting, this)},
    {"radioMessage", bind(&ClientCommandProcessor::radioMessage, this, _1)},
    {"clearRadioMessages", bind(&ClientCommandProcessor::clearRadioMessages, this)},
    {"clearCinematics", bind(&ClientCommandProcessor::clearCinematics, this)},
    {"startQuest", bind(&ClientCommandProcessor::startQuest, this, _1)},
    {"completeQuest", bind(&ClientCommandProcessor::completeQuest, this, _1)},
    {"failQuest", bind(&ClientCommandProcessor::failQuest, this, _1)},
    {"previewNewQuest", bind(&ClientCommandProcessor::previewNewQuest, this, _1)},
    {"previewQuestComplete", bind(&ClientCommandProcessor::previewQuestComplete, this, _1)},
    {"previewQuestFailed", bind(&ClientCommandProcessor::previewQuestFailed, this, _1)},
    {"clearScannedObjects", bind(&ClientCommandProcessor::clearScannedObjects, this)},
    {"played", bind(&ClientCommandProcessor::playTime, this)},
    {"deaths", bind(&ClientCommandProcessor::deathCount, this)},
    {"cinema", bind(&ClientCommandProcessor::cinema, this, _1)},
    {"suicide", bind(&ClientCommandProcessor::suicide, this)},
    {"naked", bind(&ClientCommandProcessor::naked, this)},
    {"resetAchievements", bind(&ClientCommandProcessor::resetAchievements, this)},
    {"statistic", bind(&ClientCommandProcessor::statistic, this, _1)},
    {"giveessentialitem", bind(&ClientCommandProcessor::giveEssentialItem, this, _1)},
    {"maketechavailable", bind(&ClientCommandProcessor::makeTechAvailable, this, _1)},
    {"enabletech", bind(&ClientCommandProcessor::enableTech, this, _1)},
    {"upgradeship", bind(&ClientCommandProcessor::upgradeShip, this, _1)}
  };
}

bool ClientCommandProcessor::adminCommandAllowed() const {
  return Root::singleton().configuration()->get("allowAdminCommandsFromAnyone").toBool() ||
    m_universeClient->mainPlayer()->isAdmin();
}

String ClientCommandProcessor::previewQuestPane(StringList const& arguments, function<PanePtr(QuestPtr)> createPane) {
  Maybe<String> templateId = {};
  templateId = arguments[0];
  if (auto quest = createPreviewQuest(*templateId, arguments.at(1), arguments.at(2), m_universeClient->mainPlayer().get())) {
    auto pane = createPane(quest);
    m_paneManager->displayPane(PaneLayer::ModalWindow, pane);
    return "Previewed quest";
  }
  return "No such quest";
}

StringList ClientCommandProcessor::handleCommand(String const& commandLine) {
  try {
    if (!commandLine.beginsWith("/"))
      throw StarException("ClientCommandProcessor expected command, does not start with '/'");

    String allArguments = commandLine.substr(1);
    String command = allArguments.extract();
    auto arguments = m_parser.tokenizeToStringList(allArguments);

    StringList result;
    if (auto builtinCommand = m_builtinCommands.maybe(command)) {
      result.append((*builtinCommand)(arguments));
    } else if (auto macroCommand = m_macroCommands.maybe(command)) {
      for (auto const& c : *macroCommand) {
        if (c.beginsWith("/"))
          result.appendAll(handleCommand(c));
        else
          result.append(c);
      }
    } else {
      m_universeClient->sendChat(commandLine, ChatSendMode::Broadcast);
    }
    return result;
  } catch (ShellParsingException const& e) {
    Logger::error("Shell parsing exception: %s", outputException(e, false));
    return {"Shell parsing exception"};
  } catch (std::exception const& e) {
    Logger::error("Exception caught handling client command %s: %s", commandLine, outputException(e, true));
    return {strf("Exception caught handling client command %s", commandLine)};
  }
}

bool ClientCommandProcessor::debugDisplayEnabled() const {
  return m_debugDisplayEnabled;
}

bool ClientCommandProcessor::fixedCameraEnabled() const {
  return m_fixedCameraEnabled;
}

String ClientCommandProcessor::reload() {
  Root::singleton().reload();
  return "Client Star::Root reloaded";
}

String ClientCommandProcessor::whoami() {
  return strf("Client: You are %s. You are %san Admin.",
      m_universeClient->mainPlayer()->name(), m_universeClient->mainPlayer()->isAdmin() ? "" : "not ");
}

String ClientCommandProcessor::gravity() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return strf("%s", m_universeClient->worldClient()->gravity(m_universeClient->mainPlayer()->position()));
}

String ClientCommandProcessor::debug() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_debugDisplayEnabled = !m_debugDisplayEnabled;
  return strf("Debug display %s", m_debugDisplayEnabled ? "enabled" : "disabled");
}

String ClientCommandProcessor::boxes() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return strf("Geometry debug display %s",
      m_universeClient->worldClient()->toggleCollisionDebug()
      ? "enabled" : "disabled");
}

String ClientCommandProcessor::fullbright() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return strf("Fullbright render lighting %s",
      m_universeClient->worldClient()->toggleFullbright()
      ? "enabled" : "disabled");
}

String ClientCommandProcessor::setGravity(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->worldClient()->overrideGravity(lexicalCast<float>(arguments.at(0)));
  return strf("Gravity set to %s, the change is LOCAL ONLY", arguments.at(0));
}

String ClientCommandProcessor::resetGravity() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->worldClient()->resetGravity();
  return "Gravity reset";
}

String ClientCommandProcessor::fixedCamera() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_fixedCameraEnabled = !m_fixedCameraEnabled;
  return strf("Fixed camera %s", m_fixedCameraEnabled ? "enabled" : "disabled");
}

String ClientCommandProcessor::monochromeLighting() {
  bool monochrome = !Root::singleton().configuration()->get("monochromeLighting").toBool();
  Root::singleton().configuration()->set("monochromeLighting", monochrome);
  return strf("Monochrome lighting %s", monochrome ? "enabled" : "disabled");
}

String ClientCommandProcessor::radioMessage(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() != 1)
    return "Must provide one argument";

  m_universeClient->mainPlayer()->queueRadioMessage(arguments.at(0));
  return "Queued radio message";
}

String ClientCommandProcessor::clearRadioMessages() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->mainPlayer()->log()->clearRadioMessages();
  return "Player radio message records cleared!";
}

String ClientCommandProcessor::clearCinematics() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->mainPlayer()->log()->clearCinematics();
  return "Player cinematic records cleared!";
}

String ClientCommandProcessor::startQuest(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  auto questArc = QuestArcDescriptor::fromJson(Json::parse(arguments.at(0)));
  m_universeClient->questManager()->offer(make_shared<Quest>(questArc, 0, m_universeClient->mainPlayer().get()));
  return "Quest started";
}

String ClientCommandProcessor::completeQuest(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->questManager()->getQuest(arguments.at(0))->complete();
  return strf("Quest %s complete", arguments.at(0));
}

String ClientCommandProcessor::failQuest(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->questManager()->getQuest(arguments.at(0))->fail();
  return strf("Quest %s failed", arguments.at(0));
}

String ClientCommandProcessor::previewNewQuest(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return previewQuestPane(arguments, [this](QuestPtr const& quest) {
      return make_shared<NewQuestInterface>(m_universeClient->questManager(), quest, m_universeClient->mainPlayer());
    });
}

String ClientCommandProcessor::previewQuestComplete(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return previewQuestPane(arguments, [this](QuestPtr const& quest) {
      return make_shared<QuestCompleteInterface>(quest, m_universeClient->mainPlayer(), CinematicPtr{});
    });
}

String ClientCommandProcessor::previewQuestFailed(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  return previewQuestPane(arguments, [this](QuestPtr const& quest) {
      return make_shared<QuestFailedInterface>(quest, m_universeClient->mainPlayer());
    });
}

String ClientCommandProcessor::clearScannedObjects() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_universeClient->mainPlayer()->log()->clearScannedObjects();
  return "Player scanned objects cleared!";
}

String ClientCommandProcessor::playTime() {
  return strf("Total play time: %s", Time::printDuration(m_universeClient->mainPlayer()->log()->playTime()));
}

String ClientCommandProcessor::deathCount() {
  auto deaths = m_universeClient->mainPlayer()->log()->deathCount();
  return strf("Total deaths: %s%s", deaths, deaths == 0 ? ". Well done!" : "");
}

String ClientCommandProcessor::cinema(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  m_cinematicOverlay->load(Root::singleton().assets()->json(arguments.at(0)));
  if (arguments.size() > 1)
    m_cinematicOverlay->setTime(lexicalCast<float>(arguments.at(1)));
  return strf("Started cinematic %s at %s", arguments.at(0), arguments.size() > 1 ? arguments.at(1) : "beginning");
}

String ClientCommandProcessor::suicide() {
  m_universeClient->mainPlayer()->kill();
  return "You are now dead";
}

String ClientCommandProcessor::naked() {
  auto playerInventory = m_universeClient->mainPlayer()->inventory();
  for (auto slot : EquipmentSlotNames.leftValues())
    playerInventory->addItems(playerInventory->addToBags(playerInventory->takeSlot(slot)));
  return "You are now naked";
}

String ClientCommandProcessor::resetAchievements() {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (m_universeClient->statistics()->reset()) {
    return "Achievements reset";
  }
  return "Unable to reset achievements";
}

String ClientCommandProcessor::statistic(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  StringList values;
  for (String const& statName : arguments) {
    values.append(strf("%s = %s", statName, m_universeClient->statistics()->stat(statName)));
  }
  return values.join("\n");
}

String ClientCommandProcessor::giveEssentialItem(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() < 2)
    return "Not enough arguments to /giveessentialitem";

  try {
    auto item = Root::singleton().itemDatabase()->item(ItemDescriptor(arguments.at(0)));
    auto slot = EssentialItemNames.getLeft(arguments.at(1));
    m_universeClient->mainPlayer()->inventory()->setEssentialItem(slot, item);
    return strf("Put %s in player slot %s", item->name(), arguments.at(1));
  } catch (MapException e) {
    return strf("Invalid essential item slot %s.", arguments.at(1));
  }
}

String ClientCommandProcessor::makeTechAvailable(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() == 0)
    return "Not enouch arguments to /maketechavailable";

  m_universeClient->mainPlayer()->techs()->makeAvailable(arguments.at(0));
  return strf("Added %s to player's visible techs", arguments.at(0));
}

String ClientCommandProcessor::enableTech(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() == 0)
    return "Not enouch arguments to /enabletech";

  m_universeClient->mainPlayer()->techs()->makeAvailable(arguments.at(0));
  m_universeClient->mainPlayer()->techs()->enable(arguments.at(0));
  return strf("Player tech %s enabled", arguments.at(0));
}

String ClientCommandProcessor::upgradeShip(StringList const& arguments) {
  if (!adminCommandAllowed())
    return "You must be an admin to use this command.";

  if (arguments.size() == 0)
    return "Not enouch arguments to /upgradeship";

  auto shipUpgrades = Json::parseJson(arguments.at(0));
  m_universeClient->rpcInterface()->invokeRemote("ship.applyShipUpgrades", shipUpgrades);
  return strf("Upgraded ship");
}

}
