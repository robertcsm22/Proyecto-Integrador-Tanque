# Justificación del dispositivo (Requerimiento 2)

Se eligió el **ESP32 DevKit** como controlador. El kit también trae un **NodeMCU (ESP8266)**,
que se descartó por estas razones:

| Necesidad del proyecto | ESP32 | NodeMCU (ESP8266) |
|------------------------|-------|-------------------|
| Entradas analógicas (sensor de agua; a futuro potenciómetro de calibración o un segundo sensor) | 18 canales ADC de 12 bits (0–4095) | **1** canal ADC de 10 bits |
| Pines para 8 componentes a la vez (ultrasónico, LCD, RGB ×3, buzzer, servo, pulsador, sensor de agua) | ~25 GPIO usables | ~9 GPIO usables, varios con funciones de arranque |
| Control del servo por PWM sin afectar otras salidas | 16 canales PWM por hardware (LEDC) | PWM por software |
| Interrupción para el pulsador | En cualquier GPIO | En casi todos los pines (GPIO 0 a 15) |
| WiFi para enviar al servidor | Sí | Sí |
| Programación con Arduino IDE | Sí | Sí |

El sensor de agua necesita una entrada analógica. Con el NodeMCU esa única entrada
quedaría ocupada, sin margen para crecer, y faltarían pines para conectar todo a la vez.
El ESP32 cubre todo con espacio de sobra y cuesta prácticamente lo mismo.

## Por qué no una Raspberry Pi

La Raspberry Pi es una computadora completa: necesita sistema operativo, tarda en
arrancar y no tiene entradas analógicas. Para leer sensores y mover un servo en tiempo
real basta un microcontrolador. La laptop ya hace de servidor.

## Consideraciones eléctricas

- El ESP32 trabaja a **3,3 V**. El pin Echo del ultrasónico (HY-SRF05, compatible con el
  HC-SR04) entrega 5 V, por eso lleva un divisor de voltaje (RNF-03). El diseño original
  era de 1 kΩ y 2 kΩ (5 V × 2/3 ≈ 3,3 V); en el armado se usaron dos resistencias de
  10 kΩ, que dan unos 2,5 V, un valor justo para el ESP32 (ver [pruebas.md](pruebas.md)).
- El servo consume picos de corriente que pueden reiniciar el ESP32. Lo ideal es una
  fuente propia con GND común (RNF-04); por ahora se alimenta desde VIN, y esto queda
  como limitación documentada en [pruebas.md](pruebas.md).
- El sensor de agua se alimenta a 3,3 V para que su salida no supere el máximo del ADC.
