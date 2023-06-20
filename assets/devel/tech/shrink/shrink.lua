require "/scripts/poly.lua"

function init()
  self.shrinkFactor = config.getParameter("shrinkFactor", 0.5)
  self.shrinkTime = config.getParameter("shrinkTime", 1.0)
  self.standingPoly = mcontroller.baseParameters().standingPoly
  self.crouchingPoly = mcontroller.baseParameters().crouchingPoly
  self.active = false
  self.shrinkTimer = 0
end

function update(args)
  if not self.specialLast and args.moves["special1"] then
    storage.active = not storage.active
  end
  self.specialLast = args.moves["special1"]

  if storage.active then
    self.shrinkTimer = math.min(self.shrinkTime, self.shrinkTimer + args.dt)
    updateShrink()
    tech.setToolUsageSuppressed(true)
  elseif self.shrinkTimer > 0 then
    self.shrinkTimer = math.max(0, self.shrinkTimer - args.dt)
    updateShrink()
  else
    tech.setToolUsageSuppressed(false)
  end
end

function updateShrink()
  if self.shrinkTimer > 0 then
    local scaleAmount = 1 - util.round(self.shrinkFactor * (self.shrinkTimer / self.shrinkTime), 2)
    tech.setParentDirectives("?scalenearest=" .. scaleAmount .. "=" .. scaleAmount)

    mcontroller.controlParameters({
      standingPoly = poly.scale(self.standingPoly, scaleAmount),
      crouchingPoly = poly.scale(self.crouchingPoly, scaleAmount)
    })
    mcontroller.controlModifiers({
      speedModifier = scaleAmount,
      airJumpModifier = scaleAmount
    })
  else
    tech.setParentDirectives()
  end
end

function uninit()
  self.shrinkTimer = 0
  updateShrink()
end
