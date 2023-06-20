function update(store)
  if store.data then
    if store.data.recoil then
      if not store.data.recoilTime then
        store.data.recoilTime = store.data.recoil
      end
      vremove(store.data, "recoil")
    end
  end
  return store
end
