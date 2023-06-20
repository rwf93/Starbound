#ifndef STAR_UTABLE_HPP
#define STAR_UTABLE_HPP

#include "StarTable.hpp"

namespace Star {

// Provides a method for storing, retrieving, and interpolating uneven
// n-variate data.  Access times involve a binary search over the domain of
// each dimension, so is O(log(n)*m) where n is the size of the largest
// dimension, and m is the table_rank.
// 
// BoundMode Wrap makes little mathematical sense for UTable.
template<typename ElementT, typename PositionT, size_t RankN>
class UTable {
public:
  typedef ElementT Element;
  typedef PositionT Position;
  static size_t const Rank = RankN;

  typedef Star::MultiArray<ElementT, RankN> MultiArray;

  typedef Star::MultiArrayInterpolator2<MultiArray, Position> Interpolator2;
  typedef Star::MultiArrayPiecewiseInterpolator<MultiArray, Position> PiecewiseInterpolator;

  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 2> WeightList2;
  typedef Array<Position, 4> WeightList4;
  typedef typename MultiArray::SizeList SizeList;
  typedef typename MultiArray::IndexList IndexList;
  typedef std::vector<Position> Range;
  typedef Vector<Range, Rank> RangeList;

  typedef std::function<WeightList2(Position)> WeightFunction2;

  // Evenly spaced table that UTable can be resampled into.
  typedef Table<Element, Position, Rank> ResampledTable;

  // Set input ranges on a particular dimension.  Will resize underlying storage to fit range.
  void setRange(std::size_t dim, Range const& range) {
    SizeList sizes = m_array.sizes();
    sizes[dim] = range.size();
    m_array.resize(sizes);

    m_ranges[dim] = range;
  }

  template<typename... Rest>
  void setRanges(RangeList const& ranges) {
    SizeList arraySize;

    for (size_t dim = 0; dim < Rank; ++dim) {
      arraySize[dim] = ranges[dim].size();
      m_ranges[dim] = ranges[dim];
    }

    m_array.resize(arraySize);
  }

  template<typename... T>
  void setRanges(Range const& range, T const&... rest) {
    setRanges(RangeList{range, rest...});
  }

  // Set array element based on index.
  void set(IndexList const& index, Element const& element) {
    m_array.set(index, element);
  }

  // Get array element based on index.
  Element const& get(IndexList const& index) const {
    return m_array(index);
  }

  MultiArray const& array() const {
    return m_array;
  }

  MultiArray& array() {
    return m_array;
  }

  void set2TermInterpolation(WeightFunction2 const& weightFunction, BoundMode boundMode) {
    Interpolator2 interpolator2(weightFunction, boundMode);
    m_interpolateFunction = [=](PositionList const& position) {
        return interpolator2.interpolate(this->m_array, this->toIndexSpace(position));
      };

    m_resampleFunction = [=](SizeList const& size) {
        ResampledTable resampledTable;
        resampledTable.reshape(size);

        for (size_t i = 0; i < Rank; ++i)
          resampledTable.setRange(i, *this->m_ranges[i].begin(), *--this->m_ranges[i].end());

        interpolator2.sample(this->m_array, resampledTable.array(), ResampleCoordinateOp(*this, resampledTable));
        return resampledTable;
      };
  }

  void setPiecewiseInterpolation(WeightFunction2 const& weightFunction, BoundMode boundMode) {
    PiecewiseInterpolator piecewiseInterpolator(weightFunction, boundMode);
    m_interpolateFunction = [=](PositionList const& position) {
        return piecewiseInterpolator.interpolate(this->m_array, this->toIndexSpace(position));
      };

    m_resampleFunction = [=](SizeList const& size) {
        ResampledTable resampledTable;
        resampledTable.reshape(size);

        resampledTable.eval([=](typename ResampledTable::PositionList const& position) {
            piecewiseInterpolator.interpolate(this->m_array, this->toIndexSpace(position));
          });

        return resampledTable;
      };
  }

  Element interpolate(PositionList const& coord) const {
    return m_interpolateFunction(coord);
  }

  Element operator()(PositionList const& coord) const {
    return interpolate(coord);
  }

  ResampledTable resample(SizeList const& size) const {
    return m_resampleFunction(size);
  }

  // op should take a PositionList parameter and return an element.
  template<typename OpType>
  void eval(OpType op) {
    m_array.eval(EvalWrapper<OpType>(op, *this));
  }

private:
  struct ResampleCoordinateOp {
    typedef Position Scalar;

    ResampleCoordinateOp(UTable const& u, ResampledTable const& t) :
      utable(u), table(t) {}

    Scalar operator()(size_t dim, size_t pos) {
      return inverseLinearInterpolate(utable.m_ranges[dim], table.toTableSpace(pos, dim));
    }

    PositionList operator()(size_t dim, int i) {
      return inverseLinearInterpolate(utable.getRange(dim),
          table.getTransformedCoord(i, dim));
    }

    UTable const& utable;
    ResampledTable const& table;
  };

  template<typename Coordinate>
  inline PositionList toIndexSpace(Coordinate const& coord) const {
    PositionList indexCoord;
    for (size_t i = 0; i < Rank; ++i)
      indexCoord[i] = inverseLinearInterpolate(m_ranges[i], coord[i]);
    return indexCoord;
  }

  template<typename OpType>
  struct EvalWrapper {
    EvalWrapper(OpType &o, UTable const& t) : op(o), table(t) {}

    template<typename IndexList>
    Element operator()(IndexList const& indexList) {
      PositionList rangeList;
      for (size_t i = 0; i < Rank; ++i)
        rangeList[i] = table.m_ranges[i][indexList[i]];

      return op(rangeList);
    }

    OpType& op;
    UTable const& table;
  };

  typedef std::function<Element(PositionList const&)> InterpolateFunction;
  typedef std::function<ResampledTable(SizeList const&)> ResampleFunction;

  RangeList m_ranges;
  InterpolateFunction m_interpolateFunction;
  ResampleFunction m_resampleFunction;
  MultiArray m_array;
};

typedef UTable<float, float, 2> UTable2F;
typedef UTable<double, double, 2> UTable2D;

typedef UTable<float, float, 3> UTable3F;
typedef UTable<double, double, 3> UTable3D;

typedef UTable<float, float, 4> UTable4F;
typedef UTable<double, double, 4> UTable4D;

}

#endif
