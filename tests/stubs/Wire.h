#pragma once
#include <cstddef>
#include <cstdint>

/* Script a slave's replies and record the transactions made by the master. */
class TwoWire {
 public:
  uint8_t write_status = 0;
  uint8_t receive_count = 1;
  int response = 0;
  unsigned writes = 0;
  unsigned requests = 0;
  uint8_t write_address = 0;
  uint8_t read_address = 0;
  uint8_t command = 0;
  uint8_t requested_length = 0;
  int pending = 0;

  void beginTransmission(uint8_t address) { write_address = address; }
  size_t write(uint8_t value) { command = value; return 1; }
  uint8_t endTransmission() { ++writes; return write_status; }
  uint8_t requestFrom(uint8_t address, uint8_t length) {
    ++requests;
    read_address = address;
    requested_length = length;
    pending = receive_count;
    return receive_count;
  }
  int available() { return pending; }
  int read() {
    if (pending == 0) return -1;
    --pending;
    return response;
  }
};
