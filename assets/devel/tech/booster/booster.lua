function init()
  self.lastJump = false
  self.lastBoostDirection = nil

  self.energyUsagePerSecond = config.getParameter("energyUsagePerSecond")
  self.boostSpeed = config.getParameter("boostSpeed")
  self.boostControlForce = config.getParameter("boostControlForce")
end

function update(args)
  local boostDirection

  if not mcontroller.onGround() then
    if not mcontroller.canJump() and args.moves["jump"] and not self.lastJump then
      local diag = 1 / math.sqrt(2)

      if args.moves["right"] and args.moves["up"] then
        boostDirection = {self.boostSpeed * diag, self.boostSpeed * diag}
      elseif args.moves["right"] and args.moves["down"] then
        boostDirection = {self.boostSpeed * diag, -self.boostSpeed * diag}
      elseif args.moves["left"] and args.moves["up"] then
        boostDirection = {-self.boostSpeed * diag, self.boostSpeed * diag}
      elseif args.moves["left"] and args.moves["down"] then
        boostDirection = {-self.boostSpeed * diag, -self.boostSpeed * diag}
      elseif args.moves["right"] then
        boostDirection = {self.boostSpeed, 0}
      elseif args.moves["down"] then
        boostDirection = {0, -self.boostSpeed}
      elseif args.moves["left"] then
        boostDirection = {-self.boostSpeed, 0}
      elseif args.moves["up"] then
        boostDirection = {0, self.boostSpeed}
      end
    elseif args.moves["jump"] and self.lastBoostDirection then
      boostDirection = self.lastBoostDirection
    end
  end

  self.lastJump = args.moves["jump"]
  self.lastBoostDirection = boostDirection

  if boostDirection and status.overConsumeResource("energy", self.energyUsagePerSecond * args.dt) then
    mcontroller.controlApproachVelocity(boostDirection, self.boostControlForce)

    animator.setAnimationState("boosting", "on")
    animator.setParticleEmitterActive("boostParticles", true)
  else
    animator.setAnimationState("boosting", "off")
    animator.setParticleEmitterActive("boostParticles", false)
  end
end
