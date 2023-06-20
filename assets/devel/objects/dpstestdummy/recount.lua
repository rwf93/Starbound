function init()
  self.maxHealth = config.getParameter("health")

  self.autoResetTime = config.getParameter("autoResetTime", 7)
  self.autoResetTimer = 0

  reset()

  object.setInteractive(true)
end

function update(dt)
  local currentHealth = object.health()

  self.autoResetTimer = math.max(0, self.autoResetTimer - dt)
  if self.autoResetTimer == 0 then
    reset()
  end

  if currentHealth < self.maxHealth then
    self.meterActive = true
    self.autoResetTimer = self.autoResetTime
  end

  if self.meterActive then
    self.meterTimer = self.meterTimer + dt
    self.totalDamage = self.totalDamage + (self.maxHealth - currentHealth)
    object.setHealth(self.maxHealth)
  end

  drawOutput()
end

function onInteraction(args)
  if self.meterActive then
    reset()
  else
    object.smash()
  end
end

function reset()
  self.meterActive = false
  self.meterTimer = 0
  self.totalDamage = 0
  object.setHealth(self.maxHealth)
end

function drawOutput()
  local pos = entity.position()
  pos = {pos[1] - 1.5, pos[2] + 4}
  local dps = self.meterTimer > 0 and (self.totalDamage / self.meterTimer) or 0
  local outputString = string.format("%.1f damage over\n%.1f seconds\n%.1f DPS", self.totalDamage, self.meterTimer, dps)
  world.debugText(outputString, pos, "white")
end
