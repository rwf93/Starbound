function updateQuestParameters(parameters)
  local namedIndicators = root.assetJson("/quests/quests.config:indicators")
  for _,paramValue in pairs(parameters) do
    if paramValue.indicator and namedIndicators[paramValue.indicator] then
      paramValue.indicator = namedIndicators[paramValue.indicator].image
    end
  end
end

function update(data)
  if type(data) == "string" then return data end
  updateQuestParameters(data.parameters)
  return data
end
