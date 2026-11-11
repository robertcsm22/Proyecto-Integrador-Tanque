# Manual de ejecución (Requerimiento 6)

Orden recomendado: primero el servidor, luego el ESP32.

## 1. Servidor (laptop)

Requisitos: Python 3.10 o superior. En Windows se usa `py` en lugar de `python`; si `python` abre la Microsoft Store, es por eso.

```bash
git clone https://github.com/robertcsm22/Proyecto-Integrador-Tanque.git
cd Proyecto-Integrador-Tanque/backend
py -m venv .venv
.venv\Scripts\activate
py -m pip install -r requirements.txt
py app.py
```

Debe aparecer `Running on http://0.0.0.0:5000`. Abrir http://localhost:5000: el dashboard
dice "Esperando la primera lectura del ESP32".

Anotar la IP de la laptop con `ipconfig` (campo "Dirección IPv4", por ejemplo `192.168.1.50`).
La primera vez Windows pregunta si permite a Python en la red: aceptar en **redes privadas**.

> Prueba rápida sin ESP32: en otra terminal, `py simulador.py --intervalo 1 --veces 60`.

## 2. ESP32

1. Armar el circuito según [firmware/README.md](../firmware/README.md#pines).
2. En Arduino IDE instalar el soporte **esp32** y las librerías **LiquidCrystal I2C** y **ESP32Servo**.
3. Copiar `firmware/tanque_iot/secrets.example.h` como `secrets.h` y poner la red WiFi y
   `SERVER_URL = "http://<IP-de-la-laptop>:5000/lecturas"`.
4. Abrir `tanque_iot.ino`, elegir **ESP32 Dev Module** y el puerto, y **Subir**.
5. Monitor serie a 115200: deben salir `EVENTO: wifi_conectado` y cada 5 s
   `LECTURA {...}` seguido de `servidor respondio 201`.

La laptop y el ESP32 tienen que estar en la **misma red**. Si la red de la universidad
no deja que se vean, usar el hotspot del celular para ambos.

## 3. Uso

- Dashboard: `http://<IP-de-la-laptop>:5000` desde la laptop o el celular.
- Nivel, válvula y fuga se actualizan cada 5 s; el gráfico muestra las últimas 120 lecturas.
- Con la alarma sonando, el pulsador la silencia hasta el próximo evento.

## Problemas comunes

| Síntoma | Causa probable | Solución |
|---------|----------------|----------|
| LCD encendido pero sin texto | Contraste o dirección I2C | Girar el potenciómetro de atrás; probar `LCD_DIRECCION = 0x3F` |
| Nivel siempre 0 % | Sin eco | Revisar Trig/Echo y el divisor; el sensor necesita más de 2 cm de distancia |
| El ESP32 se reinicia al mover el servo | Caída de voltaje | Servo con fuente aparte y GND común; con solo USB, probar otro puerto o un cargador de 2 A |
| `servidor no respondio` | IP, firewall o redes distintas | Revisar `SERVER_URL`, permitir Python en el firewall, misma red |
| Dashboard "No se puede contactar al servidor" | `app.py` no está corriendo | Ejecutar `py app.py` |
| `Connecting...` al subir | La placa no entra en modo carga | Mantener presionado **BOOT** al subir |
