function init()

end

function questComplete()

end

function questStart()
  local enableMissions = config.getParameter("enableMissions", {})
  for _, missionName in pairs(enableMissions) do
    player.enableMission(missionName)
  end

  local essentialItems = config.getParameter("essentialItems", {})
  for slotName, itemConfig in pairs(essentialItems) do
    player.giveEssentialItem(slotName, itemConfig)
  end

  quest.complete()
end

function update(dt)
end

function uninit()

end
