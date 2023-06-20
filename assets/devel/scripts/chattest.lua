function init()
  object.setInteractive(true)
  self.timer = math.random(0, 2)
end

function onInteraction(args)
  local chatOptions = config.getParameter("portraitChatOptions", {})
  if #chatOptions > 0 then
    local chatChoice = chatOptions[math.random(1, #chatOptions)]
    object.sayPortrait(chatChoice[1], chatChoice[2])
  end
end

function update(dt)
  self.timer = self.timer - dt
  if self.timer <= 0 then
    self.timer = 6
    local chatOptions = config.getParameter("chatOptions", {})
    if #chatOptions > 0 then
      object.say(chatOptions[math.random(1, #chatOptions)])
    end
  end
end
