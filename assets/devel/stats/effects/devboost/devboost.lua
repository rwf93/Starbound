function init()
  animator.setParticleEmitterOffsetRegion("devparticles", mcontroller.boundBox())
  animator.setParticleEmitterActive("devparticles", true)
  effect.addStatModifierGroup({
      {stat = "fallDamageMultiplier", effectiveMultiplier = 0}
    })

  script.setUpdateDelta(5)
end

function update(dt)
  mcontroller.controlParameters({
      groundForce = 500,
      airForce = 50,
      gravityMultiplier = 1.5,
      airJumpProfile = {
        autoJump = true
      },
      liquidBuoyancy = -1.0
    })
  mcontroller.controlModifiers({
      airJumpModifier = 2.5,
      liquidJumpModifier = 2.5,
      speedModifier = 5,
      liquidMovementModifier = 5
    })
end

function uninit()
  
end
