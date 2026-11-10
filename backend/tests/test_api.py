"""Pruebas de las rutas de Flask con una base de datos temporal."""

import os
import sys

import pytest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from app import crear_app


@pytest.fixture
def cliente(tmp_path):
    app = crear_app(str(tmp_path / "prueba.db"))
    return app.test_client()


def enviar(cliente, nivel, valvula="cerrada", estado="normal", fuga=False):
    return cliente.post("/lecturas", json={
        "distancia_cm": 10.0, "nivel": nivel, "fuga": fuga,
        "valvula": valvula, "estado": estado, "alarma": False,
    })


def test_post_guarda_y_get_devuelve_en_orden(cliente):
    assert enviar(cliente, 80).status_code == 201
    assert enviar(cliente, 70).status_code == 201
    lecturas = cliente.get("/lecturas").get_json()
    assert [l["nivel"] for l in lecturas] == [80, 70]
    assert lecturas[0]["fuga"] is False


def test_post_invalido_responde_400(cliente):
    r = cliente.post("/lecturas", json={"nivel": 500})
    assert r.status_code == 400
    assert r.get_json()["errores"]


def test_cambio_de_valvula_queda_registrado_como_evento(cliente):
    enviar(cliente, 25, estado="bajo")
    r = enviar(cliente, 15, valvula="abierta", estado="critico")
    assert [e["tipo"] for e in r.get_json()["eventos"]] == ["valvula_abierta", "nivel_critico"]
    eventos = cliente.get("/eventos").get_json()
    assert {e["tipo"] for e in eventos} == {"nivel_bajo", "valvula_abierta", "nivel_critico"}


def test_resumen(cliente):
    enviar(cliente, 60)
    enviar(cliente, 50)
    r = cliente.get("/resumen").get_json()
    assert r["total_lecturas"] == 2 and r["nivel_actual"] == 50
    assert r["consumo_estimado_litros"] == 0.4  # 10 % de 4 L


def test_dashboard_se_sirve_en_la_raiz(cliente):
    r = cliente.get("/")
    assert r.status_code == 200 and b"<html" in r.data.lower()
