# Pruebas y resultados

Resumen de lo que se probó, cómo, qué resultó, qué limitaciones se encontraron y qué se puede mejorar. Las evidencias están en [`pruebas/`](../pruebas/).

## 1. Pruebas del backend (automáticas)

Se ejecutan con `py -m pytest -v` dentro de `backend/`. Hay 19 pruebas: 5 de la API (`test_api.py`) y 14 de las funciones puras (`test_procesamiento.py`).

| Qué se prueba | Resultado |
|---|---|
| POST válido guarda la lectura y GET la devuelve en orden | Pasa |
| POST inválido responde 400 con la lista de errores | Pasa |
| Un cambio de válvula queda registrado como evento | Pasa |
| `/resumen` calcula estadísticas | Pasa |
| Validación, eventos, conteo de alarmas, promedios y consumo | Pasan |
| Las funciones no modifican sus argumentos | Pasa |
| `GET /` sirve el dashboard | Pasa cuando `dashboard/index.html` está en el repositorio; falla con 404 si no |

**Resultado:** 19 de 19 pasan con el dashboard en el repositorio. Sin `dashboard/index.html` (como estaba `main` antes de subirlo), pasan 18 de 19 y falla solo la prueba del dashboard.

## 2. Simulaciones

Antes de armar el circuito se validó la lógica en simuladores. Ver [`pruebas/simulacion/`](../pruebas/simulacion/).

| Simulador | Prueba | Resultado |
|---|---|---|
| Tinkercad (Arduino Uno) | Distancia de 113 cm (tanque vacío) | Nivel 0 %, LED rojo, buzzer, válvula abierta |
| Wokwi (ESP32) | Distancia de 100 cm | Nivel 0 %, LED rojo, buzzer, válvula abierta |
| Wokwi (ESP32) | Distancia de 2 cm | Nivel cerca de 98 %, LED verde, válvula cerrada |
| Wokwi (ESP32) | Potenciómetro en lugar del sensor de agua | Se activa la condición de fuga |
| Wokwi (ESP32) | Pulsador | Silencia la alarma y se registra el evento |
| Wokwi (ESP32) | WiFi | Evento de conexión |

Limitaciones de los simuladores: Tinkercad no tiene ESP32, por eso se usó un Arduino Uno; Wokwi no tiene el sensor de agua del kit, por eso se usó un potenciómetro.

## 3. Pruebas en el hardware real (ESP32)

| Componente | Cómo se probó | Resultado |
|---|---|---|
| Ultrasónico | Objeto a 10 cm | Lecturas entre 8,9 y 12,2 cm |
| Sensor de agua | En seco y mojado | 4095 en seco y entre 3300 y 3635 mojado; umbral de fuga en 3700 |
| LED RGB | Distintos niveles | Rojo crítico, amarillo bajo, verde normal |
| Buzzer | Nivel crítico y fuga | Suena |
| Pulsador | Con la alarma sonando | Silencia y genera `alarma_silenciada` |
| LCD | Mostrar estado | Funciona tras corregir conexiones |
| Servomotor | Prueba aislada con otro programa y con cargador de celular | **No gira** (zumba y vibra); probablemente dañado, hay que reemplazarlo |
| Compilación | `arduino-cli compile` | Sin errores; 81 % de memoria de programa y 15 % de RAM |

## 4. Prueba integral: ESP32 → backend → dashboard

El 9 de noviembre de 2026 se hizo una prueba con el ESP32 conectado por USB a la laptop, enviando lecturas al servidor Flask y mostrándolas en el dashboard.

- Se registraron lecturas con niveles de 0 a 100 % (el archivo de evidencia muestra 28 lecturas en total).
- Eventos observados: `nivel_critico`, `valvula_abierta`, `valvula_cerrada`, `nivel_normal`, `fuga_detectada` y `fuga_resuelta`.
- Resumen del servidor: nivel promedio de 52,6 % y 3 alarmas.
- Evidencias: [`prueba-integral-eventos.txt`](../pruebas/evidencias/prueba-integral-eventos.txt) y [`prueba-integral-dashboard.jpg`](../pruebas/evidencias/prueba-integral-dashboard.jpg).

El enlace fue por USB. **La transmisión por WiFi no se ha validado en el hardware.**

## 5. Respuesta ante distintas condiciones

| Condición | Comportamiento |
|---|---|
| Ruido en el sensor ultrasónico | La mediana de 3 mediciones descarta lecturas aisladas |
| Lectura cerca de un umbral | Los márgenes (3 % en nivel, 250 en fuga) evitan cambios repetidos |
| Una pulsación | Antirrebote de 200 ms; solo cuenta si la alarma está sonando |
| Sin WiFi o sin servidor | Tiempo de espera de 2 s; la válvula, el LCD, el LED y el buzzer siguen funcionando |
| Sin eco del ultrasónico | Se toma como tanque vacío |
| Lectura inválida enviada al servidor | Responde 400 y no la guarda |

## 6. Problemas encontrados y cómo se resolvieron

| Problema | Solución |
|---|---|
| El computador no reconocía el ESP32 | Instalar el controlador CP2102 |
| `port busy` al cargar el código | Cerrar el monitor serie; cambiar el cable USB a otro puerto |
| El ESP32 se reiniciaba solo | Cable o puerto con poca corriente |
| Ultrasónico con lecturas de 0,1 cm o sin eco | Apretar las conexiones del divisor de voltaje |
| Sensor de agua no detectaba la fuga | Medir en seco y mojado y calibrar el umbral |
| Estado y fuga cambiaban sin parar | Márgenes de histéresis |
| Una pulsación se contaba varias veces | Antirrebote y contar solo con la alarma sonando |
| `tone()` en conflicto con `Servo` (Tinkercad) | Pitido manual con `digitalWrite` y `delayMicroseconds` |
| `dangerous relocation` al usar interrupción (Wokwi) | Mover la rutina fuera de la clase y marcarla `IRAM_ATTR` |

## 7. Limitaciones

- **Servomotor sin funcionar:** la válvula física no se mueve; solo se comprobó que recibe la señal.
- **Alimentación del servo (RNF-04):** se alimenta desde VIN, sin fuente aparte.
- **WiFi:** probado en simulación, no en la placa física. La red de la universidad puede bloquear el dispositivo.
- **Divisor de voltaje del Echo:** el armado con dos resistencias de 10 kΩ entrega cerca de 2,5 V, un valor justo para el umbral de entrada del ESP32 (la documentación de pines indica 1 kΩ y 2 kΩ, que dan 3,3 V).
- **Prueba de consumo en tanque real:** el consumo en litros se calcula sobre una capacidad configurada (4 L), no medida.

## 8. Posibles mejoras

- Reemplazar el servomotor y alimentarlo con una fuente de 5 V aparte, con GND común.
- Armar el divisor del Echo con 10 kΩ arriba y dos de 10 kΩ en serie abajo: 5 V × 20 / 30 ≈ 3,3 V con las resistencias que hay.
- Validar el envío por WiFi, por ejemplo con el hotspot del celular si la red de la universidad lo bloquea.
- Agregar notificaciones (por ejemplo, un mensaje) cuando haya alarma.
- Medir el consumo con un sensor de flujo.
