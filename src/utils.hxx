#pragma once
#include <iostream>

template<typename T>
inline void print(const T& value) {
  std::cout << value << std::endl;
}

template<typename T>
inline void print(const T* value) {
  std::cout << *value << std::endl;
}

template<>
inline void print(const char* value) {
  std::cout << value << std::endl;
}

template<typename T>
inline void print_err(const T& value) {
  std::cerr << value << std::endl;
}

template<typename T>
inline void print_err(const T* value) {
  std::cerr << *value << std::endl;
}

template<>
inline void print_err(const char* value) {
  std::cerr << value << std::endl;
}
