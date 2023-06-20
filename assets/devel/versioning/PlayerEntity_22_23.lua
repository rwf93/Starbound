function update(data)
  data.techs = {
    availableTechs = jarray(),
    enabledTechs = jarray(),
    equippedTechs = jobject()
  }

  newInventory = jobject()
  newInventory.mainBag = data.inventory.bag
  newInventory.materialBag = data.inventory.tileBag
  newInventory.objectBag = data.inventory.objectBag
  newInventory.reagentBag = data.inventory.reagentBag
  newInventory.foodBag = data.inventory.foodBag
  newInventory.swapSlot = nil
  newInventory.trashSlot = nil
  newInventory.money = data.inventory.money
  newInventory.customBarGroup = 0
  newInventory.customBar = jarray()
  for i = 1, 2 do
    newInventory.customBar[i] = jarray()
    for j = 1, 6 do
      newInventory.customBar[i][j] = jarray()
      for k = 1, 2 do
        newInventory.customBar[i][j][k] = nil
      end
    end
  end
  newInventory.selectedActionBar = 0

  newInventory.beamAxe = data.inventory.essentialBar[1]
  newInventory.wireTool = data.inventory.essentialBar[2]
  newInventory.paintTool = data.inventory.essentialBar[3]

  newInventory.headSlot = data.inventory.equipment[1]
  newInventory.chestSlot = data.inventory.equipment[2]
  newInventory.legsSlot = data.inventory.equipment[3]
  newInventory.backSlot = data.inventory.equipment[4]
  newInventory.headCosmeticSlot = data.inventory.equipment[5]
  newInventory.chestCosmeticSlot = data.inventory.equipment[6]
  newInventory.legsCosmeticSlot = data.inventory.equipment[7]
  newInventory.backCosmeticSlot = data.inventory.equipment[8]

  data.inventory = newInventory

  return data
end
