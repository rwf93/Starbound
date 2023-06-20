require "/scripts/util.lua"

function alreadyVersioned(v)
  return v.__version or v.version
end

function updateItem(item)
  if alreadyVersioned(item) then return item end
  return {
      id = "Item",
      version = 7,
      content = item
    }
end

function updateQuestParameter(paramValue)
  if paramValue.type == "item" then
    paramValue.item = updateItem(paramValue.item)
  elseif paramValue.type == "itemList" then
    paramValue.items = util.map(paramValue.items, updateItem, jarray())
  end
  return paramValue
end

function updateQuestParameters(parameters)
  for paramName, paramValue in pairs(parameters) do
    parameters[paramName] = updateQuestParameter(paramValue)
  end
  return parameters
end

function updateQuestDescriptor(questDescriptor)
  if alreadyVersioned(questDescriptor) then return questDescriptor end
  questDescriptor.parameters = updateQuestParameters(questDescriptor.parameters)
  return {
      id = "QuestDescriptor",
      version = 1,
      content = questDescriptor
    }
end

function updateQuestArcDescriptor(questArcDescriptor)
  if alreadyVersioned(questArcDescriptor) then return questArcDescriptor end
  for i,questDescriptor in pairs(questArcDescriptor.quests) do
    questArcDescriptor.quests[i] = updateQuestDescriptor(questDescriptor)
  end
  return {
      id = "QuestArcDescriptor",
      version = 1,
      content = questArcDescriptor
    }
end

function updateQuest(quest)
  if alreadyVersioned(quest) then return quest end
  quest.parameters = updateQuestParameters(quest.parameters)
  quest.rewards = util.map(quest.rewards, updateItem, jarray())
  quest.arc = updateQuestArcDescriptor(quest.arc)
  return {
      id = "Quest",
      version = 1,
      content = quest
    }
end

function update(data)
  local questMaps = { "activeQuests", "ephemeralInactiveQuests", "completedQuests", "failedQuests" }

  if data.quests then
    for _,questMapName in pairs(questMaps) do
      local questMap = data.quests[questMapName]
      if questMap then
        for questId, quest in pairs(questMap) do
          questMap[questId] = updateQuest(quest)
        end
      end
    end
  end

  return data
end
