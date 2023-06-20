-- Provides a proxy to call entity.* functions on an entity id.
--
-- E.g. (from the script for a monster or object):
--
--    local npcIds = world.entityQuery(mcontroller.position(), 100, {includedTypes = {"npc"}})
--    for _, npcId in pairs(npcIds) do
--      local npc = entityProxy.create(npcId)
--      local species = npc.species()
--      world.logInfo("got %s", species)
--      npc.say("I'm a %s", species)
--    end
--
-- Note that functions are called as if they were called from the target
-- entity's lua on the entity table, so this can not call global functions
-- defined in the the entity's lua (only functions that are "entity.*").
--
entityProxy = {}

function entityProxy.create(entityId)
  local wrappers = {}

  local proxyMetatable = {
    __index = function(t, functionName)
      local wrapper = wrappers[functionName]
      if wrapper == nil then
        wrapper = function(...)
          return world.callScriptedEntity(entityId, "entity." .. functionName, ...)
        end
        wrappers[functionName] = wrapper
      end

      return wrapper
    end,

    __newindex = function(t, key, val) end
  }

  local proxy = {}
  setmetatable(proxy, proxyMetatable)
  return proxy
end
