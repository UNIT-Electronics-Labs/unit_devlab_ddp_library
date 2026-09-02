# DevLabDDP

Biblioteca Arduino autocontenida para maestros del DevLab Device Protocol
(DDP) sobre I2C.

## Instalación

Copie la carpeta `DevLabDDP` completa dentro de `Arduino/libraries` o
comprímala como ZIP e instálela desde Arduino IDE. No es necesario copiar
ningún archivo del directorio padre `protocol`.

```cpp
#include <Wire.h>
#include <DevLabDDP.h>

DevLabDDP::Master device(Wire, DevLabDDP::DEVICE_TEMT6000);
```

Para una consola que administre cualquier dispositivo DDP registrado en el
mismo bus, use `DevLabDDP::DEVICE_ANY`. En este modo, cada operación valida
que el nodo responda a DDP, sin limitarlo a un único Device ID.

`DevLabDDPProtocol.h` contiene el mapa DDP 1.0 incluido en la propia
biblioteca. `DevLabI2CBusRecovery.h` ofrece recuperación opcional del bus
para ESP32 y RP2040/RP2350.

## Dispositivos registrados

| Constante | Device ID |
|---|---:|
| `DEVICE_ANY` | `0x0000` (comodín para maestros universales) |
| `DEVICE_JOYSTICK` | `0x0101` |
| `DEVICE_TEMT6000` | `0x0102` |
| `DEVICE_DS18B20` | `0x0103` |
| `DEVICE_PIR` | `0x0104` |
| `DEVICE_HX0805` | `0x0105` |
| `DEVICE_WS12XX_NEO` | `0x0400` |

Licencia y condiciones de publicación: consulte el repositorio del proyecto.
