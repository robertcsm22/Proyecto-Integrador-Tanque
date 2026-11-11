# Paradigma orientado a eventos (firmware y backend)

Un sistema IoT no sigue un flujo lineal de principio a fin: **reacciona a lo que pasa
en el entorno**. Por eso el control se organizó alrededor de eventos y de funciones
que los atienden.

## Eventos del sistema

| Evento | Quién lo detecta | Reacción |
|--------|------------------|----------|
| `alarma_silenciada` (botón) | Interrupción de hardware del ESP32 | Se apaga el buzzer |
| `valvula_abierta` / `valvula_cerrada` | `Valvula::controlar()` | Se mueve el servo y se registra |
| `nivel_critico` / `nivel_bajo` / `nivel_normal` | `tanque_iot.ino` al comparar con el estado anterior | LED, buzzer; con `nivel_normal` la alarma se rearma |
| `fuga_detectada` / `fuga_resuelta` | `tanque_iot.ino` | LCD muestra `!! FUGA !!`, suena el buzzer |
| `wifi_conectado` / `wifi_desconectado` | `tanque_iot.ino` | Aparece o desaparece la `W` del LCD |
| Lectura periódica (cada 5 s) | Temporizador con `millis()` | Se envía el JSON al servidor |
| **Lectura recibida** | Flask (`POST /lecturas`) | Se compara con la anterior y se guardan los eventos |

## Evidencia en el código

### 1. Interrupción de hardware (evento físico real)

El pulsador no se consulta en cada vuelta: el ESP32 **avisa** cuando ocurre el flanco
de bajada, aunque el programa esté ocupado midiendo distancia. La rutina solo levanta
una bandera; el trabajo se hace después.

```cpp
// Alarma.cpp
static void IRAM_ATTR alPresionarPulsador() {
  unsigned long ahora = millis();
  if (ahora - ultimaPulsacionMs > ANTIRREBOTE_MS) {
    pulsacionPendiente = true;
    ultimaPulsacionMs = ahora;
  }
}
...
attachInterrupt(digitalPinToInterrupt(pinPulsador), alPresionarPulsador, FALLING);
```

> En la simulación, con una lectura por consulta (*polling*) un clic rápido se perdía porque
> duraba menos que una vuelta del `loop`. Con la interrupción no se pierde ninguno.

### 2. Detección de cambios y manejadores

`loop()` no repite acciones en cada vuelta: compara con el estado anterior y solo
llama al manejador cuando algo **cambió**.

```cpp
// tanque_iot.ino
if (alarma.botonPresionado()) alEventoBoton();

CambioValvula cambio = valvula.controlar(lectura.nivel);
if (cambio != SIN_CAMBIO) alEventoValvula(cambio);

if (lectura.estado != estadoAnterior) {
  alEventoCambioNivel(lectura.estado);
  estadoAnterior = lectura.estado;
}
if (lectura.fuga != fugaAnterior) {
  alEventoFuga(lectura.fuga);
  fugaAnterior = lectura.fuga;
}
```

```cpp
void alEventoFuga(bool hayFuga) {
  registrarEvento(hayFuga ? "fuga_detectada" : "fuga_resuelta");
  if (hayFuga) alarma.rearmar();  // una fuga nueva siempre suena
}
```

### 3. Eventos temporizados sin bloquear

Ni la lectura ni el envío usan `delay()` largos: se comparan tiempos con `millis()`,
así el botón y las alarmas responden mientras tanto.

```cpp
if (ahora - ultimoEnvioMs >= PERIODO_ENVIO_MS) {
  alEventoEnvio(lectura);
  ultimoEnvioMs = ahora;
}
```

### 4. El servidor reacciona a cada lectura recibida

```python
# backend/app.py
@app.post("/lecturas")
def recibir_lectura():
    ...
    anterior = db.ultima_lectura(ruta_db)
    eventos = procesamiento.detectar_eventos(anterior, nueva)
    guardada = db.guardar_lectura(ruta_db, nueva, eventos)
```

Las reglas de qué cambio produce qué evento están en una tabla (`REGLAS_EVENTOS` en
`procesamiento.py`): agregar un evento nuevo es agregar una línea.

## Justificación

- **Refleja el problema real:** al usuario le interesan los *cambios* (se vació, hay
  fuga), no una lectura repetida cada 200 ms.
- **Respuesta inmediata:** la interrupción atiende el botón al instante.
- **Menos tráfico y registros más útiles:** la tabla `eventos` guarda solo lo que pasó,
  y sirve directo para la lista de alarmas del dashboard.
- **Funciona sin red (RNF-02):** los eventos locales no dependen del WiFi; la
  comunicación es solo un evento más.
