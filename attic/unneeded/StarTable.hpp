#ifndef STAR_TABLE_HPP
#define STAR_TABLE_HPP

#include "StarMultiArrayInterpolator.hpp"

namespace Star {

// Provides a method for storing, retrieving, and interpolating even n-variate
// data.  Access time is O(n), where n is the GridRank of the Table
// Interpolation time for piecewise is O(n), O(2^n) for linear,
// O(4^n) for cubic.
template<typename ElementT, typename PositionT, size_t RankN>
class Table {
public:
  typedef ElementT Element;
  typedef PositionT Position;
  static size_t const Rank = RankN;

  typedef Star::MultiArray<ElementT, RankN> MultiArray;

  typedef Star::MultiArrayInterpolator2<MultiArray, Position> Interpolator2;
  typedef Star::MultiArrayInterpolator4<MultiArray, Position> Interpolator4;
  typedef Star::MultiArrayPiecewiseInterpolator<MultiArray, Position> PiecewiseInterpolator;

  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 2> WeightList2;
  typedef Array<Position, 4> WeightList4;
  typedef typename MultiArray::SizeList SizeList;
  typedef typename MultiArray::IndexList IndexList;

  typedef std::function<WeightList2(Position)> WeightFunction2;
  typedef std::function<WeightList4(Position)> WeightFunction4;
  typedef std::function<Element(PositionList const&)> InterpolateFunction;

  struct TableRange {
    Position min;
    Position max;
  };

  Table() {}

  Table(SizeList const& size, PositionList const& min, PositionList const& max) {
    setDimensions(size, min, max);
  }

  MultiArray const& array() const {
    return m_array;
  }

  MultiArray& array() {
    return m_array;
  }

  void setRanges(PositionList const& min, PositionList const& max) {
    for (size_t i = 0; i < Rank; ++i) {
      m_ranges[i].min = min[i];
      m_ranges[i].max = max[i];
    }
  }

  void setRange(size_t dim, Position min, Position max) {
    m_ranges[dim].min = min;
    m_ranges[dim].max = max;
  }

  TableRange const& range(size_t dim) const {
    return m_ranges[dim];
  }

  TableRange& range(size_t dim) {
    return m_ranges[dim];
  }

  void setDimension(size_t dim, size_t size, Position min, Position max) {
    SizeList sizes = m_array.sizes();
    sizes[dim] = size;
    m_array.resize(sizes);
    setRange(dim, min, max);
  }

  void setDimensions(SizeList const& sizes, PositionList const& min, PositionList const& max) {
    m_array.resize(sizes);
    setRanges(min, max);
  }

  void set(IndexList const& index, Element const& element) {
    m_array.set(index, element);
  }

  Element const& get(IndexList const& index) const {
    return m_array(index);
  }

  void set2TermInterpolation(WeightFunction2 const& weightFunction, BoundMode boundMode) {
    Interpolator2 interpolator2(weightFunction, boundMode);
    m_interpolateFunction = [=](PositionList const& position) {
        return interpolator2.interpolate(this->m_array, this->toIndexSpace(position));
      };
  }

  void set4TermInterpolation(WeightFunction4 const& weightFunction, BoundMode boundMode) {
    Interpolator4 interpolator4(weightFunction, boundMode);
    m_interpolateFunction = [=](PositionList const& position) {
        return interpolator4.interpolate(this->m_array, this->toIndexSpace(position));
      };
  }

  void setPiecewiseInterpolation(WeightFunction2 const& weightFunction, BoundMode boundMode) {
    PiecewiseInterpolator piecewiseInterpolator(weightFunction, boundMode);
    m_interpolateFunction = [=](PositionList const& position) {
        return piecewiseInterpolator.interpolate(this->m_array, this->toIndexSpace(position));
      };
  }

  Element interpolate(PositionList const& coord) const {
    return m_interpolateFunction(coord);
  }

  Element operator()(PositionList const& coord) const {
    return interpolate(coord);
  }

  // op should take a PositionList parameter and return an element.
  template<typename OpType>
  void eval(OpType op) {
    m_array.eval(EvalWrapper<OpType>(op, *this));
  }

protected:
  PositionList toIndexSpace(PositionList const& coord) const {
    PositionList indexCoord;
    for (size_t i = 0; i < Rank; ++i)
      indexCoord[i] = (coord[i] - m_ranges[i].min) / delta(i);
    return indexCoord;
  }

  Position toTableSpace(Position pos, size_t dim) const {
    return m_ranges[dim].min + pos * delta(dim);
  }

  Position delta(size_t dim) const {
    return (m_ranges[dim].max - m_ranges[dim].min) / (m_array.size(dim) - 1);
  }

private:
  enum InterpolateMode {
    Interpolate2Mode,
    Interpolate4Mode,
    PiecewiseInterpolateMode
  };

  template<typename OpType>
  struct EvalWrapper {
    EvalWrapper(OpType &o, Table const& t) : op(o), table(t) {}

    template<typename IndexList>
    Element operator()(IndexList const& indexList) {
      PositionList rangeList;
      for (size_t i = 0; i < Rank; ++i)
        rangeList[i] = table.toTableSpace(indexList[i], i);

      return op(rangeList);
    }

    OpType& op;
    Table const& table;
  };

  InterpolateFunction m_interpolateFunction;
  Array<TableRange, Rank> m_ranges;
  MultiArray m_array;
};

typedef Table<float, float, 2> Table2F;
typedef Table<double, double, 2> Table2D;

typedef Table<float, float, 3> Table3F;
typedef Table<double, double, 3> Table3D;

typedef Table<float, float, 4> Table4F;
typedef Table<double, double, 4> Table4D;

}

#endif
