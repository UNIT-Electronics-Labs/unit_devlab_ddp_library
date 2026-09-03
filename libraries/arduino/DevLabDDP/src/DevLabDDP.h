#ifndef DEVLAB_DDP_MASTER_H
#define DEVLAB_DDP_MASTER_H

#include <Arduino.h>
#include <Wire.h>
#include "DevLabDDPProtocol.h"

namespace DevLabDDP {

constexpr uint16_t DEVICE_ANY      = 0x0000U;
constexpr uint16_t DEVICE_JOYSTICK = 0x0101U;
constexpr uint16_t DEVICE_TEMT6000 = 0x0102U;
constexpr uint16_t DEVICE_DS18B20  = 0x0103U;
constexpr uint16_t DEVICE_PIR      = 0x0104U;
constexpr uint16_t DEVICE_HX0805   = 0x0105U;
constexpr uint16_t DEVICE_GT36537  = 0x0106U;
constexpr uint16_t DEVICE_WS12XX_NEO = 0x0400U;

struct DeviceInfo {
  uint16_t deviceId = 0;
  uint8_t firmwareMajor = 0;
  uint8_t firmwareMinor = 0;
  uint8_t hardwareMajor = 0;
  uint8_t hardwareMinor = 0;
  uint8_t protocolMajor = 0;
  uint8_t protocolMinor = 0;
  uint32_t capabilities = 0;
  bool valid = false;
};

class Master {
 public:
  explicit Master(TwoWire &wire, uint16_t expectedDeviceId)
      : wire_(wire), expectedDeviceId_(expectedDeviceId) {}

  uint16_t expectedDeviceId() const { return expectedDeviceId_; }

  bool ping(uint8_t address) {
    wire_.beginTransmission(address);
    return wire_.endTransmission() == 0;
  }

  bool writeByte(uint8_t address, uint8_t value) {
    wire_.beginTransmission(address);
    wire_.write(value);
    return wire_.endTransmission() == 0;
  }

  bool readCommand(uint8_t address, uint8_t command, uint8_t *data,
                   uint8_t length, uint16_t processingDelayMs = 5U) {
    if (!writeByte(address, command)) return false;
    delay(processingDelayMs);
    uint8_t received = wire_.requestFrom(address, length);
    if (received != length) {
      while (wire_.available()) wire_.read();
      return false;
    }
    for (uint8_t i = 0; i < length; ++i) data[i] = wire_.read();
    return true;
  }

  bool identify(uint8_t address, DeviceInfo &info) {
    info = DeviceInfo{};
    uint8_t bytes[4];
    if (!readCommand(address, CMD_GET_PROTOCOL, bytes, 2U)) return false;
    info.protocolMajor = bytes[0];
    info.protocolMinor = bytes[1];
    if (info.protocolMajor != DEVLAB_PROTOCOL_MAJOR) return false;

    if (!readCommand(address, CMD_GET_DEVICE_ID, bytes, 2U)) return false;
    info.deviceId = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    if (!readCommand(address, CMD_GET_FIRMWARE_VERSION, bytes, 2U)) return false;
    info.firmwareMajor = bytes[0];
    info.firmwareMinor = bytes[1];
    if (!readCommand(address, CMD_GET_HARDWARE_VERSION, bytes, 2U)) return false;
    info.hardwareMajor = bytes[0];
    info.hardwareMinor = bytes[1];
    if (!readCommand(address, CMD_GET_CAPABILITIES, bytes, 4U)) return false;
    info.capabilities = (uint32_t)bytes[0] |
                        ((uint32_t)bytes[1] << 8) |
                        ((uint32_t)bytes[2] << 16) |
                        ((uint32_t)bytes[3] << 24);
    info.valid = true;
    return true;
  }

  bool matchesExpectedDevice(uint8_t address, DeviceInfo *result = nullptr) {
    DeviceInfo local;
    if (!identify(address, local) ||
        (expectedDeviceId_ != DEVICE_ANY && local.deviceId != expectedDeviceId_)) return false;
    if (result != nullptr) *result = local;
    return true;
  }

  bool getI2cAddress(uint8_t address, uint8_t &reportedAddress) {
    if (!matchesExpectedDevice(address)) return false;
    return readCommand(address, CMD_GET_I2C_ADDR, &reportedAddress, 1U);
  }

  bool setI2cAddress(uint8_t oldAddress, uint8_t newAddress) {
    if (newAddress < 0x08U || newAddress > 0x77U ||
        !matchesExpectedDevice(oldAddress) || ping(newAddress)) return false;
    uint8_t response = 0;
    if (!readCommand(oldAddress, CMD_SET_I2C_ADDR, &response, 1U, 10U) ||
        (response & 0x0FU) != RESP_I2C_ADDR_SET) return false;
    if (!writeByte(oldAddress, newAddress)) return false;
    delay(100U);
    uint8_t received = wire_.requestFrom(oldAddress, (uint8_t)1U);
    if (received != 1U || (wire_.read() & 0x0FU) != RESP_I2C_ADDR_SET) return false;
    delay(300U);
    return matchesExpectedDevice(newAddress);
  }

  bool reset(uint8_t address) {
    return matchesExpectedDevice(address) && writeByte(address, CMD_RESET);
  }

  bool readAdc(uint8_t address, uint8_t channel, uint16_t &value) {
    uint8_t bytes[2];
    uint8_t command = channel == 0U ? CMD_READ_ADC0 : CMD_READ_ADC1;
    if (channel > 1U || !matchesExpectedDevice(address) ||
        !readCommand(address, command, bytes, 2U, 30U)) return false;
    value = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    return true;
  }

  bool readGpio0(uint8_t address, bool &value) {
    uint8_t raw;
    if (!matchesExpectedDevice(address) ||
        !readCommand(address, CMD_READ_GPIO0, &raw, 1U)) return false;
    value = raw != 0U;
    return true;
  }

 private:
  TwoWire &wire_;
  uint16_t expectedDeviceId_;
};

inline const char *deviceName(uint16_t deviceId) {
  switch (deviceId) {
    case DEVICE_JOYSTICK: return "joystick";
    case DEVICE_TEMT6000: return "temt6000";
    case DEVICE_DS18B20:  return "ds18b20";
    case DEVICE_PIR:      return "pir";
    case DEVICE_HX0805:   return "hx0805-vibration";
    case DEVICE_GT36537:  return "gt36537-light";
    case DEVICE_WS12XX_NEO: return "ws12xx-neopixel";
    default:              return "unknown";
  }
}

inline void printDeviceInfo(Print &out, uint8_t address, const DeviceInfo &info,
                            uint16_t expectedDeviceId) {
  out.print("addr=0x"); if (address < 0x10U) out.print('0'); out.print(address, HEX);
  out.print(" id=0x"); if (info.deviceId < 0x1000U) out.print('0');
  if (info.deviceId < 0x0100U) out.print('0');
  if (info.deviceId < 0x0010U) out.print('0');
  out.print(info.deviceId, HEX);
  out.print(" type="); out.print(deviceName(info.deviceId));
  out.print(" protocol="); out.print(info.protocolMajor); out.print('.'); out.print(info.protocolMinor);
  out.print(" firmware="); out.print(info.firmwareMajor); out.print('.'); out.print(info.firmwareMinor);
  out.print(" hardware="); out.print(info.hardwareMajor); out.print('.'); out.print(info.hardwareMinor);
  out.print(" caps=0x"); out.print(info.capabilities, HEX);
  out.print(" match=");
  out.println(expectedDeviceId == DEVICE_ANY || info.deviceId == expectedDeviceId ? "yes" : "NO");
}

}  // namespace DevLabDDP

#endif
