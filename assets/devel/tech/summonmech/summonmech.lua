function init()
end

function update(args)
  if args.moves["special1"] and not self.specialLast then
    world.sendEntityMessage(entity.id(), "unlockMech")
    world.sendEntityMessage(entity.id(), "toggleMech")
  end
  self.specialLast = args.moves["special1"]
end

function uninit()

end
