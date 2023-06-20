require "/versioning/QuestDescriptor_2_3.lua"

function update(data)
  updateQuestParameters(data.parameters)
  return data
end
