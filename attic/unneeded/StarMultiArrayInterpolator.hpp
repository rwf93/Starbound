#ifndef STAR_MULTI_ARRAY_INTERPOLATOR_HPP
#define STAR_MULTI_ARRAY_INTERPOLATOR_HPP

#include "StarMultiArray.hpp"
#include "StarInterpolation.hpp"

namespace Star {

// Internal Grid class, needs to be available to all temlate specializations.
// Holds index values and weights for a sample point in one dimension.
template<typename Weight, size_t Rank, size_t InterpolationWidth>
struct SampleInfo {
  // Structure for holding all SampleInfo points for every dimension.
  typedef Array< std::vector<SampleInfo>, Rank> InfoSet;

  Array<size_t, InterpolationWidth> indexes;
  Array<Weight, InterpolationWidth> weights;
};

// Internal Grid class used for piecewise interpolation.  Needs to be available
// to all template specializations.
template<typename Position, size_t Rank>
struct PiecewiseRange {
  typedef Array<PiecewiseRange, Rank> List;
  size_t dim;
  Position offset;

  bool operator<(PiecewiseRange const& pr) const {
    return pr.offset < offset;
  }
};

// To sample one array into another, there must be a way to translate index
// locations in the destination array to locations in the source array in
// source-array index space.  This class is a simple version of that that
// simply "stretches" the destination array over the source array from min to
// max.
template<typename Position, size_t Rank>
struct SimpleCoordinateOperator {
  typedef Array<size_t, Rank> SizeList;
  typedef Array<Position, Rank> PositionList;

  // Size should be the size of the *destination* array, and min/max should be
  // the index-space locations in the source array that map to the lower and
  // upper corners of the destination array.
  SimpleCoordinateOperator(SizeList const& sz, PositionList const& mn, PositionList const& mx)
    : size(sz), min(mn), max(mx) {}

  // Takes dimension and position in destination array, returns position in
  // source array.
  Position operator()(size_t dim, size_t pos) {
    return min[dim] + (max[dim] - min[dim]) * Position(pos) / Position(size[dim] - 1);
  }

  SizeList const& size;
  PositionList const& min;
  PositionList const& max;
};

template<typename MultiArrayT, typename PositionT>
struct MultiArrayInterpolator2 {
  typedef MultiArrayT MultiArray;
  typedef PositionT Position;

  typedef typename MultiArray::Element Element;
  static size_t const Rank = MultiArray::Rank;

  typedef Array<size_t, Rank> IndexList;
  typedef Array<size_t, Rank> SizeList;
  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 2> WeightList;

  typedef Star::SampleInfo<Position, Rank, 2> SampleInfo;
  typedef typename SampleInfo::InfoSet SampleInfoSet;

  typedef std::function<WeightList(Position)> WeightFunction;
  typedef std::function<Position(size_t, size_t)> CoordinateFunction;

  WeightFunction weightFunction;
  BoundMode boundMode;

  MultiArrayInterpolator2(WeightFunction wf, BoundMode b = BoundMode::Clamp)
    : weightFunction(wf), boundMode(b) {}

  Element interpolate(MultiArray const& array, PositionList const& coord) const {
    IndexList imin;
    IndexList imax;
    PositionList offset;

    for (size_t i = 0; i < Rank; ++i) {
      auto binfo = getBound2(coord[i], array.size(i), boundMode);
      imin[i] = binfo.i0;
      imax[i] = binfo.i1;
      offset[i] = binfo.offset;
    }

    return interpolateSub(array, imin, imax, offset, IndexList(), 0);
  }

  void sample(MultiArray const& source, MultiArray& destination, CoordinateFunction coordFunction) {
    SampleInfoSet sampleInfoSet;

    // Precalculate component indexes and weights for entire size of each
    // dimension.
    for (size_t dim = 0; dim < Rank; ++dim) {
      sampleInfoSet[dim].resize(destination.size(dim));
      for (size_t i = 0; i < destination.size(dim); ++i) {
        auto bound = getBound2(coordFunction(dim, i), source.size(dim), boundMode);

        SampleInfo& sampleInfo = sampleInfoSet[dim][i];
        sampleInfo.indexes[0] = bound.i0;
        sampleInfo.indexes[1] = bound.i1;
        sampleInfo.weights = weightFunction(bound.offset);
      }
    }

    IndexList resIndex;
    sampleSub(source, destination, resIndex, IndexList(), 1, 0, sampleInfoSet);
  }

  void sample(MultiArray const& source, MultiArray& destination, PositionList const& min, PositionList const& max) {
    sample(source, destination, SimpleCoordinateOperator<Position, Rank>(destination.size(), min, max));
  }

  MultiArray resample(MultiArray const& source, PositionList const& min, PositionList const& max, SizeList const& size) {
    MultiArray resampledArray(size);
    sample(source, resampledArray, min, max);
    return resampledArray;
  }

private:
  Element interpolateSub(
      MultiArray const& array,
      IndexList const& imin, IndexList const& imax,
      PositionList const& offset, IndexList const& index,
      size_t const dim) const {
    IndexList minIndex = index;
    IndexList maxIndex = index;

    minIndex[dim] = imin[dim];
    maxIndex[dim] = imax[dim];

    WeightList weights = weightFunction(offset[dim]);

    if (dim == Rank - 1) {
      return weights[0] * array(minIndex) + weights[1] * array(maxIndex);
    } else {
      return
        weights[0] * interpolateSub(array, imin, imax, offset, minIndex, dim+1) +
        weights[1] * interpolateSub(array, imin, imax, offset, maxIndex, dim+1);
    }
  }

  void sampleSub(MultiArray const& source, MultiArray& destination,
      IndexList& resIndex, IndexList const& srcIndex, Position const& weight,
      size_t dim, SampleInfoSet const& sampleInfoSet) {
    // For whatever dimension we are currently on, find the min and max
    // coords for this dimension
    IndexList minIndex = srcIndex;
    IndexList maxIndex = srcIndex;

    for (size_t i = 0; i < destination.size(dim); ++i) {
      resIndex[dim] = i;

      SampleInfo const& sampleInfo = sampleInfoSet[dim][i];

      minIndex[dim] = sampleInfo.indexes[0];
      maxIndex[dim] = sampleInfo.indexes[1];

      if (dim == Rank - 1) {
        destination(resIndex) += weight *
          (sampleInfo.weights[0] * source(minIndex) +
           sampleInfo.weights[1] * source(maxIndex));
      } else {
        sampleSub(source, destination, resIndex, minIndex,
            sampleInfo.weights[0] * weight, dim+1, sampleInfoSet);
        sampleSub(source, destination, resIndex, maxIndex,
            sampleInfo.weights[1] * weight, dim+1, sampleInfoSet);
      }
    }
  }
};

template<typename MultiArrayT, typename PositionT>
struct MultiArrayInterpolator4 {
  typedef MultiArrayT MultiArray;
  typedef PositionT Position;

  typedef typename MultiArray::Element Element;
  static size_t const Rank = MultiArray::Rank;

  typedef Array<size_t, Rank> IndexList;
  typedef Array<size_t, Rank> SizeList;
  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 4> WeightList;

  typedef Star::SampleInfo<Position, Rank, 4> SampleInfo;
  typedef typename SampleInfo::InfoSet SampleInfoSet;

  typedef std::function<WeightList(Position)> WeightFunction;
  typedef std::function<Position(size_t, size_t)> CoordinateFunction;

  WeightFunction weightFunction;
  BoundMode boundMode;

  MultiArrayInterpolator4(WeightFunction wf, BoundMode b = BoundMode::Clamp)
    : weightFunction(wf), boundMode(b) {}

  Element interpolate(MultiArray const& array, PositionList const& coord) const {
    IndexList index0;
    IndexList index1;
    IndexList index2;
    IndexList index3;
    PositionList offset;

    for (size_t i = 0; i < Rank; ++i) {
      auto bound = getBound4(coord[i], array.size(i), boundMode);
      index0[i] = bound.i0;
      index1[i] = bound.i1;
      index2[i] = bound.i2;
      index3[i] = bound.i3;
      offset[i] = bound.offset;
    }

    return interpolateSub(array, index0, index1, index2, index3, offset, IndexList(), 0);
  }

  void sample(MultiArray const& source, MultiArray& destination, CoordinateFunction coordFunction) {
    SampleInfoSet sampleInfoSet;

    // Precalculate component indexes and weights for entire size of each
    // dimension.
    for (size_t dim = 0; dim < Rank; ++dim) {
      sampleInfoSet[dim].resize(destination.size(dim));
      for (size_t i = 0; i < destination.size(dim); ++i) {
        auto bound = getBound4(coordFunction(dim, i), source.size(dim), boundMode);

        SampleInfo& sampleInfo = sampleInfoSet[dim][i];
        sampleInfo.indexes[0] = bound.i0;
        sampleInfo.indexes[1] = bound.i1;
        sampleInfo.indexes[2] = bound.i2;
        sampleInfo.indexes[3] = bound.i3;
        sampleInfo.weights = weightFunction(bound.offset);
      }
    }

    IndexList resIndex;
    sampleSub(source, destination, resIndex, IndexList(), 1, 0, sampleInfoSet);
  }

  void sample(MultiArray const& source, MultiArray& destination, PositionList const& min, PositionList const& max) {
    sample(source, destination, SimpleCoordinateOperator<Position, Rank>(destination.size(), min, max));
  }

  MultiArray resample(MultiArray const& source, PositionList const& min, PositionList const& max, SizeList const& size) {
    MultiArray resampledArray(size);
    sample(source, resampledArray, min, max);
    return resampledArray;
  }

private:
  Element interpolateSub(
      MultiArray const& array,
      IndexList const& i0, IndexList const& i1,
      IndexList const& i2, IndexList const& i3,
      PositionList const& offset, IndexList const& index,
      size_t const dim) const {
    IndexList index0 = index;
    IndexList index1 = index;
    IndexList index2 = index;
    IndexList index3 = index;

    index0[dim] = i0[dim];
    index1[dim] = i1[dim];
    index2[dim] = i2[dim];
    index3[dim] = i3[dim];

    WeightList weights = weightFunction(offset[dim]);

    if (dim == Rank - 1) {
      return
        weights[0] * array(index0) +
        weights[1] * array(index1) +
        weights[2] * array(index2) +
        weights[3] * array(index3);
    } else {
      return
        weights[0] * interpolateSub(array, i0, i1, i2, i3, offset, index0, dim+1) +
        weights[1] * interpolateSub(array, i0, i1, i2, i3, offset, index1, dim+1) +
        weights[2] * interpolateSub(array, i0, i1, i2, i3, offset, index2, dim+1) +
        weights[3] * interpolateSub(array, i0, i1, i2, i3, offset, index3, dim+1);
    }
  }

  void sampleSub(MultiArray const& source, MultiArray& destination, IndexList& resIndex,
          IndexList const& srcIndex, Position const& weight, size_t dim,
          SampleInfoSet const& sampleInfoSet) {
    // For whatever dimension we are currently on, find the min and max
    // coords for this dimension
    IndexList index0 = srcIndex;
    IndexList index1 = srcIndex;
    IndexList index2 = srcIndex;
    IndexList index3 = srcIndex;

    for (size_t i = 0; i < destination.size(dim); ++i) {
      resIndex[dim] = i;

      SampleInfo const& sampleInfo = sampleInfoSet[dim][i];

      index0[dim] = sampleInfo.indexes[0];
      index1[dim] = sampleInfo.indexes[1];
      index2[dim] = sampleInfo.indexes[2];
      index3[dim] = sampleInfo.indexes[3];

      if (dim == Rank - 1) {
        destination(resIndex) += weight *
          (sampleInfo.weights[0] * source(index0) +
           sampleInfo.weights[1] * source(index1) +
           sampleInfo.weights[2] * source(index2) +
           sampleInfo.weights[3] * source(index3));
      } else {
        sampleSub(source, destination, resIndex, index0, sampleInfo.weights[0] * weight, dim+1, sampleInfoSet);
        sampleSub(source, destination, resIndex, index1, sampleInfo.weights[1] * weight, dim+1, sampleInfoSet);
        sampleSub(source, destination, resIndex, index2, sampleInfo.weights[2] * weight, dim+1, sampleInfoSet);
        sampleSub(source, destination, resIndex, index3, sampleInfo.weights[3] * weight, dim+1, sampleInfoSet);
      }
    }
  }
};

template<typename MultiArrayT, typename PositionT>
struct MultiArrayPiecewiseInterpolator {
  typedef MultiArrayT MultiArray;
  typedef PositionT Position;

  typedef typename MultiArray::Element Element;
  static size_t const Rank = MultiArray::Rank;

  typedef Array<size_t, Rank> IndexList;
  typedef Array<size_t, Rank> SizeList;
  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 2> WeightList;

  typedef std::function<WeightList(Position)> WeightFunction;

  WeightFunction weightFunction;
  BoundMode boundMode;

  MultiArrayPiecewiseInterpolator(WeightFunction wf, BoundMode b = BoundMode::Clamp)
    : weightFunction(wf), boundMode(b) {}

  // O(n) for n-dimensions.
  Element interpolate(MultiArray const& array, PositionList const& coord) const {
    typename PiecewiseRange<Position, Rank>::List piecewiseRangeList;

    IndexList minIndex;
    IndexList maxIndex;

    for (size_t i = 0; i < Rank; ++i) {
        PiecewiseRange<Position, Rank> range;
        range.dim = i;

        auto bound = getBound2(coord[i], array.size(i), boundMode);
        minIndex[i] = bound.i0;
        maxIndex[i] = bound.i1;
        range.offset = bound.offset;

        piecewiseRangeList[i] = range;
    }

    std::sort(piecewiseRangeList.begin(), piecewiseRangeList.end());

    IndexList location = minIndex;
    Element result = array(location);
    Element last = result;
    Element current;

    for (size_t i = 0; i < Rank; ++i) {
      PiecewiseRange<Position, Rank> const& pr = piecewiseRangeList[i];
      location[pr.dim] = maxIndex[pr.dim];
      current = array(location);

      WeightList weights = weightFunction(pr.offset);
      result += last * (weights[0] - 1) + current * weights[1];
      last = current;
    }

    return result;
  }
};

// Template specializations for Rank 2

template<typename ElementT, typename PositionT>
struct MultiArrayInterpolator2<MultiArray<ElementT, 2>, PositionT> {
  typedef Star::MultiArray<ElementT, 2> MultiArray;
  typedef PositionT Position;

  typedef typename MultiArray::Element Element;
  static size_t const Rank = 2;

  typedef Array<size_t, Rank> IndexList;
  typedef Array<size_t, Rank> SizeList;
  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 2> WeightList;

  typedef Star::SampleInfo<Position, Rank, 2> SampleInfo;
  typedef typename SampleInfo::InfoSet SampleInfoSet;

  typedef std::function<WeightList(Position)> WeightFunction;
  typedef std::function<Position(size_t, size_t)> CoordinateFunction;

  WeightFunction weightFunction;
  BoundMode boundMode;

  MultiArrayInterpolator2(WeightFunction wf, BoundMode b = BoundMode::Clamp)
    : weightFunction(wf), boundMode(b) {}

  Element interpolate(MultiArray const& array, PositionList const& coord) const {
    IndexList imin;
    IndexList imax;
    PositionList offset;

    for (size_t i = 0; i < Rank; ++i) {
      auto bound = getBound2(coord[i], array.size(i), boundMode);
      imin[i] = bound.i0;
      imax[i] = bound.i1;
      offset[i] = bound.offset;
    }

    WeightList xweights = weightFunction(offset[0]);
    WeightList yweights = weightFunction(offset[1]);

    return
      xweights[0] * (yweights[0] * array(imin[0], imin[1]) + yweights[1] * array(imin[0], imax[1])) +
      xweights[1] * (yweights[0] * array(imax[0], imin[1]) + yweights[1] * array(imax[0], imax[1]));
  }

  void sample(MultiArray const& source, MultiArray& destination, CoordinateFunction coordFunction) {
    SampleInfoSet sampleInfoSet;

    // Precalculate component indexes and weights for entire size of each
    // dimension.
    for (size_t dim = 0; dim < Rank; ++dim) {
      sampleInfoSet[dim].resize(destination.size(dim));
      for (size_t i = 0; i < destination.size(dim); ++i) {
        auto bound = getBound2(coordFunction(dim, i), source.size(dim), boundMode);

        SampleInfo& sampleInfo = sampleInfoSet[dim][i];
        sampleInfo.indexes[0] = bound.i0;
        sampleInfo.indexes[1] = bound.i1;
        sampleInfo.weights = weightFunction(bound.offset);
      }
    }

    for (size_t x = 0; x < destination.size(0); ++x) {
      const SampleInfo& xSampleInfo = sampleInfoSet[0][x];

      for (size_t y = 0; y < destination.size(1); ++y) {
        const SampleInfo& ySampleInfo = sampleInfoSet[1][y];

        destination(x, y) +=
          xSampleInfo.weights[0] * (
              ySampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[0]) +
              ySampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[1])
            ) +
          xSampleInfo.weights[1] * (
              ySampleInfo.weights[0] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[0]) +
              ySampleInfo.weights[1] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[1])
            );
      }
    }
  }

  void sample(MultiArray const& source, MultiArray& destination, PositionList const& min, PositionList const& max) {
    sample(source, destination, SimpleCoordinateOperator<Position, Rank>(destination.size(), min, max));
  }

  MultiArray resample(MultiArray const& source, PositionList const& min, PositionList const& max, SizeList const& size) {
    MultiArray resampledArray(size);
    sample(source, resampledArray, min, max);
    return resampledArray;
  }
};

template<typename ElementT, typename PositionT>
struct MultiArrayInterpolator4<MultiArray<ElementT, 2>, PositionT> {
  typedef Star::MultiArray<ElementT, 2> MultiArray;
  typedef PositionT Position;

  typedef typename MultiArray::Element Element;
  static size_t const Rank = 2;

  typedef Array<size_t, Rank> IndexList;
  typedef Array<size_t, Rank> SizeList;
  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 4> WeightList;

  typedef Star::SampleInfo<Position, Rank, 4> SampleInfo;
  typedef typename SampleInfo::InfoSet SampleInfoSet;

  typedef std::function<WeightList(Position)> WeightFunction;
  typedef std::function<Position(size_t, size_t)> CoordinateFunction;

  WeightFunction weightFunction;
  BoundMode boundMode;

  MultiArrayInterpolator4(WeightFunction wf, BoundMode b = BoundMode::Clamp)
    : weightFunction(wf), boundMode(b) {}

  Element interpolate(MultiArray const& array, PositionList const& coord) const {
    IndexList index0;
    IndexList index1;
    IndexList index2;
    IndexList index3;
    PositionList offset;

    for (size_t i = 0; i < Rank; ++i) {
      auto bound = getBound4(coord[i], array.size(i), boundMode);
      index0[i] = bound.i0;
      index1[i] = bound.i1;
      index2[i] = bound.i2;
      index3[i] = bound.i3;
      offset[i] = bound.offset;
    }

    WeightList xweights = weightFunction(offset[0]);
    WeightList yweights = weightFunction(offset[1]);

    return
      xweights[0] * (
          yweights[0] * array(index0[0], index0[1]) +
          yweights[1] * array(index0[0], index1[1]) +
          yweights[2] * array(index0[0], index2[1]) +
          yweights[3] * array(index0[0], index3[1])
        ) +
      xweights[1] * (
          yweights[0] * array(index1[0], index0[1]) +
          yweights[1] * array(index1[0], index1[1]) +
          yweights[2] * array(index1[0], index2[1]) +
          yweights[3] * array(index1[0], index3[1])
        ) +
      xweights[2] * (
          yweights[0] * array(index2[0], index0[1]) +
          yweights[1] * array(index2[0], index1[1]) +
          yweights[2] * array(index2[0], index2[1]) +
          yweights[3] * array(index2[0], index3[1])
        ) +
      xweights[3] * (
          yweights[0] * array(index3[0], index0[1]) +
          yweights[1] * array(index3[0], index1[1]) +
          yweights[2] * array(index3[0], index2[1]) +
          yweights[3] * array(index3[0], index3[1])
        );
  }

  void sample(MultiArray const& source, MultiArray& destination, CoordinateFunction coordFunction) {
    SampleInfoSet sampleInfoSet;

    // Precalculate component indexes and weights for entire size of each
    // dimension.
    for (size_t dim = 0; dim < Rank; ++dim) {
      sampleInfoSet[dim].resize(destination.size(dim));
      for (size_t i = 0; i < destination.size(dim); ++i) {
        auto bound = getBound4(coordFunction(dim, i), source.size(dim), boundMode);

        SampleInfo& sampleInfo = sampleInfoSet[dim][i];
        sampleInfo.indexes[0] = bound.i0;
        sampleInfo.indexes[1] = bound.i1;
        sampleInfo.indexes[2] = bound.i2;
        sampleInfo.indexes[3] = bound.i3;
        sampleInfo.weights = weightFunction(bound.offset);
      }
    }

    for (size_t x = 0; x < destination.size(0); ++x) {
      SampleInfo& xSampleInfo = sampleInfoSet[0][x];

      for (size_t y = 0; y < destination.size(1); ++y) {
        SampleInfo& ySampleInfo = sampleInfoSet[1][y];

        destination(x, y) +=
          xSampleInfo.weights[0] * (
              ySampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[0]) +
              ySampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[1]) +
              ySampleInfo.weights[2] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[2]) +
              ySampleInfo.weights[3] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[3])
            ) +
          xSampleInfo.weights[1] * (
              ySampleInfo.weights[0] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[0]) +
              ySampleInfo.weights[1] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[1]) +
              ySampleInfo.weights[2] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[2]) +
              ySampleInfo.weights[3] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[3])
            ) +
          xSampleInfo.weights[2] * (
              ySampleInfo.weights[0] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[0]) +
              ySampleInfo.weights[1] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[1]) +
              ySampleInfo.weights[2] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[2]) +
              ySampleInfo.weights[3] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[3])
            ) +
          xSampleInfo.weights[3] * (
              ySampleInfo.weights[0] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[0]) +
              ySampleInfo.weights[1] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[1]) +
              ySampleInfo.weights[2] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[2]) +
              ySampleInfo.weights[3] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[3])
            );
      }
    }
  }

  void sample(MultiArray const& source, MultiArray& destination, PositionList const& min, PositionList const& max) {
    sample(source, destination, SimpleCoordinateOperator<Position, Rank>(destination.size(), min, max));
  }

  MultiArray resample(MultiArray const& source, PositionList const& min, PositionList const& max, SizeList const& size) {
    MultiArray resampledArray(size);
    sample(source, resampledArray, min, max);
    return resampledArray;
  }
};

// Template specializations for Rank 3

template<typename ElementT, typename PositionT>
struct MultiArrayInterpolator2<MultiArray<ElementT, 3>, PositionT> {
  typedef Star::MultiArray<ElementT, 3> MultiArray;
  typedef PositionT Position;

  typedef typename MultiArray::Element Element;
  static size_t const Rank = 3;

  typedef Array<size_t, Rank> IndexList;
  typedef Array<size_t, Rank> SizeList;
  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 2> WeightList;

  typedef Star::SampleInfo<Position, Rank, 2> SampleInfo;
  typedef typename SampleInfo::InfoSet SampleInfoSet;

  typedef std::function<WeightList(Position)> WeightFunction;
  typedef std::function<Position(size_t, size_t)> CoordinateFunction;

  WeightFunction weightFunction;
  BoundMode boundMode;

  MultiArrayInterpolator2(WeightFunction wf, BoundMode b = BoundMode::Clamp)
    : weightFunction(wf), boundMode(b) {}

  Element interpolate(MultiArray const& array, PositionList const& coord) const {
    IndexList imin;
    IndexList imax;
    PositionList offset;

    for (size_t i = 0; i < Rank; ++i) {
      auto bound = getBound2(coord[i], array.size(i), boundMode);
      imin[i] = bound.i0;
      imax[i] = bound.i1;
      offset[i] = bound.offset;
    }

    WeightList xweights = weightFunction(offset[0]);
    WeightList yweights = weightFunction(offset[1]);
    WeightList zweights = weightFunction(offset[2]);

    return
      xweights[0] * (
            yweights[0] * (
              zweights[0] * array(imin[0], imin[1], imin[2]) +
              zweights[1] * array(imin[0], imin[1], imax[2])
            ) +
            yweights[1] * (
              zweights[0] * array(imin[0], imax[1], imin[2]) +
              zweights[1] * array(imin[0], imax[1], imax[2])
            )
          ) +
      xweights[1] * (
            yweights[0] * (
              zweights[0] * array(imax[0], imin[1], imin[2]) +
              zweights[1] * array(imax[0], imin[1], imax[2])
            ) +
            yweights[1] * (
              zweights[0] * array(imax[0], imax[1], imin[2]) +
              zweights[1] * array(imax[0], imax[1], imax[2])
            )
          );
  }

  void sample(MultiArray const& source, MultiArray& destination, CoordinateFunction coordFunction) {
    SampleInfoSet sampleInfoSet;

    // Precalculate component indexes and weights for entire size of each
    // dimension.
    for (size_t dim = 0; dim < Rank; ++dim) {
      sampleInfoSet[dim].resize(destination.size(dim));
      for (size_t i = 0; i < destination.size(dim); ++i) {
        auto bound = getBound2(coordFunction(dim, i), source.size(dim), boundMode);

        SampleInfo& sampleInfo = sampleInfoSet[dim][i];
        sampleInfo.indexes[0] = bound.i0;
        sampleInfo.indexes[1] = bound.i1;
        sampleInfo.weights = weightFunction(bound.offset);
      }
    }

    for (size_t x = 0; x < destination.size(0); ++x) {
      SampleInfo& xSampleInfo = sampleInfoSet[0][x];

      for (size_t y = 0; y < destination.size(1); ++y) {
        SampleInfo& ySampleInfo = sampleInfoSet[1][y];

        for (size_t z = 0; z < destination.size(2); ++z) {
          SampleInfo& zSampleInfo = sampleInfoSet[2][z];

          destination(x, y, z) +=
            xSampleInfo.weights[0] * (
                ySampleInfo.weights[0] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[0], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[0], zSampleInfo.indexes[1])
                ) +
                ySampleInfo.weights[1] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[1], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[1], zSampleInfo.indexes[1])
                )
              ) +
            xSampleInfo.weights[1] * (
                ySampleInfo.weights[0] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[0], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[0], zSampleInfo.indexes[1])
                ) +
                ySampleInfo.weights[1] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[1], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[1], zSampleInfo.indexes[1])
                )
              );
        }
      }
    }
  }

  void sample(MultiArray const& source, MultiArray& destination, PositionList const& min, PositionList const& max) {
    sample(source, destination, SimpleCoordinateOperator<Position, Rank>(destination.size(), min, max));
  }

  MultiArray resample(MultiArray const& source, PositionList const& min, PositionList const& max, SizeList const& size) {
    MultiArray resampledArray(size);
    sample(source, resampledArray, min, max);
    return resampledArray;
  }
};

template<typename ElementT, typename PositionT>
struct MultiArrayInterpolator4<MultiArray<ElementT, 3>, PositionT> {
  typedef Star::MultiArray<ElementT, 3> MultiArray;
  typedef PositionT Position;

  typedef typename MultiArray::Element Element;
  static size_t const Rank = 3;

  typedef Array<size_t, Rank> IndexList;
  typedef Array<size_t, Rank> SizeList;
  typedef Array<Position, Rank> PositionList;
  typedef Array<Position, 4> WeightList;

  typedef Star::SampleInfo<Position, Rank, 4> SampleInfo;
  typedef typename SampleInfo::InfoSet SampleInfoSet;

  typedef std::function<WeightList(Position)> WeightFunction;
  typedef std::function<Position(size_t, size_t)> CoordinateFunction;

  WeightFunction weightFunction;
  BoundMode boundMode;

  MultiArrayInterpolator4(WeightFunction wf, BoundMode b = BoundMode::Clamp)
    : weightFunction(wf), boundMode(b) {}

  Element interpolate(MultiArray const& array, PositionList const& coord) const {
    IndexList index0;
    IndexList index1;
    IndexList index2;
    IndexList index3;
    PositionList offset;

    for (size_t i = 0; i < Rank; ++i) {
      auto bound = getBound4(coord[i], array.size(i), boundMode);
      index0[i] = bound.i0;
      index1[i] = bound.i1;
      index2[i] = bound.i2;
      index3[i] = bound.i3;
      offset[i] = bound.offset;
    }

    WeightList xweights = weightFunction(offset[0]);
    WeightList yweights = weightFunction(offset[1]);
    WeightList zweights = weightFunction(offset[2]);

    return
      xweights[0] * (
            yweights[0] * (
              zweights[0] * array(index0[0], index0[1], index0[2]) +
              zweights[1] * array(index0[0], index0[1], index1[2]) +
              zweights[2] * array(index0[0], index0[1], index2[2]) +
              zweights[3] * array(index0[0], index0[1], index3[2])
            ) +
            yweights[1] * (
              zweights[0] * array(index0[0], index1[1], index0[2]) +
              zweights[1] * array(index0[0], index1[1], index1[2]) +
              zweights[2] * array(index0[0], index1[1], index2[2]) +
              zweights[3] * array(index0[0], index1[1], index3[2])
            ) +
            yweights[2] * (
              zweights[0] * array(index0[0], index2[1], index0[2]) +
              zweights[1] * array(index0[0], index2[1], index1[2]) +
              zweights[2] * array(index0[0], index2[1], index2[2]) +
              zweights[3] * array(index0[0], index2[1], index3[2])
            ) +
            yweights[3] * (
              zweights[0] * array(index0[0], index3[1], index0[2]) +
              zweights[1] * array(index0[0], index3[1], index1[2]) +
              zweights[2] * array(index0[0], index3[1], index2[2]) +
              zweights[3] * array(index0[0], index3[1], index3[2])
            )
          ) +
      xweights[1] * (
            yweights[0] * (
              zweights[0] * array(index1[0], index0[1], index0[2]) +
              zweights[1] * array(index1[0], index0[1], index1[2]) +
              zweights[2] * array(index1[0], index0[1], index2[2]) +
              zweights[3] * array(index1[0], index0[1], index3[2])
            ) +
            yweights[1] * (
              zweights[0] * array(index1[0], index1[1], index0[2]) +
              zweights[1] * array(index1[0], index1[1], index1[2]) +
              zweights[2] * array(index1[0], index1[1], index2[2]) +
              zweights[3] * array(index1[0], index1[1], index3[2])
            ) +
            yweights[2] * (
              zweights[0] * array(index1[0], index2[1], index0[2]) +
              zweights[1] * array(index1[0], index2[1], index1[2]) +
              zweights[2] * array(index1[0], index2[1], index2[2]) +
              zweights[3] * array(index1[0], index2[1], index3[2])
            ) +
            yweights[3] * (
              zweights[0] * array(index1[0], index3[1], index0[2]) +
              zweights[1] * array(index1[0], index3[1], index1[2]) +
              zweights[2] * array(index1[0], index3[1], index2[2]) +
              zweights[3] * array(index1[0], index3[1], index3[2])
            )
          ) +
      xweights[2] * (
            yweights[0] * (
              zweights[0] * array(index2[0], index0[1], index0[2]) +
              zweights[1] * array(index2[0], index0[1], index1[2]) +
              zweights[2] * array(index2[0], index0[1], index2[2]) +
              zweights[3] * array(index2[0], index0[1], index3[2])
            ) +
            yweights[1] * (
              zweights[0] * array(index2[0], index1[1], index0[2]) +
              zweights[1] * array(index2[0], index1[1], index1[2]) +
              zweights[2] * array(index2[0], index1[1], index2[2]) +
              zweights[3] * array(index2[0], index1[1], index3[2])
            ) +
            yweights[2] * (
              zweights[0] * array(index2[0], index2[1], index0[2]) +
              zweights[1] * array(index2[0], index2[1], index1[2]) +
              zweights[2] * array(index2[0], index2[1], index2[2]) +
              zweights[3] * array(index2[0], index2[1], index3[2])
            ) +
            yweights[3] * (
              zweights[0] * array(index2[0], index3[1], index0[2]) +
              zweights[1] * array(index2[0], index3[1], index1[2]) +
              zweights[2] * array(index2[0], index3[1], index2[2]) +
              zweights[3] * array(index2[0], index3[1], index3[2])
            )
          ) +
      xweights[3] * (
            yweights[0] * (
              zweights[0] * array(index3[0], index0[1], index0[2]) +
              zweights[1] * array(index3[0], index0[1], index1[2]) +
              zweights[2] * array(index3[0], index0[1], index2[2]) +
              zweights[3] * array(index3[0], index0[1], index3[2])
            ) +
            yweights[1] * (
              zweights[0] * array(index3[0], index1[1], index0[2]) +
              zweights[1] * array(index3[0], index1[1], index1[2]) +
              zweights[2] * array(index3[0], index1[1], index2[2]) +
              zweights[3] * array(index3[0], index1[1], index3[2])
            ) +
            yweights[2] * (
              zweights[0] * array(index3[0], index2[1], index0[2]) +
              zweights[1] * array(index3[0], index2[1], index1[2]) +
              zweights[2] * array(index3[0], index2[1], index2[2]) +
              zweights[3] * array(index3[0], index2[1], index3[2])
            ) +
            yweights[3] * (
              zweights[0] * array(index3[0], index3[1], index0[2]) +
              zweights[1] * array(index3[0], index3[1], index1[2]) +
              zweights[2] * array(index3[0], index3[1], index2[2]) +
              zweights[3] * array(index3[0], index3[1], index3[2])
            )
          );
  }

  void sample(MultiArray const& source, MultiArray& destination, CoordinateFunction coordFunction) {
    SampleInfoSet sampleInfoSet;

    // Precalculate component indexes and weights for entire size of each
    // dimension.
    for (size_t dim = 0; dim < Rank; ++dim) {
      sampleInfoSet[dim].resize(destination.size(dim));
      for (size_t i = 0; i < destination.size(dim); ++i) {
        auto bound = getBound4(coordFunction(dim, i), source.size(dim), boundMode);

        SampleInfo& sampleInfo = sampleInfoSet[dim][i];
        sampleInfo.indexes[0] = bound.i0;
        sampleInfo.indexes[1] = bound.i1;
        sampleInfo.indexes[2] = bound.i2;
        sampleInfo.indexes[3] = bound.i3;
        sampleInfo.weights = weightFunction(bound.offset);
      }
    }

    for (size_t x = 0; x < destination.size(0); ++x) {
      SampleInfo& xSampleInfo = sampleInfoSet[0][x];

      for (size_t y = 0; y < destination.size(1); ++y) {
        SampleInfo& ySampleInfo = sampleInfoSet[1][y];

        for (size_t z = 0; z < destination.size(2); ++z) {
          SampleInfo& zSampleInfo = sampleInfoSet[2][z];

          destination(x, y, z) +=
            xSampleInfo.weights[0] * (
                ySampleInfo.weights[0] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[0], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[0], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[0], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[0], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[1] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[1], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[1], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[1], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[1], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[2] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[2], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[2], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[2], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[2], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[3] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[3], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[3], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[3], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[0], ySampleInfo.indexes[3], zSampleInfo.indexes[3])
                )
              ) +
            xSampleInfo.weights[1] * (
                ySampleInfo.weights[0] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[0], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[0], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[0], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[0], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[1] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[1], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[1], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[1], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[1], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[2] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[2], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[2], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[2], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[2], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[3] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[3], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[3], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[3], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[1], ySampleInfo.indexes[3], zSampleInfo.indexes[3])
                )
              ) +
            xSampleInfo.weights[2] * (
                ySampleInfo.weights[0] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[0], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[0], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[0], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[0], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[1] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[1], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[1], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[1], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[1], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[2] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[2], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[2], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[2], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[2], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[3] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[3], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[3], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[3], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[2], ySampleInfo.indexes[3], zSampleInfo.indexes[3])
                )
              ) +
            xSampleInfo.weights[3] * (
                ySampleInfo.weights[0] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[0], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[0], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[0], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[0], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[1] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[1], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[1], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[1], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[1], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[2] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[2], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[2], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[2], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[2], zSampleInfo.indexes[3])
                ) +
                ySampleInfo.weights[3] * (
                  zSampleInfo.weights[0] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[3], zSampleInfo.indexes[0]) +
                  zSampleInfo.weights[1] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[3], zSampleInfo.indexes[1]) +
                  zSampleInfo.weights[2] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[3], zSampleInfo.indexes[2]) +
                  zSampleInfo.weights[3] * source(xSampleInfo.indexes[3], ySampleInfo.indexes[3], zSampleInfo.indexes[3])
                )
              );
        }
      }
    }
  }

  void sample(MultiArray const& source, MultiArray& destination, PositionList const& min, PositionList const& max) {
    sample(source, destination, SimpleCoordinateOperator<Position, Rank>(destination.size(), min, max));
  }

  MultiArray resample(MultiArray const& source, PositionList const& min, PositionList const& max, SizeList const& size) {
    MultiArray resampledArray(size);
    sample(source, resampledArray, min, max);
    return resampledArray;
  }
};

}

#endif
