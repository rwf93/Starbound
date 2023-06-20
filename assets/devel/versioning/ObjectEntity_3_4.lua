function update(data)
  if data.animatorState and data.animatorState.states.doorState then
    if data.animatorState.states.doorState == "openRight" or data.animatorState.states.doorState == "openLeft" then
      data.animatorState.states.doorState = "open"
    elseif data.animatorState.states.doorState == "closeRight" or data.animatorState.states.doorState == "closeLeft" then
      data.animatorState.states.doorState = "closed"
    end
  end
  return data
end
