# Monitor IoT de tanque de agua

Proyecto Integrador · Paradigmas de Programación · Prof. Eddier López
Entrega: 11 de noviembre de 2026, 1:00 pm

## Problema

En Guanacaste, durante la época seca, muchas casas y edificios dependen de tanques de
almacenamiento. Normalmente nadie sabe cuánta agua queda hasta que se acaba, y las fugas
en la base pasan desapercibidas por días.

## Solución

Un monitor basado en **ESP32** que mide el nivel del tanque con un sensor ultrasónico,
avisa localmente (LCD, LED RGB y buzzer) cuando el agua baja o hay una fuga, controla una
válvula de llenado simulada con un servo y envía cada lectura por WiFi a un servidor
**Flask + SQLite**, que guarda el histórico y lo muestra en un **dashboard web**.

Diagrama de arquitectura y flujo de un dato en [docs/arquitectura.md](docs/arquitectura.md).

## Integrantes y roles

| Integrante | Usuario GitHub | Módulo que lidera |
|------------|----------------|-------------------|
| Anthony Carranza Mejía | [@An7h0ny801](https://github.com/An7h0ny801) | Firmware ESP32, circuito y simulaciones |
| Roberto Carlos Sánchez Matus | [@robertcsm22](https://github.com/robertcsm22) | Documentación, arquitectura y estructura del repositorio |
| Christopher Marchena Telles | [@chxstophermarchena-png](https://github.com/chxstophermarchena-png) | Backend (Flask + SQLite) y diseño de la maqueta |
| Christopher Lopez Melendez | [@Crisis-Wang](https://github.com/Crisis-Wang) | Dashboard |

## Estructura del repositorio

```
Proyecto-Integrador-Tanque/
├── firmware/tanque_iot/   # C++ del ESP32 (Arduino IDE): una clase por componente
├── backend/               # Flask + SQLite (Python) y funciones puras
│   └── tests/             # 19 pruebas automáticas (pytest)
├── dashboard/             # HTML + JS + Chart.js
├── docs/                  # arquitectura, justificación, paradigmas, manual
├── pruebas/               # plan, resultados y evidencias; simulaciones en Wokwi y Tinkercad
└── bitacoras/<nombre>/    # bitácora individual de cada integrante
```

## Paradigmas aplicados

| Paradigma | Dónde | Evidencia | Detalle |
|-----------|-------|-----------|---------|
| Orientado a objetos | Firmware ESP32 | Clases `SensorNivel`, `SensorFuga`, `Valvula`, `Indicador`, `Alarma`, `Comunicacion` | [orientado-objetos.md](docs/paradigmas/orientado-objetos.md) |
| Orientado a eventos | Firmware y backend | Interrupción del pulsador, manejadores `alEvento...()`, detección de eventos en cada lectura recibida | [orientado-eventos.md](docs/paradigmas/orientado-eventos.md) |
| Funcional | Backend | Funciones puras con `map`, `filter`, `reduce`: consumo, promedio diario, alarmas, detección de eventos | [funcional.md](docs/paradigmas/funcional.md) |

## Cómo ejecutar

Guía completa en [docs/manual-ejecucion.md](docs/manual-ejecucion.md). Resumen:

```bash
# 1. Servidor
cd backend
py -m venv .venv
.venv\Scripts\activate
py -m pip install -r requirements.txt
py app.py            # dashboard en http://localhost:5000
```

2. Firmware: copiar `firmware/tanque_iot/secrets.example.h` como `secrets.h`, poner WiFi e
   IP de la laptop y subir `tanque_iot.ino` con Arduino IDE (ver [firmware/README.md](firmware/README.md)).

Sin la maqueta se puede probar con `py simulador.py` o en la
[simulación de Wokwi](https://wokwi.com/projects/478455258708481025).

## Documentación

- [Arquitectura](docs/arquitectura.md) · [Justificación del ESP32](docs/justificacion-dispositivo.md) · [Requerimientos](docs/requerimientos.md)
- [Pruebas y resultados](docs/pruebas.md)
- [Cómo trabajamos con Git](CONTRIBUTING.md)
