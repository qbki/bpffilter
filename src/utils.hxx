#pragma once
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

#include "common.h"

template<typename T>
inline void
print(const T& value, bool has_next_line = true)
{
  std::cout << value;
  if (has_next_line) {
    std::cout << std::endl;
  }
}

template<typename T>
inline void
print(const T* value, bool has_next_line = true)
{
  print(*value, has_next_line);
}

template<>
inline void
print(const char* value, bool has_next_line)
{
  std::cout << value;
  if (has_next_line) {
    std::cout << std::endl;
  }
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
  const uint8_t none = 0;
  return optional.has_value() ? flag : none;
}

inline uint8_t
flag_from_boolean(bool value, uint8_t flag)
{
  const uint8_t none = 0;
  return value ? flag : none;
}

std::vector<std::string>
split(const std::string& value, const std::string& delimiter);

std::string
format_mbitps(unsigned long bytes, unsigned long seconds);

std::string
format_mbyteps(unsigned long bytes, unsigned long seconds);

std::string
format_bytes(unsigned long size);

std::string
format_ipv4_address(uint32_t value);

std::vector<StatData> generate_stats_per_cpu_array();
