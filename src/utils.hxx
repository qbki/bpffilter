#pragma once
#include <iostream>

template<typename T>
void print(const T& value) {
  std::cout << value << std::endl;
}

template<typename T>
void print(const T* value) {
  std::cout << *value << std::endl;
}

template<>
void print(const char* value) {
  std::cout << value << std::endl;
}

template<typename T>
void print_err(const T& value) {
  std::cerr << value << std::endl;
}

template<typename T>
void print_err(const T* value) {
  std::cerr << *value << std::endl;
}

template<>
void print_err(const char* value) {
  std::cerr << value << std::endl;
}
