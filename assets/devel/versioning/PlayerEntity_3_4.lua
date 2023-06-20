function update(data)
  if (not data.bookmarks) then
    data.bookmarks = jarray()
  end

  return data
end
