# Bitácora de desarrollo

## Proyecto: Monitor IoT de tanque de agua

**Estudiante:** Anthony Carranza Mejía  

## Semana 1 — 17/10/2026 al 24/10/2026

### Actividades realizadas

- Leí el enunciado del Proyecto Integrador y revisé los componentes disponibles en el kit: ESP32, sensor ultrasónico HC-SR04, sensor de agua, pantalla LCD I2C, LED RGB, buzzer, servomotor y pulsador.
- Realicé una primera simulación del circuito en Tinkercad utilizando Arduino Uno, debido a que Tinkercad no dispone de una placa ESP32.
- Revisé el repositorio del proyecto y eliminé archivos README que ya no eran necesarios en la rama `main`.

### Avances

- Se obtuvo una primera simulación funcional en Tinkercad que mostraba el nivel y el estado de la válvula en la pantalla LCD, utilizaba el LED RGB para indicar estados y activaba el buzzer como alarma.
- Se representó la válvula mediante un servomotor y se utilizaron eventos en el monitor serie para observar el comportamiento del sistema.
- Se registró la limpieza del repositorio mediante el commit `c904e1b`, con el mensaje: `Limpieza necesaria (readme ya no necesarios)`.

### Dificultades encontradas

- Al iniciar la simulación en Tinkercad, el circuito no mostraba el comportamiento esperado.
- La revisión del esquemático permitió identificar un cable que conectaba 5 V con GND, provocando un cortocircuito. También se encontraron conexiones sueltas en GND del sensor ultrasónico y en el LED.
- Tinkercad mostraba el mensaje genérico `invalid header file`, sin indicar directamente qué parte del programa provocaba el error.
- Se presentó un conflicto al utilizar `tone()` junto con la librería `Servo` en la simulación.

### Soluciones implementadas

- Revisé el circuito desde la vista de esquemático, eliminé el cable que provocaba el cortocircuito y corregí las conexiones incompletas.
- Para encontrar el error de compilación, fui comentando partes del código hasta aislar el problema.
- Reemplacé el uso de `tone()` por una generación manual del pitido mediante `digitalWrite` y `delayMicroseconds`, evitando el conflicto con la librería `Servo`.

### Decisiones técnicas

- Definí una histéresis para el control de la válvula: abrir cuando el nivel sea inferior al 20 % y cerrar cuando supere el 90 %. De esta manera se evita que la válvula cambie de estado continuamente cuando el nivel se encuentra cerca de un umbral.
- Utilicé Arduino Uno únicamente como alternativa para la primera simulación, considerando que el hardware final del proyecto sería un ESP32.

### Pruebas efectuadas

- En Tinkercad, una distancia de 113 cm representó un tanque vacío, equivalente a un nivel de 0 %.
- Para ese escenario se verificó la activación del LED rojo, el buzzer y la apertura de la válvula representada por el servomotor.
- Se comprobó que, una vez corregidas las conexiones y el conflicto de compilación, la simulación podía ejecutar la lógica prevista.

### Aprendizajes obtenidos

- Un error de cableado puede provocar síntomas que parecen errores de programación; por eso es necesario revisar tanto el código como el esquemático.
- Los simuladores pueden presentar limitaciones y mensajes de error poco descriptivos. Aislar partes del circuito y del programa facilita encontrar la causa.
- La histéresis ayuda a evitar cambios repetitivos de estado cuando una medición está cerca de un límite.
- Es importante seleccionar herramientas de simulación compatibles con el microcontrolador que se utilizará en el prototipo final.

### Próximos pasos

- Trasladar la simulación a un entorno que permita trabajar directamente con ESP32.
- Continuar la validación de los escenarios del sistema y preparar el firmware para su implementación física.

---

## Semana 2 — 25/10/2026 al 31/10/2026

### Actividades realizadas

- Trasladé la lógica del circuito a Wokwi, ya que este simulador permite utilizar un ESP32.
- Adapté la simulación a los pines definidos para el proyecto y continué las pruebas de los componentes y de la interacción entre ellos.
- Probé los escenarios principales del sistema: tanque vacío, tanque lleno, detección de fuga, silenciamiento de la alarma mediante el botón y conexión WiFi.
- Debido a que Wokwi no dispone del sensor de agua del kit, preparé una alternativa de simulación utilizando un potenciómetro conectado a una entrada analógica.
- Revisé el comportamiento del pulsador para conseguir que las pulsaciones rápidas fueran detectadas correctamente.

### Avances

- Se obtuvo una simulación del sistema en Wokwi utilizando un ESP32, con visualización del nivel en el LCD, indicación de estados mediante LED RGB, alarma sonora, representación de la válvula y eventos en el monitor serie.
- Se logró validar la lógica principal del sistema antes de trasladarla al prototipo físico.
- Se comprobó el comportamiento de la alarma, el control de la válvula y la detección simulada de fuga.
- Se guardó la simulación como proyecto público en Wokwi:  
  https://wokwi.com/projects/478455258708481025

### Dificultades encontradas

- Wokwi no cuenta con el sensor de agua utilizado en el kit, por lo que no era posible probar directamente ese componente en la simulación.
- Al presionar el botón rápidamente, en algunas ocasiones el programa no detectaba la acción.
- Al cambiar la lectura del botón a una interrupción, la compilación presentó el error `dangerous relocation`.
- Fue necesario adaptar la implementación de la interrupción a las condiciones específicas del ESP32.

### Soluciones implementadas

- Sustituí el sensor de agua por un potenciómetro conectado al GPIO 34. Ambos permiten trabajar con una señal analógica, por lo que el potenciómetro permitió variar los valores de entrada y comprobar la lógica de detección de fuga.
- Cambié la lectura del pulsador a una interrupción de hardware mediante `attachInterrupt`, configurada para detectar el flanco de bajada. Así, la pulsación no depende únicamente de que el ciclo `loop` consulte el estado del botón en el momento adecuado.
- Saqué la rutina de interrupción fuera de la clase y la marqué con `IRAM_ATTR`, atendiendo a los requisitos del ESP32 para ejecutar este tipo de función.
- Mantuve la sustitución del sensor de agua limitada al entorno de simulación; para el prototipo físico se utilizaría el sensor real.

### Decisiones técnicas

- Utilicé una interrupción de hardware para el pulsador, de manera que la acción de silenciar la alarma se gestione como un evento.
- Mantuve la organización por clases para separar las responsabilidades de cada componente.
- Definí el envío de información al servidor cada cinco segundos, procurando que la comunicación no bloquee el control local del sistema, de acuerdo con el requisito RNF-02.
- Utilicé el potenciómetro como una entrada analógica de prueba y no como sustituto definitivo del sensor de agua.

### Pruebas efectuadas

- **Tanque vacío:** una distancia de 100 cm produjo un nivel de 0 %, con LED rojo, buzzer activo y válvula abierta.
- **Tanque lleno:** una distancia de 2 cm produjo un nivel aproximado de 98 %, con LED verde y válvula cerrada.
- **Detección de fuga:** al modificar el valor del potenciómetro se activó la condición `!! FUGA !!`.
- **Pulsador:** se comprobó el registro del evento `boton_silenciar`.
- **WiFi:** se comprobó el evento `wifi_conectado` en la simulación.

### Aprendizajes obtenidos

- El polling consulta repetidamente el estado de una entrada, mientras que una interrupción permite reaccionar cuando ocurre un evento específico.
- En el ESP32, las rutinas de interrupción tienen restricciones particulares; por eso es importante ubicarlas y declararlas de acuerdo con los requisitos de la plataforma.
- Un potenciómetro puede utilizarse para simular una lectura analógica cuando el simulador no dispone del sensor real, siempre que se documenten sus limitaciones.
- Validar primero la lógica en un simulador compatible con el microcontrolador reduce los problemas al trasladar el programa al hardware.

### Próximos pasos

- Preparar el proyecto para Arduino IDE y organizar el firmware en archivos separados `.h` y `.cpp`.
- Instalar el soporte de ESP32 y las librerías necesarias.
- Continuar con el armado del circuito físico y comprobar cada componente por separado.

---

## Semana 3 — 01/11/2026 al 07/11/2026

### Actividades realizadas

- Organicé el firmware para su implementación en Arduino IDE, utilizando la estructura `firmware/tanque_iot/` y una clase por componente: `SensorNivel`, `SensorFuga`, `Valvula`, `Indicador`, `Alarma` y `Comunicacion`, además del archivo `config.h`.
- Creé la rama `feature/firmware-esp32` para trabajar en el desarrollo del firmware.
- Instalé el soporte para ESP32 en Arduino IDE (core 3.3.12) y las librerías LiquidCrystal I2C y ESP32Servo.
- Instalé el controlador CP2102 para que la computadora reconociera el ESP32 mediante el puerto COM7.
- Revisé las piezas disponibles en el kit y adapté el circuito a los componentes y materiales existentes.
- Armé el circuito en la protoboard por etapas, verificando cada parte antes de continuar:
  1. Alimentación y pantalla LCD (SDA → D21, SCL → D22).
  2. Sensor ultrasónico HY-SRF05 (Trig → D5, Echo → divisor de voltaje → D18).
  3. Sensor de agua (AO → D34, VCC → 3V3).
  4. LED RGB (D25, D26, D27) y buzzer (D14).
  5. Pulsador (D4).
  6. Servomotor (D13).
- Configuré las credenciales de conexión en `secrets.h`, evitando incluirlas en el repositorio mediante `.gitignore`.
- Incorporé las simulaciones de Wokwi y Tinkercad en `pruebas/simulacion/`.
- Probé el envío de lecturas del ESP32 al servidor y al dashboard del grupo.

### Avances

- Se completó la estructura del firmware con las clases `SensorNivel`, `SensorFuga`, `Valvula`, `Indicador`, `Alarma` y `Comunicacion`.
- El programa compiló sin errores y utilizó aproximadamente el 81 % de la memoria de programa y el 15 % de la RAM.
- Se consiguió que el circuito físico funcionara con todos los componentes, excepto el servomotor.
- Se verificó el envío de datos al dashboard mediante USB: se recibieron 25 lecturas, con niveles entre 0 % y 100 %, además de eventos de nivel, válvula y fuga.
- Se generó evidencia de las pruebas en `pruebas/evidencias/`.

### Dificultades encontradas

- La computadora no reconocía inicialmente el ESP32.
- Al intentar cargar el programa apareció el mensaje `port busy`.
- El ESP32 se reiniciaba solo y la pantalla LCD permanecía en el mensaje `Iniciando...`.
- Los materiales disponibles no coincidían completamente con los valores de resistencias previstos en el diseño; únicamente se contaba con resistencias de 1 kΩ y 10 kΩ.
- La pantalla LCD llegó a mostrar caracteres incorrectos.
- El sensor ultrasónico registraba distancias de 0,1 cm o no recibía el eco.
- El sensor de agua no detectaba correctamente la condición de fuga al inicio.
- Los estados de fuga y nivel cambiaban repetidamente debido a variaciones en las lecturas.
- La válvula no llegaba a cerrarse.
- Una sola pulsación del botón podía contarse varias veces.
- El servomotor recibía la señal, pero zumbaba y vibraba sin llegar a girar.

### Soluciones implementadas

- Instalé el controlador CP2102 para permitir la comunicación entre la computadora y el ESP32.
- Cerré Arduino IDE cuando mantenía ocupado el puerto serial y cambié el cable USB a otro puerto, ya que el anterior no suministraba suficiente corriente.
- Utilicé dos resistencias de 10 kΩ para formar el divisor de voltaje de la señal Echo del sensor ultrasónico, reduciendo la señal de 5 V a aproximadamente 2,5 V. También utilicé una resistencia de 1 kΩ en el negativo común del LED.
- Revisé y ajusté las conexiones de la pantalla LCD y reinicié el ESP32 para corregir la visualización de caracteres.
- Revisé y apreté las conexiones del divisor de voltaje del Echo para mejorar la lectura del sensor ultrasónico.
- Medí el sensor de agua en condiciones reales: obtuvo un valor de 4095 en seco y aproximadamente 3300 al estar mojado. Con esos resultados calibré el umbral de fuga en 3700.
- Agregué un margen para evitar cambios repetidos: la condición de fuga se desactiva únicamente cuando el valor supera 3950, y el estado del nivel cambia solo cuando la lectura supera el límite por un 3 %.
- Establecí el 100 % del nivel a 4 cm del sensor mediante `DISTANCIA_LLENO_CM`, debido a que el ultrasónico no mide correctamente a distancias inferiores a 2 cm.
- Configuré el botón para que solo cuente cuando la alarma está sonando.
- Probé el servomotor de forma independiente, con un programa aparte y utilizando también un cargador de celular. Como continuó sin girar, se determinó que probablemente está dañado y debe reemplazarse.

### Decisiones técnicas

- Configuré `FUGA_CON_VALOR_BAJO = true`, porque el módulo de sensor de agua LM393 utilizado reduce su valor de lectura cuando se moja.
- Utilicé `millis()` en lugar de `delay()` para gestionar los tiempos, evitando detener el control local mientras se realizan otras operaciones y se envían datos.
- Guardé las credenciales WiFi en `secrets.h`, archivo excluido del repositorio mediante `.gitignore`.
- Establecí los pines y umbrales en `config.h`, para facilitar la calibración sin modificar directamente las clases.
- Utilicé la mediana de tres mediciones del sensor ultrasónico para descartar lecturas aisladas incorrectas, en relación con el requisito RNF-01.
- Alimenté el servomotor desde el pin VIN debido a que no se disponía de una fuente de alimentación independiente. Esta condición queda como una limitación respecto al requisito RNF-04.
- Se mantuvo pendiente el reemplazo del servomotor, ya que las pruebas independientes indicaron que no giraba incluso al reducir la carga del sistema.

### Pruebas efectuadas

- **Sensor ultrasónico:** con un objeto ubicado a 10 cm, las lecturas se encontraron entre 8,9 cm y 12,2 cm.
- **Sensor de agua:** se obtuvo un valor de 4095 en seco y valores entre 3300 y 3635 al mojarse; con el umbral configurado en 3700 se detectó la condición de fuga.
- **LED RGB:** se verificó el color rojo para estado crítico, amarillo para nivel bajo y verde para nivel normal.
- **Buzzer:** se comprobó la activación de la alarma ante un nivel crítico y ante una fuga.
- **Pulsador:** se comprobó que silenciara la alarma y generara el evento `alarma_silenciada`.
- **Servomotor:** recibió la señal de control, pero no giró durante las pruebas realizadas, incluso al probarlo por separado. Resultado no satisfactorio; requiere reemplazo.
- **Comunicación con el dashboard:** mediante USB se enviaron 25 lecturas, con valores de nivel de 0 % a 100 % y eventos relacionados con nivel, válvula y fuga. La comunicación se comprobó por USB; la transmisión por WiFi quedó pendiente de configuración y validación en el hardware.

### Aprendizajes obtenidos

- El ESP32 trabaja con niveles lógicos de 3,3 V; por eso la señal Echo de 5 V del sensor ultrasónico requiere un divisor de voltaje antes de conectarse al microcontrolador.
- Los sensores físicos presentan variaciones y ruido en sus lecturas. La calibración con mediciones reales y el uso de márgenes ayudan a evitar falsas detecciones y cambios repetitivos de estado.
- Es importante calibrar los umbrales a partir de valores medidos en el hardware y no únicamente de valores supuestos.
- Probar un componente de manera independiente permite determinar si una falla proviene del código, de las conexiones o del propio componente.
- La alimentación eléctrica y la calidad de las conexiones influyen directamente en la estabilidad del ESP32 y de los periféricos.
- Separar el firmware en clases y archivos facilita el mantenimiento, la lectura y la calibración del sistema.

### Próximos pasos

- Conseguir un repuesto del servomotor con algun amigo que no lo necesite.
- Hacer nuevas pruebas para probar la funcionalidad con el servomotor.
- Configurar el Wi-fi de la universidad para ser compatible con el circuito.