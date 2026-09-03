# Arduino DDP master

`DevLabDDP` is the shared and self-contained master library for the joystick,
TEMT6000, GT36537 and DS18B20 admin sketches. Its authoritative command registry is
packaged as `DevLabDDP/src/DevLabDDPProtocol.h`; installed copies do not
depend on the parent repository.

Every device-specific operation verifies `CMD_GET_PROTOCOL`,
`CMD_GET_DEVICE_ID` and the capability bitmap first. Address changes and
resets are rejected when the node does not match the admin's expected device
ID.

Compile an admin from the container repository, for example:

```bash
arduino-cli compile \
  --fqbn esp32:esp32:esp32s3 \
  --libraries libraries/arduino \
  unit_firmware_i2c_joystick_py32/admin/ddpAdmin
```

The serial console uses hexadecimal I2C addresses. Available commands are
`scan`, `info`, `read`, `addr`, `setaddr`, `reset` and `help`.
