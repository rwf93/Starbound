function init()

end

function generateTreasure()
  world.containerTakeAll(pane.containerEntityId())
  local treasurePool = widget.getText("txtTreasurePool")
  if treasurePool and root.isTreasurePool(treasurePool) then
    local level = widget.getText("txtLevel") or 1
    local seed = widget.getText("txtSeed")
    if seed == "" then seed = nil end
    local treasure = root.createTreasure(treasurePool, level, seed)
    for _, item in pairs(treasure) do
      world.containerAddItems(pane.containerEntityId(), item)
    end
  end
end
