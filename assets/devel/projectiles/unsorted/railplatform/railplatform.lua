require "/scripts/rails.lua"

function init()
  mcontroller.setRotation(0)

  local railConfig = config.getParameter("railConfig", {})
  railConfig.speed = config.getParameter("speed")

  self.railRider = Rails.createRider(railConfig)
  self.railRider:init()
end

function update(dt)
  self.railRider:update(dt)

  if self.railRider:onRail() then
    projectile.setTimeToLive(5)
  end
end

function uninit()
  self.railRider:uninit()
end
