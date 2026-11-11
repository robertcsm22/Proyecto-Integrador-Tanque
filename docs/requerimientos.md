# Requerimientos del sistema

## Funcionales

| ID | Requerimiento | Dónde se cumple |
|----|---------------|-----------------|
| RF-01 | Medir la distancia al agua con el ultrasónico y convertirla a % de llenado | `SensorNivel` |
| RF-02 | Mostrar el nivel (%) y el estado en el LCD 16x2 | `Indicador::mostrar` |
| RF-03 | LED RGB: verde (> 50 %), amarillo (20–50 %), rojo (< 20 %) | `Indicador::colorSegunEstado` |
| RF-04 | Buzzer con nivel crítico o fuga | `Alarma::actualizar` |
| RF-05 | Detectar fuga o desborde en la base con el sensor de agua | `SensorFuga` |
| RF-06 | Abrir la válvula bajo 20 % y cerrarla sobre 90 % (histéresis) | `Valvula::controlar` |
| RF-07 | Silenciar la alarma con el pulsador | `Alarma` (interrupción) |
| RF-08 | Enviar cada lectura por WiFi cada 5–10 s | `Comunicacion::enviar` (cada 5 s) |
| RF-09 | Guardar lecturas y eventos en la base de datos | `db.py`, tablas `lecturas` y `eventos` |
| RF-10 | Dashboard con nivel actual, válvula, gráfico histórico y alarmas | `dashboard/` |

## No funcionales

| ID | Requerimiento | Dónde se cumple |
|----|---------------|-----------------|
| RNF-01 | Error de medición < ±1 cm frente a una regla | Mediana de 3 ecos en `SensorNivel`; se valida en la prueba P01 |
| RNF-02 | Sigue funcionando localmente sin WiFi | El control local no depende de `Comunicacion`; timeout HTTP de 2 s; reconexión automática |
| RNF-03 | Pin Echo protegido con divisor de voltaje 5 V → 3,3 V | Montaje con dos resistencias de 10 kΩ (unos 2,5 V en el pin). Con 1 kΩ y 2 kΩ daría 3,3 V; ver [pruebas.md](pruebas.md) |
| RNF-04 | Servo con fuente propia y GND común | **Limitación:** por ahora se alimenta desde VIN, sin fuente aparte, y el servo no gira; ver [pruebas.md](pruebas.md) |
| RNF-05 | Código en clases y módulos, en GitHub con commits frecuentes | Firmware en clases `.h/.cpp`; backend en módulos |
| RNF-06 | Dashboard se actualiza solo cada ≤ 10 s sin recargar | `dashboard/js/app.js` (cada 5 s) |
