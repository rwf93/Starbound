require("/scripts/quest/serialize.lua")
require("/scripts/questgen/generator.lua")
require("/scripts/questgen/plannerTests.lua")

function init()
  object.setInteractive(true)
  setupTests()
end

function chooseQuestGiver()
  if storage.questGiver then
    return world.loadUniqueEntity(storage.questGiver)
  end

  local entities = util.filter(world.npcQuery(object.position(), 50), function (npc)
      return world.callScriptedEntity(npc, "config.getParameter", "questGenerator.enableParticipation")
    end)
  if #entities == 0 then return nil end
  return entities[math.random(#entities)]
end

function withGenerator(func)
  local questGiver = chooseQuestGiver()
  if not questGiver then return end

  local generator = QuestGenerator.new()
  generator.debug = true
  generator.questGiver = questGiver
  generator.questPools = world.callScriptedEntity(questGiver, "config.getParameter", "questGenerator.pools") or {}

  self.co = coroutine.create(function ()
      local arc = func(generator)
      if arc then
        world.spawnStagehand(generator:position(), "questmanager", {
            uniqueId = arc.questArc.stagehandUniqueId,
            quest = {
                arc = storeQuestArcDescriptor(arc.questArc),
                participants = arc.participants
              },
            plugins = arc.managerPlugins
          })
      end
      return arc
    end)
end

function generate(questName)
  questName = questName or storage.questName
  withGenerator(function (generator)
      local quest = generator:questPool().quests[questName]
      quest.name = questName
      return generator:generate(quest)
    end)
end

function timing()
  local questGiver = chooseQuestGiver()
  if not questGiver then return end

  local generator = QuestGenerator.new()
  generator.debug = true
  generator.measureTime = true

  local stepCount = 0
  while generator.coroutine == nil or coroutine.status(generator.coroutine) ~= "dead" do
    generator:generateStep()
    stepCount = stepCount + 1
  end
  sb.logInfo("Total steps: %s", stepCount)
  sb.logInfo("Estimated generation time over multiple frames: %ss", stepCount/6)
end

function onInteraction()
  if storage.questName then
    generate()
    return
  end

  withGenerator(function (generator)
      return generator:generate()
    end)
end

function update(dt)
  if self.co and coroutine.status(self.co) ~= "dead" then
    local status, result = coroutine.resume(self.co)
    if not status then
      sb.logInfo("Generator broke: %s", result)
    end
  end
end
