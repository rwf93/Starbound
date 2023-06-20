#ifndef STAR_FFT_HPP
#define STAR_FFT_HPP

#include "StarMathCommon.hpp"

#include <complex>
#include <vector>

namespace Star {

template<typename Float>
class FFT {
public:
  typedef std::complex<Float> Complex;
  typedef std::vector<Complex> Vector;

  static Float intensity(Complex c);
  static Float phase(Complex c);

  FFT(int n);
  FFT();

  // Initializes FFT, n will be rounded up to the nearest power of 2.
  void init(int n);

  int size() const;

  Float freq(int i, Float sampleRate = 1.0f) const;

  // Computes DFT, buffer will be resized to current size.
  void transform(Vector& data) const;

  // Computes inverse DFT, buffer will be resized to current size.
  void itransform(Vector& data) const;

  // Make vector complex conjugate symmetric so that the output of a (forward)
  // transform will be real.
  void makeSymmetric(Vector& data) const;

private:
  int m_n, m_lgN;
  Vector m_omega;
  Vector m_iomega;

  void bitReverse(Vector& dest) const;
};

template<typename Float>
Float FFT<Float>::intensity(Complex c) {
  return std::abs(c);
}

template<typename Float>
Float FFT<Float>::phase(Complex c) {
  return std::arg(c);
}

template<typename Float>
FFT<Float>::FFT() : m_n(0), m_lgN(0) {}

template<typename Float>
FFT<Float>::FFT(int n) {
  init(n);
}

template<typename Float>
void FFT<Float>::init(int n) {
  if (m_n == n)
    return;

  m_n = ceilPowerOf2(n);
  m_lgN = 0;

  for (int i = m_n; i > 1; i /= 2)
    ++m_lgN;

  m_omega.resize(m_lgN);
  int m = 1;
  for (int s = 0; s < m_lgN; ++s) {
    m *= 2;
    m_omega[s] = std::exp(Complex(0, -2.0 * Constants::pi / m));
  }

  m_iomega.resize(m_lgN);
  m = 1;
  for (int s = 0; s < m_lgN; ++s) {
    m *= 2;
    m_iomega[s] = std::exp(Complex(0, 2.0 * Constants::pi / m));
  }
}

template<typename Float>
int FFT<Float>::size() const {
  return m_n;
}

template<typename Float>
Float FFT<Float>::freq(int i, Float sampleRate) const {
  return sampleRate * i / m_n;
}

template<typename Float>
void FFT<Float>::transform(Vector& data) const {
  data.resize(m_n);
  bitReverse(data);
  int m = 1;
  for (int s = 0; s < m_lgN; ++s) {
    m *= 2;
    for (int k = 0; k < m_n; k += m) {
      Complex current_omega = 1;
      for (int j = 0; j < (m / 2); ++j) {
        Complex t = current_omega * data[k + j + (m / 2)];
        Complex u = data[k + j];
        data[k + j] = u + t;
        data[k + j + (m / 2)] = u - t;
        current_omega *= m_omega[s];
      }
    }
  }
  for (int i = 0; i < m_n; ++i)
    data[i] /= m_n;
}

template<typename Float>
void FFT<Float>::itransform(Vector& data) const {
  data.resize(m_n);
  bitReverse(data);
  int m = 1;
  for (int s = 0; s < m_lgN; ++s) {
    m *= 2;
    for (int k = 0; k < m_n; k += m) {
      Complex current_omega = 1;
      for (int j = 0; j < (m / 2); ++j) {
        Complex t = current_omega * data[k + j + (m / 2)];
        Complex u = data[k + j];
        data[k + j] = u + t;
        data[k + j + (m / 2)] = u - t;
        current_omega *= m_iomega[s];
      }
    }
  }
}

template<typename Float>
void FFT<Float>::makeSymmetric(Vector& data) const {
  data.resize(m_n);
  for (int i = 0; i <= m_n / 2; ++i) {
    if (i == 0 || i == m_n / 2)
      data[i] = data[i].real();
    else
      data[m_n - i] = std::conj(data[i]);
  }
}

template<typename Float>
void FFT<Float>::bitReverse(Vector& data) const {
  for (int i = 0; i < m_n; ++i) {
    int index = i, rev = 0;
    for (int j = 0; j < m_lgN; ++j) {
      rev = (rev << 1) | (index & 1);
      index >>= 1;
    }
    if (i < rev)
      std::swap(data[i], data[rev]);
  }
}

typedef FFT<float> FFTF;
typedef FFT<double> FFTD;

}

#endif
