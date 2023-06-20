function init()
  self.flightSpeed = config.getParameter("flightSpeed", 10)
  self.flightForce = 500
  storage.active = false
end

function update(args)
  if not self.specialLast and args.moves["special1"]
      and (storage.active or not status.statPositive("activeMovementAbilities")) then

    storage.active = not storage.active
    if storage.active then
      status.setPersistentEffects("movementAbility", {{stat = "activeMovementAbilities", amount = 1}})
    else
      status.clearPersistentEffects("movementAbility")
    end
  end
  self.specialLast = args.moves["special1"]

  if storage.active then
    if args.moves["up"] and args.moves["right"] then
      mcontroller.controlApproachVelocity({self.flightSpeed, self.flightSpeed}, self.flightForce)
    elseif args.moves["down"] and args.moves["right"] then
      mcontroller.controlApproachVelocity({self.flightSpeed, -self.flightSpeed}, self.flightForce)
    elseif args.moves["up"] and args.moves["left"] then
      mcontroller.controlApproachVelocity({-self.flightSpeed, self.flightSpeed}, self.flightForce)
    elseif args.moves["down"] and args.moves["left"] then
      mcontroller.controlApproachVelocity({-self.flightSpeed, -self.flightSpeed}, self.flightForce)
    elseif args.moves["up"] then
      mcontroller.controlApproachVelocity({0, self.flightSpeed}, self.flightForce)
    elseif args.moves["down"] then
      mcontroller.controlApproachVelocity({0, -self.flightSpeed}, self.flightForce)
    elseif args.moves["right"] then
      mcontroller.controlApproachVelocity({self.flightSpeed, 0}, self.flightForce)
    elseif args.moves["left"] then
      mcontroller.controlApproachVelocity({-self.flightSpeed, 0}, self.flightForce)
    else
      mcontroller.controlApproachVelocity({0, 0}, self.flightForce)
    end

    mcontroller.controlParameters({
      collisionEnabled = false,
      gravityEnabled = false,
      liquidImpedance = 0,
      liquidFriction = 0
    })
  end

  if storage.active then
    tech.setParentDirectives("?fade=3399FFFF;0.4?multiply=FFFFFF66")
  else
    tech.setParentDirectives()
  end
end

function uninit()
  status.clearPersistentEffects("movementAbility")
end
