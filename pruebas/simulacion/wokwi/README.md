# Simulación en Wokwi (ESP32)

Simulación del monitor de tanque con el **ESP32 real del proyecto**, en [wokwi.com](https://wokwi.com) (gratis).

**Proyecto guardado (público):** https://wokwi.com/projects/478455258708481025

| Archivo | Qué es |
|---|---|
| [sketch.ino](sketch.ino) | Firmware del ESP32. Es el mismo que se carga en la placa física con Arduino IDE. |
| [diagram.json](diagram.json) | El circuito: componentes y cables. |
| [libraries.txt](libraries.txt) | Librerías: `LiquidCrystal I2C` y `ESP32Servo`. |

## Cómo abrirlo en Wokwi

1. En wokwi.com: **New Project → ESP32**.
2. Pegar `sketch.ino` en la pestaña `sketch.ino` y `diagram.json` en la pestaña `diagram.json`.
3. En **Library Manager → +**, agregar `LiquidCrystal I2C` y `ESP32Servo`.
4. Pulsar ▶. La cuenta gratis espera en una cola de compilación (1–2 min).

## Conexiones (las del documento del proyecto)

| Componente | ESP32 |
|---|---|
| HC-SR04 Trig / Echo | GPIO 5 / GPIO 18 |
| Sensor de agua (en Wokwi, un potenciómetro) | GPIO 34 |
| LCD 16x2 I2C SDA / SCL | GPIO 21 / GPIO 22 (dirección 0x27) |
| LED RGB R / G / B (220 Ω cada uno) | GPIO 25 / 26 / 27 |
| Buzzer | GPIO 14 |
| Servo (señal) | GPIO 13 |
| Pulsador (a GND) | GPIO 4 |

## Pruebas hechas en la simulación

| Prueba | Cómo | Resultado observado |
|---|---|---|
| Nivel crítico | Ultrasónico a 100 cm (tanque vacío) | LCD `Nivel: 0%`, LED rojo, buzzer, válvula ABIERTA, eventos `valvula_abierta` y `nivel_critico` |
| Tanque lleno | Clic en el ultrasónico, distancia 2 cm | LCD `98%`, LED verde, válvula CERRADA, eventos `valvula_cerrada` y `nivel_normal` |
| Fuga | Girar el potenciómetro a más de la mitad | LCD `!! FUGA !!`, LED rojo, buzzer, evento `fuga_detectada` |
| Silenciar | Pulsar el botón verde | Buzzer se calla, evento `boton_silenciar` |
| WiFi | Automático (red `Wokwi-GUEST`) | Evento `wifi_conectado`, la `W` aparece en el LCD |
| Envío | Cada 5 s | El monitor serie imprime el JSON que va a Flask |

## Conectar con el backend Flask

Cambiar `SERVIDOR_URL` en `sketch.ino` por la URL del backend (`POST /lecturas`). Mientras diga `TU-SERVIDOR`, el ESP32 no envía nada, pero todo lo local funciona (RNF-02).
Desde wokwi.com el ESP32 simulado **no llega a `localhost`** de la laptop: el backend tiene que estar en una URL pública. Con la placa física sí se usa la IP local de la laptop (misma red WiFi).

## Detalles de diseño para la defensa

- **Orientado a objetos:** una clase por componente físico (`SensorNivel`, `SensorFuga`, `Valvula`, `Indicador`, `Alarma`, `Boton`, `Pantalla`, `ClienteServidor`) y `ControladorTanque` que las coordina.
- **Orientado a eventos:** el controlador solo actúa cuando algo cambia (nivel, fuga, WiFi, botón). El **botón usa una interrupción de hardware** (`attachInterrupt`): el ESP32 avisa en el instante de la pulsación aunque el `loop` esté ocupado.
- **Histéresis:** la válvula abre bajo 20 % y cierra sobre 90 %, así no abre y cierra sin parar.
- **En el montaje real:** el Echo lleva divisor de voltaje (5 V a 3,3 V, RNF-03) y el servo va con fuente propia y GND común (RNF-04). En Wokwi no hace falta.
