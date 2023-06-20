#ifndef STAR_RUTABLE_HPP
#define STAR_RUTABLE_HPP

#include "StarInterpolation.hpp"
#include "StarArray.hpp"

#include <vector>

namespace Star {

template<typename Element, typename Position, size_t Rank, size_t StorageRank>
struct Storage;

template<typename Element, typename Position, size_t Rank>
struct Storage<Element, Position, Rank, 1> {
  std::vector<Position> positions;
  std::vector<Element> elements;

  template<typename IndexList>
  void pushDim(Position const& position, size_t dim, IndexList const& index) {
    starAssert(false);
  }

  template<typename IndexList>
  void pushElement(Position const& position, Element const& element, IndexList const& index) {
    positions.push_back(position);
    elements.push_back(element);
  }

  template<typename IndexList>
  Element const& get(IndexList const& index) const {
    return elements[index[Rank - 1]];
  }
};

template<typename Element, typename Position, size_t Rank, size_t StorageRank>
struct Storage {
  typedef Storage<Element, Position, Rank, StorageRank - 1> Stored;

  template<typename IndexList>
  void pushDim(Position const& position, size_t dim, IndexList const& index) {
    if (dim == Rank - StorageRank) {
      positions.push_back(position);
      elements.push_back(Stored());
    } else {
      elements[index[Rank - StorageRank]].pushDim(position, dim, index);
    }
  }

  template<typename IndexList>
  void pushElement(Position const& position, Element const& element, IndexList const& index) {
    elements[index[Rank - StorageRank]].pushElement(position, element, index);
  }

  template<typename IndexList>
  Element const& get(IndexList const& index) const {
    return elements[index[Rank - StorageRank]].get(index);
  }

  std::vector<Position> positions;
  std::vector<Stored> elements;
};

template<typename ElementT, typename PositionT, size_t RankN>
class RUTable {
public:
  typedef ElementT Element;
  typedef PositionT Position;
  static size_t const Rank = RankN;

  typedef Array<Position, Rank> PositionList;
  typedef Array<size_t, Rank> IndexList;
  typedef Array<size_t, Rank> SizeList;

  RUTable() : m_curDim(0), m_curIndex(IndexList::filled(0)) {}

  void pushDim(Position const& position) {
    m_storage.pushDim(position, m_curDim, m_curIndex);
    ++m_curDim;
  }

  void pushElement(Position const& position, Element const& element) {
    m_storage.pushElement(position, element, m_curIndex);
    ++m_curIndex[Rank - 1];
  }

  void popDim() {
    m_curIndex[m_curDim] = 0;
    --m_curDim;
    ++m_curIndex[m_curDim];
  }

  Element const& get(IndexList const& index) const {
    return m_storage.get(index);
  }

private:
  size_t m_curDim;
  IndexList m_curIndex;
  Storage<Element, Position, Rank, Rank> m_storage;
};

}

#endif
