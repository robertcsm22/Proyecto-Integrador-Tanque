"""Acceso a la base de datos SQLite (RF-09).

Tablas:
  lecturas: cada lectura que envia el ESP32
  eventos:  cambios detectados (alarmas, apertura y cierre de valvula, fugas)
"""

import sqlite3
from datetime import datetime

ESQUEMA = """
CREATE TABLE IF NOT EXISTS lecturas (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    fecha         TEXT    NOT NULL,
    distancia_cm  REAL    NOT NULL,
    nivel         INTEGER NOT NULL,
    fuga          INTEGER NOT NULL,
    valvula       TEXT    NOT NULL,
    estado        TEXT    NOT NULL,
    alarma        INTEGER NOT NULL
);

CREATE TABLE IF NOT EXISTS eventos (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    fecha       TEXT    NOT NULL,
    tipo        TEXT    NOT NULL,
    detalle     TEXT    NOT NULL,
    lectura_id  INTEGER REFERENCES lecturas(id)
);

CREATE INDEX IF NOT EXISTS idx_lecturas_fecha ON lecturas(fecha);
CREATE INDEX IF NOT EXISTS idx_eventos_fecha ON eventos(fecha);
"""


def conectar(ruta):
    conexion = sqlite3.connect(ruta)
    conexion.row_factory = sqlite3.Row
    return conexion


def crear_tablas(ruta):
    with conectar(ruta) as conexion:
        conexion.executescript(ESQUEMA)


def ahora():
    return datetime.now().isoformat(timespec="seconds")


def _lectura_a_dict(fila):
    datos = dict(fila)
    datos["fuga"] = bool(datos["fuga"])
    datos["alarma"] = bool(datos["alarma"])
    return datos


def guardar_lectura(ruta, lectura, eventos):
    """Guarda una lectura y sus eventos en una sola transaccion. Devuelve la lectura guardada."""
    fecha = ahora()
    with conectar(ruta) as conexion:
        cursor = conexion.execute(
            "INSERT INTO lecturas (fecha, distancia_cm, nivel, fuga, valvula, estado, alarma) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)",
            (fecha, lectura["distancia_cm"], lectura["nivel"], int(lectura["fuga"]),
             lectura["valvula"], lectura["estado"], int(lectura["alarma"])),
        )
        lectura_id = cursor.lastrowid
        conexion.executemany(
            "INSERT INTO eventos (fecha, tipo, detalle, lectura_id) VALUES (?, ?, ?, ?)",
            [(fecha, e["tipo"], e["detalle"], lectura_id) for e in eventos],
        )
    return {"id": lectura_id, "fecha": fecha, **lectura}


def ultima_lectura(ruta):
    with conectar(ruta) as conexion:
        fila = conexion.execute("SELECT * FROM lecturas ORDER BY id DESC LIMIT 1").fetchone()
    return _lectura_a_dict(fila) if fila else None


def obtener_lecturas(ruta, limite):
    """Ultimas `limite` lecturas, de la mas vieja a la mas nueva."""
    with conectar(ruta) as conexion:
        filas = conexion.execute(
            "SELECT * FROM (SELECT * FROM lecturas ORDER BY id DESC LIMIT ?) ORDER BY id",
            (limite,),
        ).fetchall()
    return [_lectura_a_dict(f) for f in filas]


def obtener_eventos(ruta, limite):
    """Ultimos `limite` eventos, del mas nuevo al mas viejo."""
    with conectar(ruta) as conexion:
        filas = conexion.execute(
            "SELECT * FROM eventos ORDER BY id DESC LIMIT ?", (limite,)
        ).fetchall()
    return [dict(f) for f in filas]
