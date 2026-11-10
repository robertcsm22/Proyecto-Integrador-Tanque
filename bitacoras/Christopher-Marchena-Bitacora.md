# Proyecto: Monitor IoT de tanque de agua

# Bitácora de: Christopher Marchena Telles 

## Semana 1 — [17/10/2026] a [24/10/2026]
### Actividades realizadas

- Analicé el enunciado del Proyecto Integrador y comparé ideas de proyecto con los componentes del kit.

- Propuse el monitor de tanque de agua: problema real en Guanacaste en época seca que aprovecha casi todo el kit.

- Investigué la arquitectura IoT por capas y las herramientas de trabajo (VS Code, Arduino IDE, Git y GitHub).

### Avances

- Lista de componentes, plan de trabajo por semanas.

### Dificultades encontradas
- No tenía muy claro qué es una solución IoT ni si nuestro sistema correspondía a una arquitectura por capas.

### Soluciones implementadas

- Investigué el concepto de IoT e identifiqué las cuatro capas del sistema: percepción (sensores y ESP32), red (WiFi + HTTP), procesamiento (Flask + SQLite) y aplicación (dashboard).

### Decisiones técnicas
- HTTP POST con JSON para comunicar el ESP32 con el servidor: más simple que MQTT.

### Pruebas efectuadas
- No aplica: semana de análisis y diseño.

### Aprendizajes obtenidos
- Qué es una solución IoT, cómo se organiza por capas y cómo leer un diagrama de bloques.
### Próximos pasos
- Diseñar la maqueta.

## Semana 2 — [25/10/2026] a [31/10/2026]

### Actividades realizadas
- Diseñé la maqueta: reservorio elevado, tanque al centro sobre una bandeja con el ultrasónico en un puente, y caja de electrónica separada del agua.
- Definí una idea de las medidas, materiales y la válvula de pinza con el servo.

### Avances
- Diseño completo de la maqueta con medidas, lista de materiales y modelo 3D de referencia.

### Dificultades encontradas
- En el primer diseño el reservorio quedaba más bajo que el borde del tanque, y el agua no hubiera bajado por gravedad.
- No distinguía qué componentes trae el kit y cuáles hay que conseguir aparte.

### Soluciones implementadas
- Subí el soporte del reservorio a unos 30 cm para que el agua baje todo el recorrido.
- Separé los componentes en tres listas: los del kit, los que hay que revisar y los materiales de la maqueta.

### Decisiones técnicas
- Tanque de plástico transparente (no vidrio) y sensor al menos 3 cm por encima del nivel máximo.
- Electrónica en una caja separada al menos 8 cm del tanque para evitar el contacto con el agua.

### Pruebas efectuadas
- No aplica: semana de diseño.

### Aprendizajes obtenidos
- La altura del sensor y el nivel máximo son constantes de calibración del firmware.

### Próximos pasos
- Desarrollar el backend.

## Semana 3 — [01/11/2026] a [07/11/2026]

### Actividades realizadas

- Creé el backend en Flask con SQLite: tablas `lecturas` y `eventos` (`db.py`), funciones puras de procesamiento (`procesamiento.py`), rutas `POST /lecturas`, `GET /lecturas`, `/eventos` y `/resumen` (`app.py`), un simulador del ESP32 y pruebas automáticas con `pytest`.

### Avances
- Commit: `a98788e` — "Creacion de parte de contenido del Backend"
- Commit: `f70910e` — "creacion de los Test"
- Commit: `ac303e1` — "Agregacion del REDME"

### Dificultades encontradas
- Al instalar las dependencias, Windows respondió "Python was not found" y redirigía a la Microsoft Store: el comando `python` no estaba disponible en la terminal.
- Al correr las pruebas, 18 de 19 pasaron. La prueba `test_dashboard_se_sirve_en_la_raiz` falló con 404 porque `app.py` sirve el dashboard desde la carpeta `../dashboard`, que todavía no estaba en el repositorio.

### Soluciones implementadas
- Usé el lanzador `py` de Windows en lugar de `python` (`py -m pip install -r requirements.txt` y `py -m pytest -v`).
- Identifiqué que el backend depende del módulo del dashboard para servir la página principal; el resto del backend funciona correctamente. [completar: copiaste la carpeta para probar localmente o esperaste a que se subiera, y el resultado]

### Decisiones técnicas
- Separar las funciones puras (`procesamiento.py`) del acceso a datos (`db.py`) para probarlas sin base de datos (paradigma funcional).
- Detectar eventos comparando cada lectura con la anterior (paradigma orientado a eventos), con las reglas en una lista `REGLAS_EVENTOS`.
- SQLite porque no necesita servidor, y validación de cada lectura con respuesta 400 si viene mal.

### Pruebas efectuadas
- `py -m pytest -v`: 19 de 19 pruebas pasaron.
### Aprendizajes obtenidos
- Qué es una función pura y cómo `map`, `filter` y `reduce` reemplazan los ciclos con acumuladores.
### Próximos pasos
- Armar la maqueta y probar el backend con el ESP32 real.
