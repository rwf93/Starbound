function update(data)
  if data.quests then
    for questName, quest in pairs(data.quests) do
      -- remove unused and deprecated entries
      quest.planet = nil
      quest.triggerPrefix = nil
      quest.triggersReceived = nil

      -- update conditions
      for i, condition in ipairs(quest.conditions) do
        if condition.kind == "gather" then
          condition.count = condition.item.count
          condition.item = condition.item.name
        elseif condition.kind == "gatherTrigger" then
          condition.kind = "gather"
          condition.tag, condition.trigger = condition.trigger, nil
        elseif condition.kind == "trigger" then
          if condition.trigger == "ai.repairthrusters" then
            condition.kind = "shiplevel"
            condition.level = 2
          elseif condition.trigger == "ai.repairftl" then
            condition.kind = "shiplevel"
            condition.level = 3
          else
            sb.logInfo("Unknown quest trigger '%s'", condition.trigger)
          end
        else
          sb.logInfo("Unknown quest condition kind '%s'", condition.kind)
        end

        condition.type, condition.kind = condition.kind, nil
      end
      quest.completionConditions, quest.conditions = quest.conditions, nil

      -- check for weirdness
      if quest.status == "pending" then
        quest.status = "active"
        sb.logInfo("Quest '%s' has unknown status Pending; setting to Active")
      end
    end
  end

  return data
end
