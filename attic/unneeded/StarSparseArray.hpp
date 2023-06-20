#ifndef STAR_SPARSE_ARRAY_HPP
#define STAR_SPARSE_ARRAY_HPP

#include <vector>
#include <limits>
#include <algorithm>

namespace Star {

// Sparse "infinite" (mapped to every possible value of IndexT) 1d array.
// Run-length compressed.  Every element is always "set" -- starts out with
// default constructed value of DataT.  Stays run-length compressed as elements
// are added or removed.  CompareT should compare equality of two DataT,
// defaults to operator==.  IndexT must be an integer-like type.
template<typename DataT, typename IndexT, typename CompareT = std::equal_to<DataT>,
  typename IndexLimitsT = std::numeric_limits<IndexT> >
class SparseArray {
public:
  typedef DataT Data;
  typedef IndexT Index;
  typedef CompareT Compare;
  typedef IndexLimitsT IndexLimits;

  static Index minIndex() {
    return IndexLimits::min();
  }

  static Index maxIndex() {
    return IndexLimits::max();
  }

  struct Element {
    Element(Index const& i, Data const& v) : index(i), value(v) {}
    Index index;
    Data value;

    bool operator<(Element const& e) const {
      return index < e.index;
    }

    bool operator<(Index const& i) const {
      return index < i;
    }
  };

  struct ElementCompare {
    ElementCompare(Compare const& r) : compare(r) {}
    bool operator()(Element const& e1, Element const& e2) const {
      return e1.index == e2.index && compare(e1.value, e2.value);
    }
    Compare const& compare;
  };

  typedef std::vector<Element> ElementList;
  typedef typename ElementList::const_iterator ConstElementIterator;
  typedef typename ElementList::iterator ElementIterator;

  ElementIterator rangeForIndex(Index const& i) {
    ElementIterator it = std::lower_bound(m_elements.begin(), m_elements.end(), i);

    if (it != m_elements.end()) {
      if (it->index != i)
        --it;
    } else {
      --it;
    }
    return it;
  }

  ConstElementIterator rangeForIndex(Index const& i) const {
    return const_cast<SparseArray*>(this)->rangeForIndex(i);
  }

  ElementList const& elements() const {
    return m_elements;
  }

  SparseArray(Compare const& c = Compare()) : m_compare(c) {
    m_elements.insert(m_elements.begin(), Element(minIndex(), Data()));
  }

  SparseArray(Data const& d, Compare const& c = Compare()) : m_compare(c) {
    m_elements.insert(m_elements.begin(), Element(minIndex(), d));
  }

  Data const& get(Index const& i) const {
    return rangeForIndex(i)->value;
  }

  void set(Index const& i, Data const& d) {
    fill(i, i, d);
  }

  void fill(Data const& d) {
    fill(minIndex(), maxIndex(), d);
  }

  // Find the ranges in the array that include begin and end, brange, and erange, as well as the length of erange.
  // Delete every range in between, but not including brange and erange.
  // Find the distance between begin index and brange start, and end index and erange end (bdiff, ediff)
  // Store value of erange.
  // If erange is not the same range as brange, delete erange.
  // If brange value is equal to inserted value, then don't need to change brange.
  // Else, if bdiff is 0, then check the range previous to brange.  If values are equal, erase brange, else replace brange,
  // Else, insert (begin, value) after brange.
  // Now, if ediff is 0, then check if next range value is equal to this value, if so, erase.
  // Else, insert (end+1, erange.value) after previously inserted range.
  // Sets *inclusive* range.
  void fill(Index const& begin, Index const& end, Data const& d) {
    ElementIterator brangeIt = rangeForIndex(begin);
    ElementIterator erangeIt = rangeForIndex(end);

    if (brangeIt != erangeIt) {
      ++brangeIt;
      if (brangeIt != erangeIt) {
        erangeIt = m_elements.erase(brangeIt, erangeIt);
        brangeIt = erangeIt;
      }
      --brangeIt;
    }

    Index erangeEnd;
    ElementIterator pastIt(erangeIt);
    ++pastIt;
    if (pastIt == m_elements.end())
      erangeEnd = maxIndex();
    else
      erangeEnd = pastIt->index - 1;

    bool bdiff = begin != brangeIt->index;
    bool ediff = erangeEnd != end;

    Data erangeVal = erangeIt->value;
    if (brangeIt != erangeIt) {
      erangeIt = m_elements.erase(erangeIt);
      brangeIt = erangeIt;
      --brangeIt;
    }

    // insertIt should point to range after erange after this.
    ElementIterator insertIt = brangeIt;
    if (!m_compare(insertIt->value, d)) {
      if (!bdiff) {
        if (insertIt != m_elements.begin()) {
          ElementIterator prevIt = insertIt;
          --prevIt;
          if (m_compare(prevIt->value, d)) {
            insertIt = m_elements.erase(insertIt);
          } else {
            insertIt->value = d;
            ++insertIt;
          }
        } else {
          insertIt->value = d;
          ++insertIt;
        }
      } else {
        ++insertIt;
        insertIt = m_elements.insert(insertIt, Element(begin, d));
        ++insertIt;
      }
    } else {
      ++insertIt;
    }

    if (!ediff) {
      if (insertIt != m_elements.end()) {
        if (m_compare(insertIt->value, d))
          m_elements.erase(insertIt);
      }
    } else  {
      ElementIterator prevIt = insertIt;
      --prevIt;
      if (!m_compare(prevIt->value, erangeVal))
        m_elements.insert(insertIt, Element(end+1, erangeVal));
    }
  }

  template<typename Func>
  void transform(Index const& begin, Index const& end, Func f) {
    ElementIterator brangeIt = rangeForIndex(begin);
    ElementIterator erangeIt = rangeForIndex(end);
    Data erangeVal = erangeIt->value;

    Index erangeEnd;
    ElementIterator pastIt(erangeIt);
    ++pastIt;
    if (pastIt == m_elements.end())
      erangeEnd = maxIndex();
    else
      erangeEnd = pastIt->index - 1;

    bool bdiff = begin != brangeIt->index;
    bool ediff = erangeEnd != end;

    if (bdiff) {
      Data d = f(brangeIt->value);
      if (!m_compare(d, brangeIt->value)) {
        brangeIt = m_elements.insert(++brangeIt, Element(begin, d));
      }
    } else {
      brangeIt->value = f(brangeIt->value);
    }

    ElementIterator last = brangeIt;
    ElementIterator cur = brangeIt;
    ++cur;
    while(true) {
      if (cur == m_elements.end())
        break;
      if (cur->index > erangeEnd)
        break;

      cur->value = f(cur->value);
      if (m_compare(cur->value, last->value)) {
        cur = m_elements.erase(cur);
        last = cur;
        --last;
      } else {
        last = cur++;
      }
    }

    if (!ediff) {
      return;
    } else {
      if (!m_compare(last->value, erangeVal))
        m_elements.insert(cur, Element(end+1, erangeVal));
    }
  }

  class Proxy {
  public:
    Proxy(SparseArray& s, Index const& i) : ref(s), index(i) {}
    operator Data const& () { return ref.get(index); }
    Proxy& operator=(Data const& d) {
      ref.set(index, d);
      return *this;
    }

  private:
    SparseArray& ref;
    Index const& index;
  };

  Data const& operator()(Index const& i) const {
    return get(i);
  }

  Proxy operator()(Index const& i) {
    return Proxy(*this, i);
  }

  bool operator==(SparseArray const& sa) const {
    if (m_elements.size() != sa.m_elements.size())
      return false;
    return std::equal(m_elements.begin(), m_elements.end(), sa.m_elements.begin(), ElementCompare(m_compare));
  }

  // Force compression (useful if m_compare is changed.)
  void compress() {
    for (ElementIterator i = m_elements.begin(); i != m_elements.end(); ++i) {
      if (i != m_elements.begin()) {
        ElementIterator p = i;
        --p;
        if (m_compare(p->value, i->value)) {
          i = m_elements.erase(i);
          --i;
        }
      }
    }
  }

  void setCompare(Compare c) {
    m_compare = c;
  }

  Compare const& compare() const {
    return m_compare;
  }

private:
  ElementList m_elements;
  Compare m_compare;
};

// 2 dimensional SparseArray
template<typename DataT, typename IndexT, typename CompareT = std::equal_to<DataT>,
  typename IndexLimitsT = std::numeric_limits<IndexT> >
class SparseGrid {
public:
  typedef DataT Data;
  typedef IndexT Index;
  typedef CompareT Compare;
  typedef IndexLimitsT IndexLimits;
  typedef SparseArray<Data, Index, Compare, IndexLimitsT> ArrayElement;

  // Compare rows, ignoring rows with complexity greater than 'limit'
  struct RowCompare {
    RowCompare(size_t cl = std::numeric_limits<size_t>::max()) : limit(cl) {}

    bool operator()(ArrayElement const& a, ArrayElement const& b) const {
      if (a.elements().size() > limit || b.elements().size() > limit)
        return false;
      else
        return a == b;
    }

    size_t limit;
  };

  typedef SparseArray<ArrayElement, Index, RowCompare, IndexLimits> RowArray;

  static Index minIndex() {
    return IndexLimits::min();
  }

  static Index maxIndex() {
    return IndexLimits::max();
  }

  SparseGrid(size_t compareLimit = 1) : m_rows(Data(), RowCompare(compareLimit)) {}

  size_t compareLimit() const {
    return m_rows.compare().limit;
  }

  void setCompareLimit(size_t cl) {
    m_rows.setCompare(RowCompare(cl));
  }

  Data const& get(Index const& i, Index const& j) const {
    return m_rows.get(i).get(j);
  }

  void set(Index const& i, Index const& j, Data const& d) {
    ArrayElement e = m_rows.get(i);
    e(j) = d;
    m_rows.set(i, e);
  }

  Data const& operator()(Index const& i, Index const& j) const {
    return m_rows.get(i)(j);
  }

  friend class Proxy;
  class Proxy {
  public:
    Proxy(SparseGrid& s, Index const& i, Index const& j) : ref(s), index_i(i), index_j(j) {}
    operator Data const&() const { return ref.get(index_i, index_j); }

    Proxy& operator=(Data const& d) {
      ref.set(index_i, index_j, d);
      return *this;
    }

  private:
    SparseGrid& ref;
    Index const& index_i;
    Index const& index_j;
  };

  Proxy operator()(Index const& i, Index const& j) {
    return Proxy(*this, i, j);
  }

  class FillTransform {
  public:
    FillTransform(Index const& min, Index const& max, Data const& d) : m_min(min), m_max(max), m_val(d) {}

    ArrayElement operator()(ArrayElement const& e) const {
      ArrayElement ne(e);
      ne.fill(m_min, m_max, m_val);
      return ne;
    }

  private:
    Index const& m_min;
    Index const& m_max;
    Data const& m_val;
  };

  void fill(Data const& d) {
    fill(minIndex(), maxIndex(), ArrayElement(d));
  }

  void fill(Index const& rmin, Index const& rmax, ArrayElement const& row) {
    m_rows.fill(rmin, rmax, row);
  }

  void fill(Index const& rmin, Index const& cmin, Index const& rmax, Index const& cmax, Data const& d) { 
    m_rows.transform(rmin, rmax, FillTransform(cmin, cmax, d));
  }

  // Full compression, turns off RowCompare limit temporarily.
  void compress() {
    size_t oldCompareLimit = compareLimit();
    setCompareLimit(std::numeric_limits<size_t>::max());
    m_rows.compress();
    setCompareLimit(oldCompareLimit);
  }

  RowArray const& rows() const {
    return m_rows;
  }

  size_t elementCount() const {
    size_t count = 0;
    for (typename RowArray::ConstElementIterator i = m_rows.begin(); i != m_rows.end(); ++i)
      count += m_rows.elements.count();
    return count;
  }

private:
  RowArray m_rows; 
};

}

#endif
