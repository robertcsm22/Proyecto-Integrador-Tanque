"""Simulador del ESP32: envia lecturas falsas al backend.

Sirve para probar el backend y el dashboard sin la maqueta. Imita el
comportamiento del firmware: el tanque se vacia, la valvula abre bajo 20 %,
se llena hasta mas de 90 % y cierra. A veces simula una fuga.

Uso (con el servidor ya corriendo):
    python simulador.py                       # http://127.0.0.1:5000, cada 5 s
    python simulador.py --intervalo 1 --veces 60
"""

import argparse
import random
import time

import requests

UMBRAL_ABRIR, UMBRAL_CERRAR, UMBRAL_BAJO = 20, 90, 50
ALTURA_CM = 20.0


def estado_de(nivel):
    if nivel < UMBRAL_ABRIR:
        return "critico"
    if nivel < UMBRAL_BAJO:
        return "bajo"
    return "normal"


def main():
    parser = argparse.ArgumentParser()
  
    parser.add_argument("--intervalo", type=float, default=5)
    parser.add_argument("--veces", type=int, default=0, help="0 = sin fin")
    args = parser.parse_args()

    nivel, valvula, fuga_restante, enviadas = 70.0, "cerrada", 0, 0
    while args.veces == 0 or enviadas < args.veces:
        # misma histeresis que el firmware
        if nivel < UMBRAL_ABRIR and valvula == "cerrada":
            valvula = "abierta"
        elif nivel > UMBRAL_CERRAR and valvula == "abierta":
            valvula = "cerrada"

        nivel += random.uniform(6, 10) if valvula == "abierta" else -random.uniform(1, 4)
        nivel = max(0.0, min(100.0, nivel))

        if fuga_restante == 0 and random.random() < 0.04:
            fuga_restante = random.randint(3, 6)
        fuga = fuga_restante > 0
        fuga_restante = max(0, fuga_restante - 1)

        estado = estado_de(nivel)
        lectura = {
            "distancia_cm": round(ALTURA_CM * (1 - nivel / 100), 1),
            "nivel": round(nivel),
            "fuga": fuga,
            "valvula": valvula,
            "estado": estado,
            "alarma": estado == "critico" or fuga,
        }
        try:
            r = requests.post(args.url, json=lectura, timeout=3)
            eventos = [e["tipo"] for e in r.json().get("eventos", [])]
            print(f"{r.status_code} nivel={lectura['nivel']:3d}% valvula={valvula:8s} "
                  f"fuga={fuga!s:5s} eventos={eventos}")
        except requests.RequestException as error:
            print(f"sin conexion con el servidor: {error}")
        enviadas += 1
        time.sleep(args.intervalo)


if __name__ == "__main__":
    main()
