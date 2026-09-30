#pragma once
#include <cstddef>
#include <cstdint>
#define HEX 16
extern unsigned long test_delay_ms;
inline void delay(unsigned long ms) { test_delay_ms = ms; }
class Print {
 public:
  template <typename T> void print(T, int = 10) {}
  template <typename T> void println(T) {}
};
