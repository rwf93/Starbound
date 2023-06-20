#ifndef STAR_GUARDED_PTR_HPP
#define STAR_GUARDED_PTR_HPP

#include "StarException.hpp"
#include "StarCasting.hpp"

namespace Star {

STAR_EXCEPTION(GuardedPtrInvalidAccess, StarException);
STAR_EXCEPTION(BadGuardedPtrCast, StarException);

// Simple guarded pointer class that, when the object that it points to is
// destructed, automatically sets itself to invalid.  Similar to
// enable_shared_from_this, with the benefit of not requiring that the class be
// managed by a shared_ptr.  Accessing the guarded pointer is thread safe, with
// one huge caveat:  Because this model of guarded pointers relies on putting
// no restrictions on object destruction, there is no thread safe way of
// *locking* the pointer from destruction.  The pattern of if (guardedPtr) {
// guardedPtr->doThings(); } is inherently unsafe in a context where the
// original object that guardedPtr points to could be deconstructed at any
// time.  If you need thread safety from destruction, then the only viable way
// to do this is enable_shared_from_this and weak_ptr.
template<typename T>
class GuardedPtr {
public:
  typedef typename std::decay<T>::type Type;
  typedef Type* PtrType;
  typedef Type& RefType;

  GuardedPtr();
  GuardedPtr(PtrType ptr, shared_ptr<atomic<bool>> sharedValid);

  template<typename Other>
  GuardedPtr(GuardedPtr<Other> const& other);
  template<typename Other>
  GuardedPtr(GuardedPtr<Other>&& other);

  template<typename Other>
  GuardedPtr& operator=(GuardedPtr<Other> const& other);
  template<typename Other>
  GuardedPtr& operator=(GuardedPtr<Other>&& other);

  bool valid() const;
  explicit operator bool() const;

  PtrType get() const;

  template<typename T2>
  GuardedPtr<T2> as() const;

  template<typename T2>
  GuardedPtr<T2> convert() const;

  void reset();

  PtrType operator->() const;
  RefType operator*() const;

  bool operator==(GuardedPtr const& ptr) const;
  bool operator!=(GuardedPtr const& ptr) const;

  bool operator==(PtrType ptr) const;
  bool operator!=(PtrType ptr) const;

private:
  template<typename Other>
  friend class GuardedPtr;

  PtrType m_ptr;
  shared_ptr<atomic<bool>> m_sharedValid;
};

// Holds a pointer and enables construction of GuardedPtr versions of that
// pointer.  If constructed with autoDelete, then the pointer will be
// automatically deleted on destruction.
template<typename T>
class GuardedHolder {
public:
  typedef typename std::decay<T>::type Type;
  typedef Type* PtrType;

  GuardedHolder();
  GuardedHolder(PtrType ptr, bool autoDelete);
  ~GuardedHolder();

  GuardedHolder(GuardedHolder const& rhs) = delete;
  GuardedHolder& operator=(GuardedHolder const& rhs) = delete;

  GuardedHolder(GuardedHolder&& guardedHolder);
  GuardedHolder& operator=(GuardedHolder&& guardedHolder);

  PtrType pointer() const;
  GuardedPtr<Type> guardedPtr() const;

  // Clears the held pointer and invalidates any GuardedPtrs that reference it.
  void clear();
  // Clears the held pointer and invalidates any GuardedPtrs that reference it,
  // then updates the pointer value to a new value.
  void reset(PtrType ptr, bool autoDelete);

private:
  bool m_autoDelete;
  PtrType m_ptr;
  shared_ptr<atomic<bool>> m_sharedValid;
};

// Easy way to automatically support guardedPtrs to this.
class GuardedHolderBase {
public:
  GuardedHolderBase();
  virtual ~GuardedHolderBase();

  // Invalidates all existing GuardedPtrs to this, and resets the this holder
  // to 'this' again.
  void invalidateExistingGuardedPointers();

  template<typename T>
  friend GuardedPtr<T> guardedPtr(T& t);

  template<typename T>
  friend GuardedPtr<T const> guardedPtr(T const& t);

  template<typename T>
  friend GuardedPtr<T> guardedPtr(T* t);

  template<typename T>
  friend GuardedPtr<T const> guardedPtr(T const* t);

  template<typename T>
  friend GuardedPtr<T> guardedPtr(shared_ptr<T> t);

  template<typename T>
  friend GuardedPtr<T const> guardedPtr(shared_ptr<T const> t);

private:
  GuardedHolder<GuardedHolderBase> m_thisHolder;
};

template<typename T1, typename T2>
GuardedPtr<T1> as(GuardedPtr<T2> const& t);

template<typename T1, typename T2>
GuardedPtr<T1 const> as(GuardedPtr<T2 const> const& t);

template<typename T>
GuardedPtr<T>::GuardedPtr()
  : m_ptr(nullptr) {}

template<typename T>
GuardedPtr<T>::GuardedPtr(PtrType ptr, shared_ptr<atomic<bool>> sharedValid)
  : m_ptr(move(ptr)), m_sharedValid(move(sharedValid)) {}

template<typename T>
template<typename Other>
GuardedPtr<T>::GuardedPtr(GuardedPtr<Other> const& other) {
  m_ptr = other.m_ptr;
  m_sharedValid = other.m_sharedValid;
}

template<typename T>
template<typename Other>
GuardedPtr<T>::GuardedPtr(GuardedPtr<Other>&& other) {
  m_ptr = other.m_ptr;
  m_sharedValid = move(other.m_sharedValid);
}

template<typename T>
template<typename Other>
GuardedPtr<T>& GuardedPtr<T>::operator=(GuardedPtr<Other> const& other) {
  m_ptr = other.m_ptr;
  m_sharedValid = other.m_sharedValid;
  return *this;
}

template<typename T>
template<typename Other>
GuardedPtr<T>& GuardedPtr<T>::operator=(GuardedPtr<Other>&& other) {
  m_ptr = other.m_ptr;
  m_sharedValid = move(other.m_sharedValid);
  return *this;
}

template<typename T>
bool GuardedPtr<T>::valid() const {
  return m_sharedValid && *m_sharedValid;
}

template<typename T>
GuardedPtr<T>::operator bool() const {
  return valid();
}

template<typename T>
auto GuardedPtr<T>::get() const -> PtrType {
  if (valid())
    return m_ptr;
  else
    return nullptr;
}

template<typename T>
template<typename T2>
GuardedPtr<T2> GuardedPtr<T>::convert() const {
  if (auto p = as<T2>())
    return p;

  throw BadGuardedPtrCast();
}

template<typename T>
template<typename T2>
GuardedPtr<T2> GuardedPtr<T>::as() const {
  if (valid()) {
    if (auto p = Star::as<T2>(m_ptr))
      return GuardedPtr<T2>(p, m_sharedValid);
  }

  return {};
}

template<typename T>
void GuardedPtr<T>::reset() {
  m_sharedValid.reset();
}

template<typename T>
auto GuardedPtr<T>::operator->() const -> PtrType {
  if (auto p = get())
    return p;
  else
    throw GuardedPtrInvalidAccess("Invalid access to invalid GuardedPtr");
}

template<typename T>
auto GuardedPtr<T>::operator*() const -> RefType {
  return *operator->();
}

template<typename T>
bool GuardedPtr<T>::operator==(GuardedPtr const& ptr) const {
  return get() == ptr.get();
}

template<typename T>
bool GuardedPtr<T>::operator!=(GuardedPtr const& ptr) const {
  return get() != ptr.get();
}

template<typename T>
bool GuardedPtr<T>::operator==(PtrType ptr) const {
  return get() == ptr;
}

template<typename T>
bool GuardedPtr<T>::operator!=(PtrType ptr) const {
  return get() != ptr;
}

template<typename T>
GuardedHolder<T>::GuardedHolder(PtrType ptr, bool autoDelete)
  : m_autoDelete(false), m_ptr(nullptr) {
  reset(ptr, autoDelete);
}

template<typename T>
GuardedHolder<T>::~GuardedHolder() {
  clear();
}

template<typename T>
GuardedHolder<T>::GuardedHolder(GuardedHolder&& guardedHolder) {
  operator=(move(guardedHolder));
}

template<typename T>
GuardedHolder<T>& GuardedHolder<T>::operator=(GuardedHolder&& guardedHolder) {
  clear();
  m_autoDelete = move(guardedHolder.m_autoDelete);
  m_ptr = move(guardedHolder.m_ptr);
  m_sharedValid = move(guardedHolder.m_sharedValid);
  return *this;
}

template<typename T>
auto GuardedHolder<T>::pointer() const -> PtrType {
  return m_ptr;
}

template<typename T>
auto GuardedHolder<T>::guardedPtr() const -> GuardedPtr<Type> {
  return GuardedPtr<T>(m_ptr, m_sharedValid);
}

template<typename T>
void GuardedHolder<T>::clear() {
  reset(nullptr, false);
}

template<typename T>
void GuardedHolder<T>::reset(PtrType ptr, bool autoDelete) {
  if (m_autoDelete && m_ptr)
    delete m_ptr;

  if (m_sharedValid) {
    *m_sharedValid = false;
    m_sharedValid.reset();
  }

  m_autoDelete = autoDelete;
  m_ptr = ptr;
  if (m_ptr)
    m_sharedValid = make_shared<atomic<bool>>(true);
}

inline GuardedHolderBase::GuardedHolderBase()
  : m_thisHolder(this, false) {}

inline GuardedHolderBase::~GuardedHolderBase() {}

inline void GuardedHolderBase::invalidateExistingGuardedPointers() {
  m_thisHolder.reset(this, false);
}

template<typename T>
GuardedPtr<T> guardedPtr(T& t) {
  return t.m_thisHolder.guardedPtr().template convert<T>();
}

template<typename T>
GuardedPtr<T const> guardedPtr(T const& t) {
  return t.m_thisHolder.guardedPtr().template convert<T const>();
}

template<typename T>
GuardedPtr<T> guardedPtr(T* t) {
  return t->m_thisHolder.guardedPtr().template convert<T>();
}

template<typename T>
GuardedPtr<T const> guardedPtr(T const* t) {
  return t->m_thisHolder.guardedPtr().template convert<T const>();
}

template<typename T>
GuardedPtr<T> guardedPtr(shared_ptr<T> t) {
  return t->m_thisHolder.guardedPtr().template convert<T>();
}

template<typename T>
GuardedPtr<T const> guardedPtr(shared_ptr<T const> t) {
  return t->m_thisHolder.guardedPtr().template convert<T const>();
}

}

#endif
