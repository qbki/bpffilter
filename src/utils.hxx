#pragma once
#include <cstdint>
#include <iostream>
#include <optional>

template<typename T>
inline void
print(const T& value)
{
  std::cout << value << std::endl;
}

template<typename T>
inline void
print(const T* value)
{
  std::cout << *value << std::endl;
}

template<>
inline void
print(const char* value)
{
  std::cout << value << std::endl;
}

template<typename T>
inline void
print_err(const T& value)
{
  std::cerr << value << std::endl;
}

template<typename T>
inline void
print_err(const T* value)
{
  std::cerr << *value << std::endl;
}

template<>
inline void
print_err(const char* value)
{
  std::cerr << value << std::endl;
}

template<typename T>
inline uint8_t
flag_from_optional(std::optional<T> optional, uint8_t flag)
{
  const unsigned char none = 0;
  return optional.has_value() ? flag : none;
}
