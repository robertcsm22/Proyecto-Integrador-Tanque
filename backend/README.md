# Backend (Flask + SQLite)

Recibe las lecturas del ESP32, las guarda y las muestra en el dashboard.

- **Funcional:** `procesamiento.py` usa funciones puras con `map`, `filter` y `reduce`.
- **Orientado a eventos:** cada lectura nueva se compara con la anterior y se registran los cambios (válvula, nivel crítico, fuga…).

## Archivos

- `app.py`: rutas y servidor
- `db.py`: base de datos (tablas `lecturas` y `eventos`)
- `procesamiento.py`: validación, eventos, promedios, alarmas y consumo
- `simulador.py`: envía lecturas falsas para probar sin la maqueta
- `tests/`: 19 pruebas con `pytest`

## Rutas

- `POST /lecturas`: guarda una lectura y devuelve los eventos detectados
- `GET /lecturas`: últimas lecturas
- `GET /eventos`: últimos eventos
- `GET /resumen`: promedios, mínimo, máximo, alarmas y consumo
- `GET /`: dashboard

## Cómo ejecutar

```bash
cd backend
python -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
python app.py
```

Abrir en http://localhost:5000 (desde el celular o el ESP32, usar la IP de la laptop).

Si el ESP32 no se conecta, permitir Python en el Firewall de Windows.

## Probar

```bash
python simulador.py --intervalo 1 --veces 60   # sin el ESP32
python -m pytest -v                            # pruebas
```