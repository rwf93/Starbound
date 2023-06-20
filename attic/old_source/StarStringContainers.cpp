#include "StarStringContainers.hpp"

#include <cctype>
#include "pcre.h"

namespace Star {

StringList::StringList()
  : Base() {}

StringList::StringList(const Base& l)
  : Base(l) {}

StringList::StringList(Base&& l)
  : Base(std::move(l)) {}

StringList::StringList(size_t n, String::Char const* const * list) {
  for (size_t i = 0; i < n; ++i)
    append(String(list[i]));
}

StringList::StringList(size_t len, const String& s1)
  : Base(len, s1) {
}

StringList::StringList(std::initializer_list<String> list)
  : Base(list) {
}

bool StringList::contains(const String& s, String::CaseSensitivity cs) const {
  for (const_iterator i = begin(); i != end(); ++i) {
    if (s.compare(*i, cs) == 0)
      return true;
  }
  return false;
}

StringList StringList::trimAll(const String& pattern) const {
  StringList r;
  for (auto const& s : *this)
    r.append(s.trim(pattern));
  return r;
}

String StringList::join(const String& separator) const {
  String joinedString;
  for (const_iterator i = begin(); i != end(); ++i) {
    if (i != begin())
      joinedString += separator;
    joinedString += *i;
  }

  return joinedString;
}

StringList& StringList::append(String const& e) {
  Base::append(e);
  return *this;
}

StringList& StringList::append(String&& e) {
  Base::append(std::move(e));
  return *this;
}

StringList& StringList::prepend(String const& e) {
  Base::prepend(e);
  return *this;
}

StringList& StringList::prepend(String&& e) {
  Base::prepend(std::move(e));
  return *this;
}

StringList& StringList::remove(String const& e) {
  Base::remove(e);
  return *this;
}

StringList& StringList::erase(size_t index) {
  Base::erase(index);
  return *this;
}

StringList& StringList::erase(size_t begin, size_t end) {
  Base::erase(begin, end);
  return *this;
}

StringList& StringList::insert(size_t pos, String const& e) {
  Base::insert(pos, e);
  return *this;
}

StringList& StringList::set(size_t pos, String const& e) {
  Base::set(pos, e);
  return *this;
}

StringList StringList::slice(SliceIndex a, SliceIndex b, int i) const {
  return Star::slice(*this, a, b, i);
}

std::ostream& operator<<(std::ostream& os, const StringList& list) {
  os << "(";
  for (auto i = list.begin(); i != list.end(); ++i) {
    if (i != list.begin())
      os << ", ";

    os << '\'' << *i << '\'';
  }
  os << ")";
  return os;
}

}
