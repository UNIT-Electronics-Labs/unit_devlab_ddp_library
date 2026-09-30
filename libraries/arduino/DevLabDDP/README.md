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
| `DEVICE_GT36537` | `0x0106` |
| `DEVICE_DPAD` | `0x0107` |
| `DEVICE_WS12XX_NEO` | `0x0400` |

## Lectura del D-Pad por polling

Desde la versión 1.1.0, `Master::readButtons(address, processingDelayMs = 2)`
devuelve directamente el estado actual de S1–S4 como un `int16_t`:

| Resultado o máscara | Significado |
|---|---|
| `-1` | Error I2C o respuesta inválida. |
| `DPAD_NONE` (`0`) | Ningún botón presionado. |
| `DPAD_S1` (`1`) | S1 presionado. |
| `DPAD_S2` (`2`) | S2 presionado. |
| `DPAD_S3` (`4`) | S3 presionado. |
| `DPAD_S4` (`8`) | S4 presionado. |

Las combinaciones se forman con OR: `DPAD_S1 | DPAD_S3` vale `5`. Guarde el
resultado en `int` o `int16_t` y compruebe el error antes de examinar los bits.
Un error no se convierte en cero ni conserva un estado anterior.

Configure `Wire` con los pines de su placa y valide el módulo con
`dpad.matchesExpectedDevice(0x20)` en `setup()`. Después puede consultar:

```cpp
DevLabDDP::Master dpad(Wire, DevLabDDP::DEVICE_DPAD);

void loop() {
  int buttons = dpad.readButtons(0x20);
  if (buttons < 0) {
    // Handle the I2C error here.
    return;
  }

  if (buttons & DevLabDDP::DPAD_S1) {
    // Your action while S1 is pressed, including combinations with S1.
  }

  switch (buttons) {
    case DevLabDDP::DPAD_NONE:
      // No buttons pressed.
      break;
    case DevLabDDP::DPAD_S1 | DevLabDDP::DPAD_S2:
      // Your action for exactly S1 + S2.
      break;
    default:
      // Other individual buttons or combinations.
      break;
  }
  delay(25);  // Polling interval chosen by the application.
}
```

La función no imprime ni genera eventos. Devuelve la misma máscara en cada
consulta mientras se mantengan esos botones, incluso si estaban presionados
en la primera lectura. Es una lectura síncrona: envía el comando `0x80`, espera
2 ms por defecto y lee un byte. No repite la identificación en cada consulta;
la aplicación decide la frecuencia de polling y cómo responder a los errores.
El parámetro opcional permite cambiar la espera de respuesta, incluso a cero
para las pruebas de clock stretching del firmware.

Los ejemplos completos, con selección de pines y polling mediante `millis()`,
están en el repositorio D-Pad bajo `examples/buttons/individualControls` y
`examples/buttons/combinedControls`. Para disponer de esta API en Arduino IDE,
instale la carpeta `DevLabDDP` actualizada siguiendo las instrucciones de arriba.

Licencia y condiciones de publicación: consulte el repositorio del proyecto.
