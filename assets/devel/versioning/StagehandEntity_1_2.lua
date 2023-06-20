function updatePlugins(plugins)
  for questId, questPlugin in pairs(plugins) do
    plugins[questId] = jarray()
    table.insert(plugins[questId], questPlugin)
  end
  return plugins
end

function updatePluginStorage(pluginStorage)
  for questId, pluginData in pairs(pluginStorage) do
    pluginStorage[questId] = jarray()
    table.insert(pluginStorage[questId], pluginData)
  end
  return pluginStorage
end

function updateScriptStorage(storage)
  if storage.quest and storage.quest.pluginStorage then
    storage.quest.pluginStorage = updatePluginStorage(storage.quest.pluginStorage)
  end
  return storage
end

function update(data)
  if data.type == "questmanager" then
    data.plugins = data.plugins and updatePlugins(data.plugins)
    data.scriptStorage = data.scriptStorage and updateScriptStorage(data.scriptStorage)
  end

  return data
end
