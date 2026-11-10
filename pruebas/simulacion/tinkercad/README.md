# Simulación en Tinkercad: Monitor de tanque de agua

Diseño en Tinkercad: **"Monitor tanque de agua IoT"** (cuenta de Tinkercad del grupo, sección Diseños → Circuitos).
Código: [tanque_tinkercad.ino](tanque_tinkercad.ino) (es el mismo que está cargado en Tinkercad, en Código → Texto).

Tinkercad no tiene ESP32 ni sensor de agua, así que la simulación usa un **Arduino Uno** y un **sensor de humedad de suelo** en lugar del sensor de agua. La lógica y las clases son las mismas que van al ESP32. Lo único que cambia son los pines y que el envío al servidor se simula por el monitor serie.

## Conexiones

| Componente | Pin del componente | Arduino Uno |
|---|---|---|
| Ultrasónico HC-SR04 | VCC / Trig / Echo / GND | 5V / D7 / D6 / GND |
| LCD 16x2 I2C (MCP23008, 0x20) | VCC / GND / SDA / SCL | 5V / GND / SDA / SCL |
| LED RGB (cátodo común) | Rojo / Verde / Azul | D11 / D10 / D9 |
| LED RGB | Cátodo | Resistencia 220 Ω → GND |
| Buzzer (piezo) | + / − | D8 / GND |
| Micro servo (válvula) | Señal / Potencia / Tierra | D3 / 5V / GND |
| Sensor de humedad (fuga) | VCC / SIG / GND | 5V / A1 / GND |
| Pulsador | Un lado / lado opuesto | D2 / GND (INPUT_PULLUP) |

## Cómo probarlo

1. Pulsar **Iniciar simulación** y abrir el **Monitor en serie** (abajo, en el panel de Código).
2. **Nivel:** clic en el ultrasónico y arrastrar el objeto que aparece encima. Con el tanque de 100 cm:
   - más de 80 cm de distancia → nivel bajo 20 % → LED rojo, buzzer y la válvula (servo) se abre;
   - 50–80 cm → nivel 20–50 % → LED amarillo;
   - menos de 10 cm → nivel sobre 90 % → la válvula se cierra (histéresis 20 %/90 %).
3. **Fuga:** clic en el sensor de humedad y subir el deslizador → LCD muestra `!! FUGA !!`, LED rojo y buzzer.
4. **Silenciar:** pulsar el botón con la alarma sonando → el buzzer se calla hasta el próximo evento.
5. En el monitor serie aparecen los **eventos** (`EVENTO: valvula_abierta`, `EVENTO: nivel_critico`…) y cada 5 s una **lectura** (`LECTURA nivel=… fuga=… valvula=… estado=…`). En el ESP32 esa lectura es la que se envía por HTTP a Flask.

## Diferencias con el montaje real (para la documentación)

- **Placa:** Uno en vez de ESP32 (Tinkercad no tiene ESP32). En el ESP32 cambian los pines y no hace falta nada de lo siguiente.
- **Buzzer:** en Tinkercad `tone()` no compila junto con la librería `Servo`, por eso el pitido se genera a mano (`Alarma::pitido`). En el ESP32 se puede usar `tone()` o `ledcWriteTone()`.
- **Estados:** son constantes `int` (`NORMAL`, `BAJO`, `CRITICO`) porque el preprocesador de Tinkercad no acepta un `enum` como parámetro de un método.
- **Divisor de voltaje del Echo:** no hace falta en el Uno (trabaja a 5 V); en el ESP32 sí (RNF-03).
- **Servo:** en la simulación se alimenta del 5V del Arduino; en el real va con su fuente aparte y GND común (RNF-04).
