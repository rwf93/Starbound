function init()
  self.modes = {"off", "worldInfo", "rectQuery", "radQuery", "lineQuery", "showCollision", "resolveCollision", "lineCollision"}
  self.mode = 1
  self.radii = {0.5, 1.5, 5}
  self.radius = 2
  self.shapes = {"square", "diamond", "octagon"}
  self.shape = 1
  self.boundModes = {"MetaBoundBox", "CollisionArea", "Position"}
  self.boundMode = 1
  self.entityTypes = {"all", "creature", "mobile", "player", "monster", "npc", "itemDrop", "projectile", "plant", "plantDrop", "effect"}
  self.entityType = 1
  self.collideTolerance = 2
  self.lastMoves = {}
end

function update(args)
  if args.moves["special1"] and not self.lastMoves["special1"] then
    cycleMode()
  elseif args.moves["special2"] and not self.lastMoves["special2"] then
    cycleRadius()
  elseif args.moves["special3"] and not self.lastMoves["special3"] then
    if self.mode == 3 or self.mode == 4 or self.mode == 5 then
      if args.moves["down"] and not self.lastMoves["down"] then
        cycleBoundMode()
      else
        cycleEntityType()
      end
    elseif self.mode == 6 or self.mode == 7 then
      cycleShape()
    end
  end

  self.lastMoves = args.moves

  local modeStr = self.modes[self.mode]
  args.aimPosition = tech.aimPosition()
  if modeStr == "worldInfo" then
    doWorldInfo(args)
  elseif modeStr == "rectQuery" then
    doRectQuery(args)
  elseif modeStr == "radQuery" then
    doRadQuery(args)
  elseif modeStr == "lineQuery" then
    doLineQuery(args)
  elseif modeStr == "showCollision" then
    doShowCollision(args)
  elseif modeStr == "resolveCollision" then
    doResolveCollision(args)
  elseif modeStr == "lineCollision" then
    doLineCollision(args)
  end

  return 0
end

function cycleMode()
  self.mode = (self.mode % #self.modes) + 1
end

function cycleRadius()
  self.radius = (self.radius % #self.radii) + 1
end

function cycleShape()
  self.shape = (self.shape % #self.shapes) + 1
end

function cycleBoundMode()
  self.boundMode = (self.boundMode % #self.boundModes) + 1
end

function cycleEntityType()
  self.entityType = (self.entityType % #self.entityTypes) + 1
end

function doWorldInfo(args)
  local pos = args.aimPosition
  local message = string.format("Position %.3f, %.3f\nGravity %s\nLight %.3f\nWind %s\nFG %s (mod %s)\nBG %s (mod %s)",
      pos[1], pos[2],
      world.gravity(pos),
      world.lightLevel(pos),
      world.windLevel(pos),
      world.material(pos, "foreground"),
      world.mod(pos, "foreground"),
      world.material(pos, "background"),
      world.mod(pos, "background"))
  world.debugPoint(pos, "green")
  world.debugText(message, {pos[1] + 1, pos[2] - 1}, "white")
end

function makeQueryOptions()
  local queryOptions = {
    boundMode = self.boundModes[self.boundMode]
  }

  if self.entityTypes[self.entityType] ~= "all" then
    queryOptions.includedTypes = {self.entityTypes[self.entityType]}
  end

  return queryOptions
end

function doRectQuery(args)
  local queryRadius = self.radii[self.radius]

  local ll = {args.aimPosition[1] - queryRadius, args.aimPosition[2] - queryRadius}
  local ur = {args.aimPosition[1] + queryRadius, args.aimPosition[2] + queryRadius}

  local entityIds = world.entityQuery(ll, ur, makeQueryOptions())

  drawDebugRect({ll[1], ll[2], ur[1], ur[2]}, "#7777FF")
  world.debugText(string.format("%s entities (rect, %s, %s)", #entityIds, self.entityTypes[self.entityType], self.boundModes[self.boundMode]), {ll[1], ur[2]}, "white")

  for _, entityId in pairs(entityIds) do
    world.debugPoint(world.entityPosition(entityId), "green")
    world.debugText(world.entityType(entityId).." "..entityId, world.entityPosition(entityId), "white")
  end
end

function doRadQuery(args)
  local queryRadius = self.radii[self.radius]

  local entityIds = world.entityQuery(args.aimPosition, queryRadius, makeQueryOptions())

  drawDebugPoly(makePoly("octagon", queryRadius), "#7777FF", args.aimPosition)
  world.debugText(string.format("%s entities (rad, %s, %s)", #entityIds, self.entityTypes[self.entityType], self.boundModes[self.boundMode]), {args.aimPosition[1] - queryRadius, args.aimPosition[2] + queryRadius}, "white")

  for _, entityId in pairs(entityIds) do
    world.debugPoint(world.entityPosition(entityId), "green")
    world.debugText(world.entityType(entityId).." "..entityId, world.entityPosition(entityId), "white")
  end
end

function doLineQuery(args)
  local queryRadius = self.radii[self.radius]

  local ll = {args.aimPosition[1] - queryRadius, args.aimPosition[2] - queryRadius}
  local ur = {args.aimPosition[1] + queryRadius, args.aimPosition[2] + queryRadius}

  local entityIds = world.entityLineQuery(ll, ur, makeQueryOptions())

  world.debugLine(ll, ur, "#7777FF")
  world.debugText(string.format("%s entities (line, %s, %s)", #entityIds, self.entityTypes[self.entityType], self.boundModes[self.boundMode]), {ll[1], ur[2]}, "white")

  for _, entityId in pairs(entityIds) do
    world.debugPoint(world.entityPosition(entityId), "green")
    world.debugText(world.entityType(entityId).." "..entityId, world.entityPosition(entityId), "white")
  end
end

function doShowCollision(args)
  local polyRadius = self.radii[self.radius]
  local polyShape = self.shapes[self.shape]
  local collidePoly = makePoly(polyShape, polyRadius)

  local colliding = world.polyCollision(collidePoly, args.aimPosition)

  if colliding then
    drawDebugPoly(collidePoly, "red", args.aimPosition)
  else
    drawDebugPoly(collidePoly, "green", args.aimPosition)
  end
end

function doResolveCollision(args)
  local polyRadius = self.radii[self.radius]
  local polyShape = self.shapes[self.shape]
  local collidePoly = makePoly(polyShape, polyRadius)

  local resolvedPoint = world.resolvePolyCollision(collidePoly, args.aimPosition, self.collideTolerance)

  if resolvedPoint == nil then
    drawDebugPoly(collidePoly, "red", args.aimPosition)
  elseif resolvedPoint[1] == args.aimPosition[1] and resolvedPoint[2] == args.aimPosition[2] then
    drawDebugPoly(collidePoly, "green", args.aimPosition)
  else
    world.debugLine(args.aimPosition, resolvedPoint, "yellow")
    drawDebugPoly(collidePoly, "blue", resolvedPoint)
    world.debugText(string.format("%.2f", world.magnitude(args.aimPosition, resolvedPoint)), args.aimPosition, "white")
  end
end

function doLineCollision(args)
  local lineRadius = self.radii[self.radius]
  local lineMin = {args.aimPosition[1] - lineRadius, args.aimPosition[2] + lineRadius}
  local lineMax = {args.aimPosition[1] + lineRadius, args.aimPosition[2] - lineRadius}
  local colliding = world.lineTileCollision(lineMin, lineMax)

  if colliding then
    world.debugLine(lineMin, lineMax, "red")
  else
    world.debugLine(lineMin, lineMax, "green")
  end
end

function makePoly(shape, radius)
  if shape == "square" then
    return {
      {-radius, -radius},
      {-radius, radius},
      {radius, radius},
      {radius, -radius}
    }
  elseif shape == "diamond" then
    return {
      {-radius, 0},
      {0, radius},
      {radius, 0},
      {0, -radius}
    }
  elseif shape == "octagon" then
    local smr = radius / 3
    return {
      {-radius, smr},
      {-smr, radius},
      {smr, radius},
      {radius, smr},
      {radius, -smr},
      {smr, -radius},
      {-smr, -radius},
      {-radius, -smr}
    }
  end

  sb.logInfo("Failed to make poly for shape %s and radius %s", shape, radius)
  return {}
end

function drawDebugRect(rect, color, basePos)
  if basePos then rect = translate(rect, basePos) end
  world.debugLine({rect[1], rect[2]}, {rect[1], rect[4]}, color)
  world.debugLine({rect[1], rect[2]}, {rect[3], rect[2]}, color)
  world.debugLine({rect[3], rect[4]}, {rect[1], rect[4]}, color)
  world.debugLine({rect[3], rect[4]}, {rect[3], rect[2]}, color)
end

function drawDebugPoly(poly, color, basePos)
  local basePos = basePos or {0, 0}
  if #poly >= 3 then
    for i, point in ipairs(poly) do
      world.debugLine(translate(point, basePos), translate(poly[(i % #poly) + 1], basePos), color)
    end
  end
end

function translate(pointOrRect, offset)
  if #pointOrRect == 4 then
    return {pointOrRect[1] + offset[1], pointOrRect[2] + offset[2], pointOrRect[3] + offset[1], pointOrRect[4] + offset[2]}
  elseif #pointOrRect == 2 then
    return {pointOrRect[1] + offset[1], pointOrRect[2] + offset[2]}
  end
end

function printTable(t, indent)
  if not indent then
    indent = ""
    sb.logInfo("Printing table...")
  end
  local tnames = {}
  for k,v in pairs(t) do
    tnames[#tnames + 1] = k
  end
  table.sort(tnames, function(a, b) return a < b end)
  for _, key in ipairs(tnames) do
    if type(t[key]) == "table" then
      sb.logInfo(indent.."table "..key)
      printTable(t[key], indent.."  ")
    elseif type(t[key]) == "function" then
      sb.logInfo(indent.."function "..key)
    else
      sb.logInfo(indent..type(t[key]).." "..key.." = %s", t[key])
    end
  end
end
