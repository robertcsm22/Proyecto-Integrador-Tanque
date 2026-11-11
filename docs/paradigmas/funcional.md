# Paradigma funcional (backend)

## Dónde se aplica

En [backend/procesamiento.py](../../backend/procesamiento.py). Todo el cálculo sobre los
datos (validar, detectar eventos, estadísticas) está hecho con **funciones puras**:

- reciben datos y devuelven datos nuevos;
- no leen ni escriben la base de datos, no imprimen, no dependen de la hora;
- no modifican sus argumentos (se comprueba en `test_las_funciones_no_modifican_sus_argumentos`);
- con la misma entrada siempre dan la misma salida.

Los efectos secundarios (SQLite, HTTP) quedan separados en `db.py` y `app.py`.

## Evidencia en el código

### `map`, `filter` y `reduce`: consumo estimado

```python
def consumo_estimado_litros(lecturas, capacidad_litros):
    bajadas = map(lambda par: par[0]["nivel"] - par[1]["nivel"], pares_consecutivos(lecturas))
    solo_consumo = filter(lambda diferencia: diferencia > 0, bajadas)
    porcentaje_total = reduce(lambda a, b: a + b, solo_consumo, 0)
    return round(porcentaje_total * capacidad_litros / 100, 2)
```

1. `map`: de cada par de lecturas seguidas calcula cuánto bajó el nivel.
2. `filter`: descarta las subidas (cuando la válvula llena el tanque, no es consumo).
3. `reduce`: suma las bajadas.

### `reduce` para agrupar: promedio diario

```python
def promedio_diario(lecturas):
    por_dia = reduce(
        lambda grupos, l: {**grupos, dia_de(l): grupos.get(dia_de(l), []) + [l["nivel"]]},
        lecturas,
        {},
    )
    return {dia: round(promedio(niveles), 1) for dia, niveles in sorted(por_dia.items())}
```

En cada paso se crea un diccionario **nuevo** (`{**grupos, ...}`) en lugar de modificar uno existente.

### `filter` + `reduce`: conteo de alarmas

```python
def contar_alarmas(eventos):
    return reduce(lambda total, _: total + 1, filter(es_alarma, eventos), 0)
```

### Funciones como datos: reglas de eventos

Las reglas son una lista de tuplas con funciones (`lambda`). Detectar eventos es
filtrar las reglas que se cumplen y transformarlas en eventos:

```python
REGLAS_EVENTOS = [
    (lambda a, n: a["valvula"] != "abierta" and n["valvula"] == "abierta",
     "valvula_abierta", lambda n: f"Valvula abierta con nivel {n['nivel']} %"),
    ...
]

def detectar_eventos(anterior, nueva):
    base = anterior if anterior is not None else LECTURA_INICIAL
    activas = filter(lambda regla: regla[0](base, nueva), REGLAS_EVENTOS)
    return list(map(lambda regla: {"tipo": regla[1], "detalle": regla[2](nueva)}, activas))
```

### Composición: el resumen

`resumen()` solo combina las demás funciones puras; Flask le pasa los datos y devuelve
el resultado como JSON.

## Justificación

- **Predecible y fácil de probar:** las 14 pruebas de `test_procesamiento.py` corren sin
  servidor ni base de datos, en milisegundos.
- **Sin errores por estado compartido:** como nada se modifica, dos peticiones al mismo
  tiempo no se pueden estorbar al calcular.
- **Encaja con el problema:** procesar un histórico es transformar una lista de lecturas
  en un número o en otra lista, que es justo lo que hacen `map`, `filter` y `reduce`.
- **Separación clara:** si mañana se cambia SQLite por otra base, `procesamiento.py`
  no cambia.

## Cómo probarlo

```bash
cd backend
py -m pytest tests/test_procesamiento.py -v
```
