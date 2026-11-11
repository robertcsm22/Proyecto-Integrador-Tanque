# Proyecto: Monitor IoT de tanque de agua

# Bitácora de: Roberto Carlos Sánchez Matus

Nota: el grupo trabajó en local durante las primeras semanas y subió todo al repositorio junto, de acuerdo con el profesor. Por eso los commits son posteriores a las fechas de cada semana.

## Semana 1 — 17/10/2026 al 24/10/2026

### Actividades realizadas
- Participé en la lluvia de ideas del proyecto con los componentes del kit (grifo con sensor, basurero inteligente, bomba de agua, entre otras); el grupo se decidió por el monitor de tanque de agua.
- Creé la estructura inicial del repositorio: carpetas `firmware/`, `backend/`, `dashboard/`, `docs/`, `pruebas/` y `bitacoras/`, con su README.
- Escribí `CONTRIBUTING.md` con el flujo de ramas, el formato de mensajes de commit y los tags por semana, y agregué la plantilla de Pull Request, la plantilla de bitácora, el `.gitignore` y `secrets.example.h`.

### Avances
- Commit: `9ab0b70` — "chore: estructura inicial del repositorio" (21/10/2026).
- El grupo definió la arquitectura por capas (percepción, red, procesamiento y aplicación) e hizo la primera simulación en Tinkercad.

### Decisiones técnicas
- Dejar las credenciales WiFi fuera del repositorio: `secrets.h` en `.gitignore` y una plantilla `secrets.example.h`.
- Pedir commits desde la cuenta de cada integrante, pequeños y frecuentes, porque el enunciado evalúa la trazabilidad individual.

### Pruebas efectuadas
- No aplica en mi parte.

### Próximos pasos
- Seguir el avance del diseño y del firmware.

## Semana 2 — 25/10/2026 al 31/10/2026

### Avance del equipo
- Diseño de la maqueta: reservorio elevado, tanque al centro con el ultrasónico sobre el nivel máximo y la electrónica en una caja separada del agua.
- Traslado de la simulación a Wokwi para usar un ESP32; pruebas de tanque vacío, tanque lleno, fuga, pulsador y WiFi.
- Se eligió el ESP32 y no el NodeMCU por pines y entradas analógicas, y el pulsador se atiende por interrupción.

### Pruebas del equipo
- Wokwi: 100 cm da nivel 0 % y válvula abierta; 2 cm da cerca de 98 % y válvula cerrada.

## Semana 3 — 01/11/2026 al 07/11/2026

### Avance del equipo
- Backend en Flask con SQLite, funciones puras con `map`, `filter` y `reduce`, simulador del ESP32 y 19 pruebas con pytest.
- Firmware del ESP32 en clases y armado del circuito en la protoboard por etapas.
- Prueba integral ESP32, servidor y dashboard por USB.

### Dificultades del equipo
- El servomotor no gira en las pruebas y probablemente hay que reemplazarlo.

## Semana 4 — 08/11/2026 al 11/11/2026

### Actividades realizadas
- Clonar el repositorio y revisar su estado actual: firmware y backend de mis compañeros, el dashboard recién subido, las simulaciones, las evidencias y las bitácoras.
- Revisar la documentación preparada por el grupo contra el código del repositorio y contra el montaje real, y ajustar lo que no coincidía.
- Integrar al repositorio la documentación en `docs/`: requerimientos, arquitectura, justificación del dispositivo, los tres paradigmas, pruebas y manual de ejecución.
- Agregar el `README.md` principal (el repositorio no tenía uno) y el `README.md` del dashboard.
- Trabajar en la rama `docs/documentacion-final` y subirla a GitHub para abrir el Pull Request a `develop`.

### Avances
- Commit: `a862093` — "chore: ignorar .pytest_cache"
- Commit: `58e97a1` — "docs(dashboard): README del dashboard"
- Commit: `9f576f4` — "docs(docs): requerimientos funcionales y no funcionales"
- Commit: `8f043b2` — "docs(docs): diagrama de arquitectura por capas"
- Commit: `e2739a8` — "docs(docs): justificacion del ESP32"
- Commit: `0564034` — "docs(docs): justificacion de los tres paradigmas"
- Commit: `04a09ab` — "docs(docs): pruebas, resultados y limitaciones"
- Commit: `8a6dbe3` — "docs(docs): manual de ejecucion e indice"
- Commit: `fb15bec` — "docs: README principal"
- `develop` quedó sincronizada con `main` antes de crear la rama.

### Dificultades encontradas
- Al revisar la documentación encontré diferencias con lo que realmente se armó: el divisor de voltaje del Echo estaba como 1 kΩ y 2 kΩ y se usaron dos de 10 kΩ (cerca de 2,5 V en el pin), y el servo aparecía con fuente propia cuando se alimenta desde VIN.
- El simulador del backend falla con `AttributeError` porque usa `args.url` y la opción `--url` no está definida. Es código de un compañero y se le avisó al grupo.
- La carpeta donde empecé no era el repositorio (`not a git repository`), y al escribir comandos seguidos en una sola línea la terminal no los reconocía.
- La bitácora de Christopher Lopez tiene el nombre con doble extensión (`.md.md`).

### Soluciones implementadas
- Documenté esas diferencias como limitaciones en `docs/pruebas.md` y en los requerimientos RNF-03 y RNF-04, en lugar de describir lo que debería ser.
- Cloné el repositorio en `C:\dev`, fuera de OneDrive, y usé la terminal de VS Code con un comando por línea; en CMD escribí el `.gitignore` con `echo` porque `printf` no existe.
- Hice un commit por cada documento para que el historial muestre qué aporté y cuándo.

### Decisiones técnicas
- Cada documento de paradigma tiene tres partes: dónde se aplica, evidencia en el código y justificación, porque el enunciado pide justificar y no solo mencionar.
- Documentar las limitaciones (servo sin girar, WiFi probado solo por USB, alimentación por VIN, divisor del Echo) como parte de las pruebas, como pide el requerimiento 4.
- Usar `py` en lugar de `python` en los comandos de Windows del manual y del README.
- Mantener el flujo `docs/documentacion-final` → `develop` → `main` con Pull Request, como indica `CONTRIBUTING.md`.

### Pruebas efectuadas
- Con `git status` verifiqué que solo se agregaran mis archivos y que no se modificara `backend/` ni `firmware/`.
- Con `git log --oneline` verifiqué que los 9 commits quedaran en orden sobre `develop`.

### Aprendizajes obtenidos
- Cómo trabajar con ramas, commits pequeños con mensajes con prefijo (`docs`, `chore`) y Pull Requests, y por qué dejan rastro individual del trabajo.
- Por qué conviene que la documentación describa lo que realmente se armó, incluidas las limitaciones, y no solo el diseño.
- Repasé los tres paradigmas del proyecto para poder explicarlos: clases por componente en el firmware, eventos e interrupción del pulsador, y funciones puras con `map`, `filter` y `reduce` en el backend.

### Próximos pasos
- Abrir y completar el Pull Request a `develop`, y luego de `develop` a `main`.
- Comprobar en una carpeta limpia: `git clone`, `py -m pytest` y `py app.py`, y que los diagramas y enlaces se vean bien en GitHub.
- Preparar la defensa individual: flujo de un dato, los tres paradigmas, la histéresis del 20 % y 90 %, el divisor de voltaje y las limitaciones.
