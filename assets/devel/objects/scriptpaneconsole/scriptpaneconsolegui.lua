require "/scripts/vec2.lua"

function init()
  self.buttonPresses = 0

  self.canvas = widget.bindCanvas("cvsTest")

  local rs = sb.makeRandomSource(1234)
  self.canvasOffsetMagnitude = 40
  self.canvasOffsetAngle = 0
end

function update(dt)
  self.canvas:clear()
  self.canvasOffsetAngle = self.canvasOffsetAngle + 0.01
  local canvasOffset = vec2.withAngle(self.canvasOffsetAngle, self.canvasOffsetMagnitude)
  self.canvas:drawTiledImage("/interface/xhover.png", {0, 0}, {0, 0, 100, 100}, 1)
  self.canvas:drawTiledImage("/interface/xhover.png", {0, 0}, {105, 0, 205, 100}, 2)
end

function buttonPressed(widgetName, widgetData)
  self.buttonPresses = self.buttonPresses + 1
  widget.setText("lblTestLabel", "count " .. self.buttonPresses)
  widget.focus("tbTestTextbox")
  widget.playSound("/sfx/npc/monsters/monster_surprise.ogg")
end

function textboxChanged(widgetName, widgetData)
  widget.setText("btnTestButton", widget.getText(widgetName))
end

function sliderChanged(widgetName, widgetData)
  widget.setProgress("prgTestProgress", widget.getSliderValue(widgetName))
  widget.setImageRotation("imgTestImage", 2 * math.pi * widget.getSliderValue(widgetName) / 100)
end

function radioGroupChanged(widgetName, widgetData)
  if widgetData and widgetData.color then
    widget.setImage("imgTestImage", "/animations/cat/cat" .. widgetData.color .. ".png")
  end
end

function checkboxChanged(widgetName, widgetData)
  widget.setButtonEnabled("btnTestButton", widget.getChecked(widgetName))
end
