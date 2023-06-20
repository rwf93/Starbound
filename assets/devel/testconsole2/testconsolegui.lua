windowCenter = {196, 104}
gameDims = {100, 70}
paddleDims = {2, 8}
ballDims = {3, 3}
ballSpeed = 60
paddleSpeed = 60

gameRect = {
    windowCenter[1] - gameDims[1] - ballDims[1],
    windowCenter[2] - gameDims[2] - ballDims[2],
    windowCenter[1] + gameDims[1] + ballDims[1],
    windowCenter[2] + gameDims[2] + ballDims[2]
  }

p1, p2, ball = {}, {}, {}

function init()
  newGame()

  self.canvas = widget.bindCanvas("scriptCanvas")
  widget.focus("scriptCanvas")
end

function newGame()
  p1.pos = {windowCenter[1] - gameDims[1] + 5, windowCenter[2]}
  p2.pos = {windowCenter[1] + gameDims[1] - 5, windowCenter[2]}
  p1.score = 0
  p2.score = 0
  p1.move = 0
  p2.move = 0

  resetBall()
end

function update(dt)
  self.canvas:clear()

  ball.pos[1] = ball.pos[1] + ball.vel[1] * dt
  ball.pos[2] = ball.pos[2] + ball.vel[2] * dt

  p1.pos[2] = p1.pos[2] + p1.move * dt
  p2.pos[2] = p2.pos[2] + p2.move * dt

  if ball.pos[2] >= windowCenter[2] + gameDims[2] or ball.pos[2] <= windowCenter[2] - gameDims[2] then
    ball.vel[2] = -ball.vel[2]
  end

  if ballHitPaddle(p1) then
    ball.vel[1] = ballSpeed
  elseif ballHitPaddle(p2) then
    ball.vel[1] = -ballSpeed
  end

  if ball.pos[1] >= windowCenter[1] + gameDims[1] then
    p1.score = p1.score + 1
    resetBall()
  elseif ball.pos[1] <= windowCenter[1] - gameDims[1] then
    p2.score = p2.score + 1
    resetBall()
  end

  drawBorders()
  drawPaddle(p1)
  drawPaddle(p2)
  drawBall()
  drawScore()
end

function resetBall()
  ball.pos = {windowCenter[1], windowCenter[2]}
  ball.vel = {ballSpeed * util.randomDirection(), ballSpeed * util.randomDirection()}
end

function ballHitPaddle(paddle)
  return math.abs(ball.pos[1] - paddle.pos[1]) < paddleDims[1] + ballDims[1] and math.abs(ball.pos[2] - paddle.pos[2]) < paddleDims[2]
end

function drawBorders()
  self.canvas:drawRect(
      gameRect,
      {0, 40, 0}
    )
end

function drawBall()
  self.canvas:drawRect(
      {ball.pos[1] - ballDims[1], ball.pos[2] - ballDims[2], ball.pos[1] + ballDims[1], ball.pos[2] + ballDims[2]},
      {255, 255, 255}
    )
end

function drawPaddle(paddle)
  self.canvas:drawRect(
      {paddle.pos[1] - paddleDims[1], paddle.pos[2] - paddleDims[2], paddle.pos[1] + paddleDims[1], paddle.pos[2] + paddleDims[2]},
      {255, 255, 255}
    )
end

function drawScore()
  self.canvas:drawText(
      "Player 1: "..p1.score,
      {position={gameRect[1] - 85, gameRect[4]}, horizontalAnchor="left", verticalAnchor="top"},
      14,
      {255, 255, 255}
    )
  self.canvas:drawText(
      "Player 2: "..p2.score,
      {position={gameRect[3] + 5, gameRect[4]}, horizontalAnchor="left", verticalAnchor="top"},
      14,
      {255, 255, 255}
    )
end

function canvasClickEvent(position, button, isButtonDown)
  sb.logInfo("Mouse button %s was %s at %s", button, isButtonDown and "pressed" or "released", position)
end

function canvasKeyEvent(key, isKeyDown)
  sb.logInfo("Key %s was %s", key, isKeyDown and "pressed" or "released")

  if key == 43 then
    p1.move = isKeyDown and paddleSpeed or 0
  elseif key == 68 then
    p1.move = isKeyDown and -paddleSpeed or 0
  elseif key == 87 then
    p2.move = isKeyDown and paddleSpeed or 0
  elseif key == 88 then
    p2.move = isKeyDown and -paddleSpeed or 0
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
