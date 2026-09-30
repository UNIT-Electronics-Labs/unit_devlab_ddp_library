#include <cassert>
#include <cstdio>
#include <initializer_list>
#include <DevLabDDP.h>

unsigned long test_delay_ms = 0;

int main() {
  TwoWire bus;
  DevLabDDP::Master dpad(bus, DevLabDDP::DEVICE_DPAD);

  // Each of the sixteen physical states must survive one poll unchanged.
  for (int state = 0; state < 16; ++state) {
    bus.response = state;
    unsigned before = bus.requests;
    assert(dpad.readButtons(0x20) == state);
    assert(bus.requests == before + 1);  // No repeated identification.
    assert(bus.writes == bus.requests);
    assert(bus.write_address == 0x20 && bus.read_address == 0x20);
    assert(bus.command == 0x80 && bus.requested_length == 1);
    assert(test_delay_ms == 2);
  }

  // Held, changed and released states are immediately available, with no
  // edge suppression, cached old values, or initial-neutral requirement.
  for (int state : {5, 5, 4, 0, 8}) {
    bus.response = state;
    assert(dpad.readButtons(0x20) == state);
  }
  bus.response = 5;  // Physical S1 + S3 frame.
  int buttons = dpad.readButtons(0x20);
  assert(buttons == (DevLabDDP::DPAD_S1 | DevLabDDP::DPAD_S3));
  assert((buttons & DevLabDDP::DPAD_S2) == 0);
  assert((buttons & DevLabDDP::DPAD_S4) == 0);

  // Missing or malformed data must never look like idle or four keys held.
  for (int response : {0x10, 0x81, 0xFF}) {
    bus.response = response;
    assert(dpad.readButtons(0x20) == -1);
  }
  bus.receive_count = 0;
  assert(dpad.readButtons(0x20) == -1);
  bus.receive_count = 1;
  bus.response = -1;
  assert(dpad.readButtons(0x20) == -1);

  unsigned before = bus.requests;
  bus.write_status = 2;  // Address NACK: do not start a read after it.
  assert(dpad.readButtons(0x20) == -1);
  assert(bus.requests == before);

  // The next successful call recovers, including idle at a custom address.
  bus.write_status = 0;
  bus.response = 0;
  assert(dpad.readButtons(0x37, 0) == DevLabDDP::DPAD_NONE);
  assert(bus.write_address == 0x37 && bus.read_address == 0x37);
  assert(test_delay_ms == 0);
  bus.response = 15;
  assert(dpad.readButtons(0x37) == 15);

  std::puts("PASS: button polling states, combinations, errors and recovery");
}
