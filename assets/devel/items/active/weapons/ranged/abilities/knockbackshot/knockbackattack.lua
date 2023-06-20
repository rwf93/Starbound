function setupAbility(altAbilityConfig)
  local knockbackAttack = altAbilityConfig

  function knockbackAttack.init()
    -- sb.logInfo("Initializing knockbackAttack")
  end

  function knockbackAttack.update(dt, fireMode, shiftHeld)
    if fireMode == "alt"
        and storage.fireTimer == 0
        and not world.pointTileCollision(firePosition())
        and status.overConsumeResource("energy", knockbackAttack.energyCost) then

      storage.fireTimer = knockbackAttack.cooldown -- TODO: maybe use separate cooldown?
      -- TODO: appropriate projectile
      animator.setAnimationState("firing", "fire", true)
      animator.setPartTag("muzzleFlash", "variant", math.random(1, 3))
      animator.playSound("knockback")
      animator.burstParticleEmitter("knockback")
      self.recoilTimer = config.getParameter("recoilTime", 0.08)
      mcontroller.addMomentum(vec2.mul(aimVector(), -knockbackAttack.momentum))
      -- TODO: knock back enemies
    end
  end

  return knockbackAttack
end
