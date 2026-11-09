"""Pruebas de las funciones puras de procesamiento.py.

Como las funciones son puras, se prueban sin servidor ni base de datos:
misma entrada, misma salida.

Ejecutar desde la carpeta backend:  python -m pytest -v
"""

import copy
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import procesamiento as p


def lectura(nivel, valvula="cerrada", estado="normal", fuga=False, fecha="2026-11-09T10:00:00"):
    return {"nivel": nivel, "valvula": valvula, "estado": estado, "fuga": fuga, "fecha": fecha}


# validacion

def test_lectura_valida_no_tiene_errores():
    assert p.validar_lectura({"nivel": 50, "fuga": False, "valvula": "cerrada", "estado": "normal"}) == []


def test_lectura_invalida_lista_cada_error():
    errores = p.validar_lectura({"nivel": 150, "fuga": "no", "valvula": "medio"})
    assert len(errores) == 4  # nivel, fuga, valvula y estado faltante


def test_cuerpo_que_no_es_objeto():
    assert p.validar_lectura(None) == ["el cuerpo debe ser un objeto JSON"]


#  eventos

def test_primera_lectura_normal_no_genera_eventos():
    assert p.detectar_eventos(None, lectura(80)) == []


def test_abrir_valvula_y_nivel_critico():
    tipos = [e["tipo"] for e in p.detectar_eventos(
        lectura(25, estado="bajo"), lectura(15, valvula="abierta", estado="critico"))]
    assert tipos == ["valvula_abierta", "nivel_critico"]


def test_fuga_detectada_y_resuelta():
    seco, mojado = lectura(60), lectura(60, fuga=True)
    assert [e["tipo"] for e in p.detectar_eventos(seco, mojado)] == ["fuga_detectada"]
    assert [e["tipo"] for e in p.detectar_eventos(mojado, seco)] == ["fuga_resuelta"]


def test_sin_cambios_no_hay_eventos():
    assert p.detectar_eventos(lectura(60), lectura(58)) == []


def test_contar_alarmas_solo_cuenta_critico_y_fuga():
    eventos = [{"tipo": "nivel_critico"}, {"tipo": "valvula_abierta"},
               {"tipo": "fuga_detectada"}, {"tipo": "nivel_normal"}]
    assert p.contar_alarmas(eventos) == 2


def test_contar_por_tipo():
    eventos = [{"tipo": "a"}, {"tipo": "b"}, {"tipo": "a"}]
    assert p.contar_por_tipo(eventos) == {"a": 2, "b": 1}


# estadisticas

def test_promedio():
    assert p.promedio([10, 20, 30]) == 20
    assert p.promedio([]) == 0


def test_promedio_diario_agrupa_por_fecha():
    lecturas = [lectura(40, fecha="2026-11-08T09:00:00"), lectura(60, fecha="2026-11-08T18:00:00"),
                lectura(90, fecha="2026-11-09T09:00:00")]
    assert p.promedio_diario(lecturas) == {"2026-11-08": 50.0, "2026-11-09": 90.0}


def test_consumo_ignora_las_subidas_por_llenado():
    
    lecturas = [lectura(60), lectura(50), lectura(100), lectura(80)]
    assert p.consumo_estimado_litros(lecturas, 4) == 1.2


def test_resumen_con_lista_vacia():
    r = p.resumen([], [], 4)
    assert r["total_lecturas"] == 0 and r["nivel_actual"] is None and r["consumo_estimado_litros"] == 0


def test_las_funciones_no_modifican_sus_argumentos():
    lecturas = [lectura(60), lectura(40, estado="bajo")]
    eventos = [{"tipo": "nivel_bajo"}]
    copia_l, copia_e = copy.deepcopy(lecturas), copy.deepcopy(eventos)
    p.resumen(lecturas, eventos, 4)
    p.detectar_eventos(lecturas[0], lecturas[1])
    assert lecturas == copia_l and eventos == copia_e
