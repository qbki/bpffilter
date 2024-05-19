#pragma once
#include <algorithm>
#include <limits>

template<typename T>
class Sampler
{
private:
  T _last_sample;
  T _distance;
  T _min;
  T _max;

public:
  /**
   * Creates a sampler with default values.
   */
  Sampler()
    : _last_sample(0)
    , _distance(0)
    , _min(std::numeric_limits<T>::max())
    , _max(0)
  {
  }

  /**
   * This function:
   * * calculates distance (magnitude) between new "value" and
   *   previously saved "value".
   * * updates "min" and "max" values.
   * * saves currently passed "value".
   * @param value A new sample
   */
  void sample(T value)
  {
    _distance = value - _last_sample;
    _min = std::min(_min, _distance);
    _max = std::max(_max, _distance);
    _last_sample = value;
  }

  /**
   * @return Distance (magnitude) between two last samples.
   */
  T distance() { return _distance; }

  /**
   * @return A minimum value out of all samples.
   */
  T min() { return _min; }

  /**
   * @return A maximum value out of all samples.
   */
  T max() { return _max; }
};
