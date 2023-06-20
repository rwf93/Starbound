function init()
  animator.rotateGroup("board", 0.6, true)
end

function uninit()
  
end

function update(dt, fireMode, shiftHeld)
  self.aimAndDir = table.pack(activeItem.aimAngleAndDirection(-1, activeItem.ownerAimPosition()))
  activeItem.setFacingDirection(self.aimAndDir[2])

  mcontroller.controlApproachYVelocity(config.getParameter("floatSpeed"), config.getParameter("floatForce"))
end
