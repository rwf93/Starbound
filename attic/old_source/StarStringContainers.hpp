#ifndef STAR_STRING_CONTAINERS_HPP
#define STAR_STRING_CONTAINERS_HPP

#include "StarString.hpp"
#include "StarList.hpp"
#include "StarMap.hpp"
#include "StarSet.hpp"
#include "StarMaybe.hpp"

namespace Star {

class StringList : public List<String> {
public:
  typedef List<String> Base;
  typedef Base::iterator iterator;
  typedef Base::const_iterator const_iterator;

  typedef Base::value_type value_type;

  template<typename Container>
  static StringList from(Container const& m) {
    return StringList(m.begin(), m.end());
  }

  StringList();
  StringList(Base const& l);
  StringList(Base&& l);
  StringList(size_t len, String::Char const* const* list);
  StringList(size_t len, String const& s1);
  StringList(std::initializer_list<String> list);

  template<typename InputIterator>
  StringList(InputIterator beg, InputIterator end) : Base(beg, end) {}

  bool contains(String const& s, String::CaseSensitivity cs = String::CaseSensitive) const;
  StringList trimAll(String const& pattern = "") const;
  String join(String const& separator = "") const;

  StringList& append(String const& e);
  StringList& append(String&& e);
  StringList& prepend(String const& e);
  StringList& prepend(String&& e);

  template<typename Container>
  StringList& appendAll(Container&& list);
  template<typename Container>
  StringList& prependAll(Container&& list);

  StringList& remove(String const& e);

  StringList& erase(size_t index);
  StringList& erase(size_t begin, size_t end);
  using Base::erase;

  StringList& insert(size_t pos, String const& e);
  template<typename Container>
  StringList& insertAll(size_t pos, Container const& l);
  using Base::insert;

  StringList& set(size_t pos, String const& e);

  StringList slice(SliceIndex a = SliceIndex(), SliceIndex b = SliceIndex(), int i = 1) const;
};

std::ostream& operator<<(std::ostream& os, StringList const& list);

// A type that holds a reference to String/std::string/char*, or an actual held
// String.  The lifetime of the class must not be longer than that of a normal
// const&.  Is efficient for both lvalue and rvalue references.  Internally to
// StringMap, hold() will be called to ensure that the ref holds an actual
// copy of the string.
class StringRef {
public:
  // All lvalue constructors make the StringRef hold only the pointer, all
  // rvalue constructors cause StringRef to hold the actual String.
  StringRef(String const& s);
  StringRef(String&& s);
  StringRef(std::string const& s);
  StringRef(std::string&& s);
  StringRef(char const* p);

  // No copy constructor is provided to make tracking ownership sane.
  StringRef(StringRef&& sr);
  StringRef& operator=(StringRef&& sr);

  char const* ptr() const;

  // Cause this StringRef to always hold a String, not just a ptr.  Will be
  // cheap if StringRef was constructed with an rvalue.
  void hold();

  // Gets a const& to the held String if it is held, UB otherwise.
  String const& held() const;

  bool operator==(StringRef const& ref) const;

private:
  void set(String s);

  Maybe<String> m_held;
  char const* m_ptr;
};

std::ostream& operator<<(std::ostream& os, StringRef const& ref);

template<>
struct hash<StringRef> {
  size_t operator()(StringRef const& s) const;
  CharHasher<char> impl;
};

struct CaseInsensitiveStringHash {
  size_t operator()(StringRef const& s) const;
  CharHasher<char> impl;
};

struct CaseInsensitiveStringCompare {
  bool operator()(StringRef const& lhs, StringRef const& rhs) const;
};

// A sort of version of HashMap<String, T> that is much faster than the normal
// one for a great many use cases.  For example, normal HashMap<String, T>
// forces the user to copy from a char* into a String to do basic things like
// search.  This class avoids that by nicely supporting char const* as a key
// type without inducing copying.
//
// It is *almost*, but not *completely* compatible with a normal Map type.  In
// particular, iteration is difficult because the key type held internally
// *cannot* be String when using std::unordered_map as the base map.  As a
// result, iteration is somewhat weird in that an iterator always returns a
// pair of refs, so something like:
//
// for (auto p : map)
//   p.second.mutate();
// 
// Will affect the copy in the map, and the second value in the pair is always
// mutable whenever non-const iterators are used.
template<typename MappedT, typename HashT = CharHasher<char>, typename ComparatorT = std::equal_to<StringRef>>
class StringMap {
private:
public:
  typedef String key_type;
  typedef MappedT mapped_type;
  typedef pair<key_type, mapped_type> value_type;

  typedef HashT Hash;
  typedef ComparatorT Comparator;

  struct InternalHash {
    bool operator()(StringRef const& s) const;
    Hash hash;
  };

  struct InternalComparator {
    bool operator()(StringRef const& a, StringRef const& b) const;
    Comparator comparator;
  };

  typedef std::unordered_map<StringRef, mapped_type, InternalHash, InternalComparator> InternalMap;
  typedef typename InternalMap::iterator IteratorBase;
  typedef typename InternalMap::const_iterator ConstIteratorBase;

  template<typename T>
  struct ArrowProxy {
    T* operator->();
    T t;
  };

  struct iterator : public IteratorBase {
    iterator();
    iterator(IteratorBase i);

    IteratorBase const& base() const;

    ArrowProxy<pair<key_type const&, mapped_type&> const> operator->() const;
    pair<key_type const&, mapped_type&> const operator*() const;
  };

  struct const_iterator : public ConstIteratorBase {
    const_iterator();
    const_iterator(ConstIteratorBase i);

    ConstIteratorBase const& base() const;

    ArrowProxy<pair<key_type const&, mapped_type const&> const> operator->() const;
    pair<key_type const&, mapped_type const&> const operator*() const;
  };

  template<typename Container>
  static StringMap from(Container const& m) {
    return StringMap(m.begin(), m.end());
  }

  StringMap();
  StringMap(StringMap const& map);
  StringMap(StringMap&& map);
  template<typename InputIterator>
  StringMap(InputIterator beg, InputIterator end);
  StringMap(std::initializer_list<value_type> list);

  StringList keys() const;
  List<mapped_type> values() const;
  List<value_type> pairs() const;

  bool contains(StringRef const& k) const;

  mapped_type& get(StringRef const& k);
  mapped_type const& get(StringRef const& k) const;

  mapped_type value(StringRef const& k, mapped_type d = mapped_type()) const;
  Maybe<mapped_type const&> maybe(StringRef const& k) const;
  Maybe<mapped_type&> maybe(StringRef const& k);

  mapped_type& operator[](StringRef k);

  StringMap& operator=(StringMap const& map);
  StringMap& operator=(StringMap&& map);

  pair<iterator, bool> insert(value_type v);

  bool insert(StringRef k, mapped_type v);
  iterator insert(iterator pos, value_type v);

  bool remove(StringRef const& k);

  mapped_type take(StringRef const& k);

  const_iterator cbegin() const;
  const_iterator cend() const;

  const_iterator begin() const;
  const_iterator end() const;

  iterator begin();
  iterator end();

  size_t size() const;

  iterator erase(iterator i);
  size_t erase(key_type const& k);

  iterator find(StringRef const& k);

  const_iterator find(StringRef const& k) const;

  void clear();

  bool empty() const;

  // Appends all values of given map into this map.  If overwite is false, then
  // skips values that already exist in this map.  Returns false if any keys
  // previously existed.
  template<typename MapType>
  bool merge(MapType const& m, bool overwrite = false);

  mapped_type& add(StringRef k, mapped_type v);

  bool operator==(StringMap const& m) const;

private:
  InternalMap m_map;
};

typedef HashSet<String> StringSet;

template<typename MappedT, typename HashT, typename ComparatorT>
std::ostream& operator<<(std::ostream& os, StringMap<MappedT, HashT, ComparatorT> const& ref);

inline size_t hash<StringRef>::operator()(StringRef const& s) const {
  return impl(s.ptr());
};

inline size_t CaseInsensitiveStringHash::operator()(StringRef const& s) const {
  auto p = s.ptr();
  size_t h = 0;
  while (*p) {
    String::Char c;
    p += utf8DecodeChar(p, &c);
    h = h * 101 + String::toLower(c);
  }
  return h;
}

inline bool CaseInsensitiveStringCompare::operator()(StringRef const& a, StringRef const& b) const {
  auto lhs = a.ptr();
  auto rhs = b.ptr();
  while (*lhs && *rhs) {
    String::Char lhsc;
    lhs += utf8DecodeChar(lhs, &lhsc);

    String::Char rhsc;
    rhs += utf8DecodeChar(rhs, &rhsc);

    if (String::toLower(lhsc) != String::toLower(rhsc))
      return false;
  }
  return !*lhs && !*rhs;
}

template<typename Container>
StringList& StringList::appendAll(Container&& list) {
  Base::appendAll(std::forward<Container&&>(list));
  return *this;
}

template<typename Container>
StringList& StringList::prependAll(Container&& list) {
  Base::prependAll(std::forward<Container&&>(list));
  return *this;
}

template<typename Container>
StringList& StringList::insertAll(size_t pos, Container const& l) {
  Base::insertAll(pos, std::forward<Container&&>(l));
  return *this;
}

inline StringRef::StringRef(StringRef&& sr) {
  operator=(std::move(sr));
}

inline StringRef::StringRef(String const& s)
  : m_ptr(s.utf8Ptr()) {}

inline StringRef::StringRef(String&& s) {
  set(std::move(s));
}

inline StringRef::StringRef(std::string const& s)
  : m_ptr(s.c_str()) {}

inline StringRef::StringRef(std::string&& s) {
  set(std::move(s));
}

inline StringRef::StringRef(char const* p)
  : m_ptr(p) {}

inline StringRef& StringRef::operator=(StringRef&& sr) {
  if (sr.m_held) {
    set(sr.m_held.take());
  } else {
    m_held.reset();
    m_ptr = sr.m_ptr;
  }
  return *this;
}

inline char const* StringRef::ptr() const {
  return m_ptr;
}

inline void StringRef::hold() {
  if (!m_held)
    set(String(m_ptr));
}

inline String const& StringRef::held() const {
  return *m_held;
}

inline bool StringRef::operator==(StringRef const& ref) const {
  if (m_held) {
    if (ref.m_held)
      return *m_held == *ref.m_held;
  }
  return strcmp(ptr(), ref.ptr()) == 0;
}

inline void StringRef::set(String s) {
  m_held = std::move(s);
  m_ptr = m_held->utf8Ptr();
}

inline std::ostream& operator<<(std::ostream& os, StringRef const& ref) {
  os << ref.ptr();
  return os;
}

template<typename MappedT, typename HashT, typename ComparatorT>
bool StringMap<MappedT, HashT, ComparatorT>::InternalHash::operator()(StringRef const& s) const {
  return hash(s.ptr());
}

template<typename MappedT, typename HashT, typename ComparatorT>
bool StringMap<MappedT, HashT, ComparatorT>::InternalComparator::operator()(StringRef const& a, StringRef const& b) const {
  return comparator(a.ptr(), b.ptr());
}

template<typename MappedT, typename HashT, typename ComparatorT>
template<typename T>
T* StringMap<MappedT, HashT, ComparatorT>::ArrowProxy<T>::operator->() {
  return &t;
}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>::iterator::iterator() {}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>::iterator::iterator(IteratorBase i)
  : IteratorBase(std::move(i)) {}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::iterator::base() const -> IteratorBase const& {
  return *this;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::iterator::operator->() const -> ArrowProxy<pair<key_type const&, mapped_type&> const> {
  return {pair<key_type const&, mapped_type&>(base()->first.held(), base()->second)};
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::iterator::operator*() const -> pair<key_type const&, mapped_type&> const {
  return {base()->first.held(), base()->second};
}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>::const_iterator::const_iterator() {}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>::const_iterator::const_iterator(ConstIteratorBase i)
  : ConstIteratorBase(std::move(i)) {}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::const_iterator::base() const -> ConstIteratorBase const& {
  return *this;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::const_iterator::operator->() const -> ArrowProxy<pair<key_type const&, mapped_type const&> const> {
  return {pair<key_type const&, mapped_type const&>(base()->first.held(), base()->second)};
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::const_iterator::operator*() const -> pair<key_type const&, mapped_type const&> const {
  return {base()->first.held(), base()->second};
}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>::StringMap() {}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>::StringMap(StringMap const& map) {
  for (auto const& p : map)
    insert(p);
}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>::StringMap(StringMap&& map) {
  m_map = std::move(map.m_map);
}

template<typename MappedT, typename HashT, typename ComparatorT>
template<typename InputIterator>
StringMap<MappedT, HashT, ComparatorT>::StringMap(InputIterator beg, InputIterator end) {
  for (auto i = beg; i != end; ++i)
    insert(*i);
}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>::StringMap(std::initializer_list<value_type> list) {
  for (auto const& p : list)
    insert(p);
}

template<typename MappedT, typename HashT, typename ComparatorT>
StringList StringMap<MappedT, HashT, ComparatorT>::keys() const {
  StringList keys;
  for (auto const& p : *this)
    keys.append(p.first);
  return keys;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::values() const -> List<mapped_type> {
  List<mapped_type> values;
  for (auto const& p : *this)
    values.append(p.second);
  return values;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::pairs() const -> List<value_type> {
  List<value_type> pairs;
  for (auto const& p : *this)
    pairs.append(p);
  return pairs;
}

template<typename MappedT, typename HashT, typename ComparatorT>
bool StringMap<MappedT, HashT, ComparatorT>::contains(StringRef const& k) const {
  return m_map.find(k) != m_map.end();
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::get(StringRef const& k) -> mapped_type& {
  auto i = m_map.find(k);
  if (i == m_map.end())
    throw MapException(strf("Key '%s' not found in StringMap::get()", k.ptr()));

  return i->second;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::get(StringRef const& k) const -> mapped_type const& {
  return const_cast<StringMap*>(this)->get(k);
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::value(StringRef const& k, mapped_type d) const -> mapped_type {
  auto i = m_map.find(k);
  if (i == m_map.end())
    return std::move(d);
  else
    return i->second;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::maybe(StringRef const& k) const -> Maybe<mapped_type const&> {
  auto i = m_map.find(k);
  if (i == m_map.end())
    return {};
  else
    return i->second;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::maybe(StringRef const& k) -> Maybe<mapped_type&> {
  auto i = m_map.find(k);
  if (i == m_map.end())
    return {};
  else
    return i->second;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::operator[](StringRef k) -> mapped_type& {
  auto i = m_map.find(k);
  if (i == m_map.end()) {
    k.hold();
    i = m_map.insert(pair<StringRef, mapped_type>(std::move(k), mapped_type())).first;
  }

  return i->second;
}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>& StringMap<MappedT, HashT, ComparatorT>::operator=(StringMap const& map) {
  clear();
  for (auto const& p : map)
    insert(p);

  return *this;
}

template<typename MappedT, typename HashT, typename ComparatorT>
StringMap<MappedT, HashT, ComparatorT>& StringMap<MappedT, HashT, ComparatorT>::operator=(StringMap&& map) {
  m_map = std::move(map.m_map);
  return *this;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::insert(value_type v) -> std::pair<iterator, bool> {
  return m_map.insert(pair<StringRef, mapped_type>(std::move(v.first), std::move(v.second)));
}

template<typename MappedT, typename HashT, typename ComparatorT>
bool StringMap<MappedT, HashT, ComparatorT>::insert(StringRef k, mapped_type v) {
  k.hold();
  return m_map.insert(pair<StringRef, mapped_type>(std::move(k), std::move(v))).second;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::insert(iterator pos, value_type v) -> iterator {
  return m_map.insert(pos, pair<StringRef, mapped_type>(std::move(v.first), std::move(v.second)));
}

template<typename MappedT, typename HashT, typename ComparatorT>
bool StringMap<MappedT, HashT, ComparatorT>::remove(StringRef const& k) {
  auto i = m_map.find(k);
  if (i != m_map.end()) {
    m_map.erase(i);
    return true;
  } else {
    return false;
  }
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::take(StringRef const& k) -> mapped_type {
  auto i = m_map.find(k);
  if (i != m_map.end()) {
    MappedT v = std::move(i->second);
    m_map.erase(i);
    return std::move(v);
  } else {
    throw MapException(strf("Key '%s' not found in StringMap::take()", k));
  }
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::cbegin() const -> const_iterator {
  return m_map.cbegin();
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::cend() const -> const_iterator {
  return m_map.cend();
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::begin() const -> const_iterator {
  return m_map.begin();
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::end() const -> const_iterator {
  return m_map.end();
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::begin() -> iterator {
  return m_map.begin();
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::end() -> iterator {
  return m_map.end();
}

template<typename MappedT, typename HashT, typename ComparatorT>
size_t StringMap<MappedT, HashT, ComparatorT>::size() const {
  return m_map.size();
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::erase(iterator i) -> iterator {
  return m_map.erase(i);
}

template<typename MappedT, typename HashT, typename ComparatorT>
size_t StringMap<MappedT, HashT, ComparatorT>::erase(key_type const& k) {
  if (remove(k))
    return 1;
  return 0;
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::find(StringRef const& k) -> iterator {
  return m_map.find(k);
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::find(StringRef const& k) const -> const_iterator {
  return m_map.find(k);
}

template<typename MappedT, typename HashT, typename ComparatorT>
void StringMap<MappedT, HashT, ComparatorT>::clear() {
  m_map.clear();
}

template<typename MappedT, typename HashT, typename ComparatorT>
bool StringMap<MappedT, HashT, ComparatorT>::empty() const {
  return m_map.empty();
}

template<typename MappedT, typename HashT, typename ComparatorT>
template<typename MapType>
bool StringMap<MappedT, HashT, ComparatorT>::merge(MapType const& m, bool overwrite) {
  return mapMerge(*this, m, overwrite);
}

template<typename MappedT, typename HashT, typename ComparatorT>
auto StringMap<MappedT, HashT, ComparatorT>::add(StringRef k, mapped_type v) -> mapped_type& {
  k.hold();
  auto p = m_map.insert(pair<StringRef, mapped_type>(std::move(k), std::move(v)));
  if (!p.second)
    throw MapException(strf("Entry with key '%s' already present.", outputAny(k)));
  else
    return p.first->second;
}

template<typename MappedT, typename HashT, typename ComparatorT>
bool StringMap<MappedT, HashT, ComparatorT>::operator==(StringMap const& m) const {
  return mapsEqual(*this, m);
}

template<typename MappedT, typename HashT, typename ComparatorT>
std::ostream& operator<<(std::ostream& os, StringMap<MappedT, HashT, ComparatorT> const& ref) {
  printMap(os, ref);
  return os;
}

}

#endif
