require "/scripts/poly.lua"

function update(data)
  if type(data) == "string" then return data end
  for _,paramValue in pairs(data.parameters) do
    if paramValue.type == "location" then
      paramValue.region = poly.boundBox(paramValue.location)
      paramValue.location = nil
    end
  end

  return data
end
