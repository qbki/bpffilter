#pragma once
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

#include "common.h"

/**
 * Prints "value" to the standard output.
 * @param value A std::ostream compatible type.
 * @param has_next_line Adds a new line when it equals true.
 */
template<typename T>
inline void
print(const T& value, bool has_next_line = true)
{
  std::cout << value;
  if (has_next_line) {
    std::cout << '\n';
  }
}

/**
 * Prints dereferenced "value" to the standard output.
 * @param value A std::ostream compatible type.
 * @param has_next_line Adds a new line when it equals true.
 */
template<typename T>
inline void
print(const T* value, bool has_next_line = true)
{
  print(*value, has_next_line);
}

/**
 * Prints a C string to the standard output.
 * @param value A std::ostream compatible type.
 * @param has_next_line Adds a new line when it equals true.
 */
template<>
inline void
print(const char* value, bool has_next_line)
{
  std::cout << value;
  if (has_next_line) {
    std::cout << '\n';
  }
}

/**
 * Prints "value" to the standard error.
 * @param value A std::ostream compatible type.
 */
template<typename T>
inline void
print_err(const T& value)
{
  std::cerr << value << std::endl;
}

/**
 * Prints dereferenced "value" to the standard error.
 * @param value A std::ostream compatible type.
 */
template<typename T>
inline void
print_err(const T* value)
{
  std::cerr << *value << std::endl;
}

/**
 * Prints a C string to the standard error.
 * @param value A std::ostream compatible type.
 */
template<>
inline void
print_err(const char* value)
{
  std::cerr << value << std::endl;
}

/**
 * Flushes data to standard output.
 */
void
flush();

/**
 * A helper for creating a bit array item from an std::optional.
 *
 * uint8t flags = flag_from_optional(optional_a, 0b01)
 *              | flag_from_optional(optional_b, 0b10);
 *
 * @param optional A value for analisys.
 * @param flag A flag what is used when "optional" has value.
 * @return Returns "flag" if "optional" has value, or 0 if "optional" is empty.
 */
template<typename T>
inline uint8_t
flag_from_optional(std::optional<T> optional, uint8_t flag)
{
  const uint8_t none = 0;
  return optional.has_value() ? flag : none;
}

/**
 * A helper for creating a bit array item from boolean.
 *
 * uint8t flags = flag_from_boolean(bool_value_a, 0b01)
 *              | flag_from_boolean(bool_value_b, 0b10);
 *
 * @param value A boolean value for analisys.
 * @param flag A flag what is used when "value" equals true.
 * @return Returns "flag" if "value" equals true, or 0 if "value" equals false.
 */
inline uint8_t
flag_from_boolean(bool value, uint8_t flag)
{
  const uint8_t none = 0;
  return value ? flag : none;
}

/**
 * Splits "value" using "delimiter".
 * @param value A string for splitting.
 * @param delimiter A delimiter, it is not included in output.
 * @return An array of strings based on amount of delimiters
 *         in the "value" string plus one.
 */
std::vector<std::string>
split(const std::string& value, const std::string& delimiter);

/**
 * Converts bytes per seconds to ISO 80000 formatted string
 * where 1Kibit/s equals 1024 bit/s.
 * @param bytes Raw bytes as an integer value.
 * @param seconds Elapsed seconds.
 * @return Formatted string.
 */
std::string
format_mbitps(unsigned long bytes, unsigned long seconds);

/**
 * Converts bytes per seconds to ISO 80000 formatted string
 * where 1KiB/s equals 1024 bytes/s.
 * @param bytes Raw bytes as an integer value.
 * @param seconds Elapsed seconds.
 * @return Formatted string.
 */
std::string
format_mbyteps(unsigned long bytes, unsigned long seconds);

/**
 * Converts bytes to ISO 80000 formatted string where 1KiB equals 1024 bytes.
 * @param size Raw bytes as an integer value.
 * @return Formatted string.
 */
std::string
format_bytes(unsigned long size);

/**
 * Converts an integer IPv4 value to a dot separated string ("127.0.0.1").
 * Value should be in a little-endian format.
 * @param value An integer IPv4 value.
 * @return A dot separated string.
 */
std::string
format_ipv4_address(uint32_t value);

/**
 * Creates an array of StatData items or throws an error.
 * @return An array of StatData items based on
 *         the quantity of CPUs in a system.
 */
std::vector<StatData> generate_stats_per_cpu_array();
