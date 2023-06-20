function init()
  self.dead = false

  self.state = stateMachine.create({
    "aggroState",
    "spinAttack",
    "bombAttack",
    "spawnAttack",
    "explodeState"
  })
  self.state.shuffleStates()

  entity.setDeathParticleBurst("deathPoof")
end

--------------------------------------------------------------------------------
function update(dt)
  if util.trackTarget(entity.configParameter("noticeDistance")) then
    attack(self.targetId)
  end
  entity.setAggressive(self.targetId ~= nil)
  entity.setDamageOnTouch(self.targetId ~= nil)

  if not self.state.update(dt) then
    entity.setAnimationState("movement", "idle")
  end
end

--------------------------------------------------------------------------------
function damage(args)
  if self.targetId == nil then
    attack(args.sourceId)
  end
end

--------------------------------------------------------------------------------
function shouldDie()
  return entity.health() <= 0 or self.dead
end

--------------------------------------------------------------------------------
function attack(targetId)
  local stateName = self.state.stateDesc()
  if not string.find(stateName, 'Attack$') and stateName ~= "explodeState" then
    if self.state.pickState({ targetId = targetId }) then
      self.targetId = targetId
    end
  end
end

--------------------------------------------------------------------------------
aggroState = {}

function aggroState.enterWith(params)
  if params.targetId == nil then return nil end

  entity.setAnimationState("movement", "aggro")

  return { timer = entity.configParameter("aggroTime") }
end

function aggroState.update(dt, stateData)
  mcontroller.controlFly({ 0, entity.configParameter("aggroMoveSpeed") })

  stateData.timer = stateData.timer - dt
  return stateData.timer <= 0
end

function aggroState.leavingState(stateData)
  self.state.pickState({ attack = true })
end

--------------------------------------------------------------------------------
spinAttack = {}

function spinAttack.enterWith(params)
  if self.targetId == nil then return true end
  if not params.attack then return nil end

  entity.setAnimationState("movement", "spin")
  return {}
end

function spinAttack.update(dt, stateData)
  if self.targetId == nil then return true end

  if stateData.targetPosition == nil then
    stateData.targetPosition = self.targetPosition
  end

  local toTarget = world.distance(stateData.targetPosition, mcontroller.position())
  if world.magnitude(toTarget) <= 1.0 then
    return true
  else
    mcontroller.controlFly(vec2.mul(vec2.norm(toTarget), mcontroller.baseParameters().flySpeed))
  end

  return false
end

function spinAttack.leavingState(stateData)
  self.state.pickState({ explode = true })
end

--------------------------------------------------------------------------------
bombAttack = {}

function bombAttack.enterWith(params)
  if self.targetId == nil then return true end
  if not params.attack then return nil end

  return {
    timer = 0,
    totalTime = entity.randomizeParameterRange("bombAttackTimeRange"),
    direction = util.randomDirection()
  }
end

function bombAttack.enteringState(stateData)
  entity.setFireDirection({0, 0}, {1, 0})
  -- entity.startFiring("bomb")
end

function bombAttack.update(dt, stateData)
  if self.targetId == nil then return true end

  mcontroller.controlFly({ 0, 0 })

  local ratio = stateData.timer / stateData.totalTime
  local angle = ratio * math.pi
  local direction = {
    math.cos(angle) * stateData.direction,
    math.sin(angle)
  }
  direction = vec2.mul(direction, 2.0)

  -- entity.setFireDirection(direction, direction)

  stateData.timer = stateData.timer + dt
  return stateData.timer >= stateData.totalTime
end

function bombAttack.leavingState(stateData)
  -- entity.stopFiring()
  self.state.pickState({ explode = true })
end

--------------------------------------------------------------------------------
spawnAttack = {}

function spawnAttack.enterWith(params)
  if self.targetId == nil then return true end
  if not params.attack then return nil end

  return { timer = entity.configParameter("spawnAttackWaitTime") }
end

function spawnAttack.enteringState(stateData)
  entity.burstParticleEmitter("deathPoof")

  local initialSpeed = entity.configParameter("spawnAttackInitialSpeed")
  for _, offset in pairs(entity.configParameter("spawnAttackOffsets")) do
    local entityId = world.spawnMonster("glitchspider", entity.toAbsolutePosition(offset))
    world.callScriptedEntity(entityId, "setSpawnVelocity", vec2.mul(offset, initialSpeed))
  end
end

function spawnAttack.update(dt, stateData)
  if self.targetId == nil then return nil end

  mcontroller.controlFly({ 0, 0 })

  stateData.timer = stateData.timer - dt
  return stateData.timer <= 0
end

function spawnAttack.leavingState(stateData)
  self.state.pickState({ explode = true })
end

--------------------------------------------------------------------------------
explodeState = {}

function explodeState.enterWith(params)
  if not params.explode or self.dead then return nil end

  return { timer = entity.configParameter("explodeTime") }
end

function explodeState.enteringState(stateData)
  -- entity.startFiring("deathexplosion")
  -- entity.setFireDirection({0, 0}, {1, 0})
end

function explodeState.update(dt, stateData)
  mcontroller.controlFly({ 0, 0 })

  stateData.timer = stateData.timer - dt
  return stateData.timer <= 0
end

function explodeState.leavingState(stateData)
  -- entity.stopFiring()
  self.dead = true
end
