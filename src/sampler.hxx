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
  Sampler()
    : _last_sample(0)
    , _distance(0)
    , _min(std::numeric_limits<T>::max())
    , _max(0)
  {
  }

  void sample(T value)
  {
    _distance = value - _last_sample;
    _min = std::min(_min, _distance);
    _max = std::max(_max, _distance);
    _last_sample = value;
  }

  T distance() { return _distance; }

  T min() { return _min; }

  T max() { return _max; }
};
