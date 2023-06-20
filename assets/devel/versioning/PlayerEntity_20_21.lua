require "/scripts/util.lua"

function addQuests(questManagerMap, quests)
  for questId, quest in pairs(quests) do
    questManagerMap[questId] = quest
  end
end

function updateQuest(quest, state)
  quest.state = state
  return quest
end

function updateQuests(quests, state)
  return util.map(quests, function (quest)
      local quest = root.loadVersionedJson(quest, "Quest")
      quest = updateQuest(quest, state)
      return root.makeCurrentVersionedJson("Quest", quest)
    end, jobject())
end

function update(data)
  local quests = data.quests

  quests.quests = jobject()

  addQuests(quests.quests, updateQuests(quests.activeQuests or jarray(), "Active"))
  addQuests(quests.quests, updateQuests(quests.completedQuests or jarray(), "Complete"))
  addQuests(quests.quests, updateQuests(quests.failedQuests or jarray(), "Failed"))
  addQuests(quests.quests, updateQuests(quests.newQuests or jarray(), "Offer"))

  jremove(quests, "activeQuests")
  jremove(quests, "completedQuests")
  jremove(quests, "failedQuests")
  jremove(quests, "newQuests")

  return data
end
