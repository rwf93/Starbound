pathState = {
  moveToTargetMinDistance = 3,
  maxYDistanceFromSpawnPoint = 5,
  defaultIndoorSearchRadius = 30
}

function pathState.enter()
  local targetPosition,targetBeacon = pathState.findGoal()
  local desiredDistance = config.getParameter("pathing.desiredDistance", 2)

  if not targetPosition then 
    return nil 
  elseif world.magnitude(world.distance(targetPosition, mcontroller.position())) < desiredDistance then
    return nil
  end

  return {
    targetPosition = targetPosition,
    targetBeacon = targetBeacon,
    desiredDistance = desiredDistance,
    pather = Pather:new({run = config.getParameter("pathing.run", true)})
  }
end

function pathState.enteringState(stateData)
  local position = mcontroller.position()aa

  script.setUpdateDelta(1)
end

function pathState.update(dt, stateData)
  local position = mcontroller.position()

  -- Find goal position
  if not stateData.targetPosition then
    return true
  end

  local toTarget = world.distance(stateData.targetPosition, position)
  local moved = pather.move(stateData.targetPosition, dt)
  if moved == "running" then
    mcontroller.controlFace(pather.deltaX)
    setMovementState()
  elseif moved == false then
    -- Stay in pathing state even when stuck
    setIdleState()
    return false
  elseif moved == true then
    -- Reached the target
    return true
  end

  if world.magnitude(toTarget) <= stateData.desiredDistance then
    return true
  end
end

function pathState.leavingState(stateData)
  world.debugText("leavingPathState", mcontroller.position(), "red")

  local position = mcontroller.position()
  if stateData.targetBeacon ~= nil then
    local distance = world.magnitude(world.distance(stateData.targetPosition, position))
    if distance < stateData.desiredDistance then
      world.callScriptedEntity(stateData.targetBeacon, "onInteraction")
    end
  end
end

function pathState.findGoal()
  local nearObjects = world.entityQuery(mcontroller.position(), 100, {
    includedTypes = { "object"},
    withoutEntityId = entity.id(),
    boundMode = "Position",
    order = "nearest"
  })
  local targetPosition
  for _,objectId in pairs(nearObjects) do
    if world.entityName(objectId) == "apexstatue3" then
      local targetPosition = world.entityPosition(objectId)
      targetPosition[2] = targetPosition[2] - pathState.collisionBottom()
      return targetPosition, objectId
    end
  end
end

function pathState.collisionBottom()
  local lowest = 0
  for _,point in pairs(mcontroller.collisionPoly()) do
    if point[2] < lowest then
      lowest = point[2]
    end
  end
  return lowest
end
