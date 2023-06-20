require "/scripts/rect.lua"

function init()
  mcontroller.resetAnchorState()
  local vector = vec2.rotate({1, 0}, mcontroller.rotation())
  if vector[1] < 0 then
    vector = vec2.rotate(vector, -0.2)
  else
    vector = vec2.rotate(vector, 0.2)
  end
  local ballisticVelocity = vec2.mul(vector, 50)
  mcontroller.setVelocity(ballisticVelocity)
end

function update(dt)
  local stickingDirection = mcontroller.stickingDirection()
  if not stickingDirection then
    local angle = vec2.angle(mcontroller.velocity())
    mcontroller.setRotation(angle - math.pi / 2)
  else
    self.expireOnMove = true
  end

  if self.expireOnMove and (mcontroller.running() or mcontroller.walking()) then
    effect.expire()
  end

  mcontroller.controlParameters({
    standingPoly = { {-0.75, -2.0}, {-0.35, -2.5}, {0.35, -2.5}, {0.75, -2.0}, {0.75, -0.5}, {0.35, 0.25}, {-0.35, 0.25}, {-0.75, -0.5} },
    stickyCollision = true,
    stickyForce = 500
  })
end

function uninit()
  mcontroller.setRotation(0)
  mcontroller.setPosition(vec2.add(mcontroller.position(), {0, 1}))
end
