"""Procesamiento de lecturas con programacion funcional.

Todas las funciones de este modulo son puras: reciben datos y devuelven datos
nuevos, sin leer la base de datos, sin modificar sus argumentos y sin efectos
secundarios. Por eso se pueden probar solas (ver tests/test_procesamiento.py).

Se usan map, filter y reduce en lugar de ciclos con variables acumuladoras.
"""

from functools import reduce

ESTADOS_VALIDOS = ("normal", "bajo", "critico")
VALVULA_VALIDA = ("abierta", "cerrada")


#  validacion

def validar_lectura(datos):
    """Devuelve la lista de errores de una lectura (vacia si es valida)."""
    if not isinstance(datos, dict):
        return ["el cuerpo debe ser un objeto JSON"]

    reglas = [
        ("nivel", lambda v: isinstance(v, (int, float)) and 0 <= v <= 100,
         "nivel debe ser un numero entre 0 y 100"),
        ("fuga", lambda v: isinstance(v, bool), "fuga debe ser true o false"),
        ("valvula", lambda v: v in VALVULA_VALIDA, "valvula debe ser 'abierta' o 'cerrada'"),
        ("estado", lambda v: v in ESTADOS_VALIDOS, "estado debe ser normal, bajo o critico"),
    ]
    fallidas = filter(lambda regla: regla[0] not in datos or not regla[1](datos[regla[0]]), reglas)
    return list(map(lambda regla: regla[2], fallidas))


def normalizar_lectura(datos):
    """Devuelve una copia de la lectura con solo los campos que se guardan."""
    return {
        "distancia_cm": float(datos.get("distancia_cm", 0) or 0),
        "nivel": int(round(datos["nivel"])),
        "fuga": bool(datos["fuga"]),
        "valvula": datos["valvula"],
        "estado": datos["estado"],
        "alarma": bool(datos.get("alarma", False)),
    }


#  eventos

# Cada regla compara la lectura anterior con la nueva y dice que evento genera

REGLAS_EVENTOS = [
    (lambda a, n: a["valvula"] != "abierta" and n["valvula"] == "abierta",
     "valvula_abierta", lambda n: f"Valvula abierta con nivel {n['nivel']} %"),
    (lambda a, n: a["valvula"] == "abierta" and n["valvula"] != "abierta",
     "valvula_cerrada", lambda n: f"Valvula cerrada con nivel {n['nivel']} %"),
    (lambda a, n: a["estado"] != "critico" and n["estado"] == "critico",
     "nivel_critico", lambda n: f"Nivel critico: {n['nivel']} %"),
    (lambda a, n: a["estado"] != "bajo" and n["estado"] == "bajo",
     "nivel_bajo", lambda n: f"Nivel bajo: {n['nivel']} %"),
    (lambda a, n: a["estado"] != "normal" and n["estado"] == "normal",
     "nivel_normal", lambda n: f"Nivel recuperado: {n['nivel']} %"),
    (lambda a, n: not a["fuga"] and n["fuga"],
     "fuga_detectada", lambda n: "Se detecto agua en la base del tanque"),
    (lambda a, n: a["fuga"] and not n["fuga"],
     "fuga_resuelta", lambda n: "La base del tanque esta seca"),
]

# Estado "neutro" para la primera lectura que llega
LECTURA_INICIAL = {"valvula": "cerrada", "estado": "normal", "fuga": False}

TIPOS_ALARMA = ("nivel_critico", "fuga_detectada")


def detectar_eventos(anterior, nueva):
    """Lista de eventos que produce pasar de la lectura anterior a la nueva."""
    base = anterior if anterior is not None else LECTURA_INICIAL
    activas = filter(lambda regla: regla[0](base, nueva), REGLAS_EVENTOS)
    return list(map(lambda regla: {"tipo": regla[1], "detalle": regla[2](nueva)}, activas))


def es_alarma(evento):
    return evento["tipo"] in TIPOS_ALARMA


def contar_alarmas(eventos):
    """Cantidad de alarmas (nivel critico y fugas) en una lista de eventos."""
    return reduce(lambda total, _: total + 1, filter(es_alarma, eventos), 0)


def contar_por_tipo(eventos):
    """Diccionario {tipo de evento: cantidad}."""
    return reduce(
        lambda conteo, e: {**conteo, e["tipo"]: conteo.get(e["tipo"], 0) + 1},
        eventos,
        {},
    )


# --------------------------------------------------------------- estadisticas

def promedio(valores):
    """Promedio de una lista de numeros (0 si esta vacia)."""
    valores = list(valores)
    if not valores:
        return 0
    return reduce(lambda a, b: a + b, valores) / len(valores)


def dia_de(lectura):
    """'2026-11-09T14:30:00' -> '2026-11-09'"""
    return lectura["fecha"][:10]


def promedio_diario(lecturas):
    """Promedio del nivel por dia: {'2026-11-09': 63.5, ...}."""
    por_dia = reduce(
        lambda grupos, l: {**grupos, dia_de(l): grupos.get(dia_de(l), []) + [l["nivel"]]},
        lecturas,
        {},
    )
    return {dia: round(promedio(niveles), 1) for dia, niveles in sorted(por_dia.items())}


def pares_consecutivos(lecturas):
    """[a, b, c] -> [(a, b), (b, c)]"""
    return list(zip(lecturas, lecturas[1:]))


def consumo_estimado_litros(lecturas, capacidad_litros):
    """Litros consumidos: suma de las bajadas de nivel entre lecturas seguidas.

    Las subidas (cuando la valvula llena el tanque) no cuentan como consumo.
    """
    bajadas = map(lambda par: par[0]["nivel"] - par[1]["nivel"], pares_consecutivos(lecturas))
    solo_consumo = filter(lambda diferencia: diferencia > 0, bajadas)
    porcentaje_total = reduce(lambda a, b: a + b, solo_consumo, 0)
    return round(porcentaje_total * capacidad_litros / 100, 2)


def resumen(lecturas, eventos, capacidad_litros):
    """Resumen para el dashboard, calculado solo a partir de los datos."""
    niveles = list(map(lambda l: l["nivel"], lecturas))
    return {
        "total_lecturas": len(lecturas),
        "nivel_actual": niveles[-1] if niveles else None,
        "nivel_promedio": round(promedio(niveles), 1),
        "nivel_minimo": min(niveles) if niveles else None,
        "nivel_maximo": max(niveles) if niveles else None,
        "promedio_diario": promedio_diario(lecturas),
        "total_alarmas": contar_alarmas(eventos),
        "eventos_por_tipo": contar_por_tipo(eventos),
        "consumo_estimado_litros": consumo_estimado_litros(lecturas, capacidad_litros),
    }
