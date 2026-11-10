"""Servidor Flask del monitor de tanque.

Rutas:
  POST /lecturas   el ESP32 envia una lectura en JSON (RF-08, RF-09)
  GET  /lecturas   historico de lecturas (?limite=100)
  GET  /eventos    alarmas y cambios de valvula (?limite=50)
  GET  /resumen    estadisticas calculadas con funciones puras
  GET  /           dashboard web (RF-10)

Orientado a eventos: cada lectura que llega es un evento; el servidor la
compara con la anterior y registra los eventos que se produjeron.
"""

import os

from flask import Flask, jsonify, request, send_from_directory

import db
import procesamiento

CARPETA = os.path.dirname(os.path.abspath(__file__))
CARPETA_DASHBOARD = os.path.join(CARPETA, "..", "dashboard")
RUTA_DB = os.environ.get("TANQUE_DB", os.path.join(CARPETA, "tanque.db"))
CAPACIDAD_LITROS = float(os.environ.get("TANQUE_LITROS", "4"))  # recipiente de la maqueta


def crear_app(ruta_db=RUTA_DB):
    app = Flask(__name__, static_folder=None)
    db.crear_tablas(ruta_db)

    def limite(por_defecto, maximo=2000):
        try:
            return max(1, min(int(request.args.get("limite", por_defecto)), maximo))
        except ValueError:
            return por_defecto

    @app.post("/lecturas")
    def recibir_lectura():
        datos = request.get_json(silent=True)
        errores = procesamiento.validar_lectura(datos)
        if errores:
            return jsonify({"errores": errores}), 400

        nueva = procesamiento.normalizar_lectura(datos)
        anterior = db.ultima_lectura(ruta_db)
        eventos = procesamiento.detectar_eventos(anterior, nueva)
        guardada = db.guardar_lectura(ruta_db, nueva, eventos)

        for e in eventos:
            print(f"[evento] {e['tipo']}: {e['detalle']}")
        return jsonify({"lectura": guardada, "eventos": eventos}), 201

    @app.get("/lecturas")
    def listar_lecturas():
        return jsonify(db.obtener_lecturas(ruta_db, limite(100)))

    @app.get("/eventos")
    def listar_eventos():
        return jsonify(db.obtener_eventos(ruta_db, limite(50)))

    @app.get("/resumen")
    def ver_resumen():
        lecturas = db.obtener_lecturas(ruta_db, 2000)
        eventos = db.obtener_eventos(ruta_db, 2000)
        return jsonify(procesamiento.resumen(lecturas, eventos, CAPACIDAD_LITROS))

    @app.get("/")
    def dashboard():
        return send_from_directory(CARPETA_DASHBOARD, "index.html")

    @app.get("/<path:archivo>")
    def archivos_dashboard(archivo):
        return send_from_directory(CARPETA_DASHBOARD, archivo)

    return app


if __name__ == "__main__":
    # host 0.0.0.0 para que el ESP32 pueda llegar desde la red local
    crear_app().run(host="0.0.0.0", port=5000, debug=False)
