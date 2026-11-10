/*
 * Monitor de tanque de agua - ESP32 (simulacion en Wokwi y montaje real)
 *
 * Paradigmas aplicados:
 *  - Orientado a objetos: cada componente fisico es una clase con su estado
 *    encapsulado (SensorNivel, SensorFuga, Valvula, Indicador, Alarma, Boton,
 *    Pantalla, ClienteServidor) y un ControladorTanque que las coordina.
 *  - Orientado a eventos: el controlador detecta cambios (nivel bajo, nivel
 *    lleno, fuga, boton presionado) y solo reacciona cuando ocurren.
 *
 * Conexiones (iguales al documento del proyecto):
 *  HC-SR04        Trig GPIO 5, Echo GPIO 18 (en el real con divisor de voltaje)
 *  Sensor de agua GPIO 34 (en Wokwi se simula con un potenciometro)
 *  LCD 16x2 I2C   SDA GPIO 21, SCL GPIO 22, direccion 0x27
 *  LED RGB        R GPIO 25, G GPIO 26, B GPIO 27 (220 ohm cada uno)
 *  Buzzer         GPIO 14
 *  Servo          senal GPIO 13 (en el real con fuente aparte y GND comun)
 *  Pulsador       GPIO 4 a GND (INPUT_PULLUP)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// ---------------- Configuracion ----------------
const char* WIFI_SSID     = "Wokwi-GUEST";  // red del simulador; cambiar en el real
const char* WIFI_PASSWORD = "";
// URL del backend Flask (POST /lecturas). Mientras siga con "TU-SERVIDOR"
// el dispositivo no envia nada, pero todo lo local sigue funcionando.
const char* SERVIDOR_URL  = "http://TU-SERVIDOR:5000/lecturas";

const float ALTURA_TANQUE_CM = 100.0;  // distancia del sensor al fondo
const int   UMBRAL_ABRIR     = 20;     // %: abrir valvula por debajo (RF-06)
const int   UMBRAL_CERRAR    = 90;     // %: cerrar valvula por encima (RF-06)
const int   UMBRAL_BAJO      = 50;     // %: verde sobre 50, amarillo 20-50, rojo bajo 20 (RF-03)
const int   UMBRAL_FUGA      = 1500;   // lectura analogica (0-4095) que se considera fuga
const unsigned long PERIODO_ENVIO_MS = 5000;  // RF-08: cada 5-10 s

enum Estado { NORMAL, BAJO, CRITICO };

const char* nombreEstado(Estado e) {
  if (e == NORMAL) return "normal";
  if (e == BAJO)   return "bajo";
  return "critico";
}

// ---------------- Clases ----------------

class SensorNivel {
  int pinTrig, pinEcho;
  float altura;
public:
  SensorNivel(int trig, int echo, float alturaCm)
    : pinTrig(trig), pinEcho(echo), altura(alturaCm) {}

  void iniciar() {
    pinMode(pinTrig, OUTPUT);
    pinMode(pinEcho, INPUT);
  }

  float distanciaCm() {
    digitalWrite(pinTrig, LOW);
    delayMicroseconds(2);
    digitalWrite(pinTrig, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinTrig, LOW);
    long duracion = pulseIn(pinEcho, HIGH, 30000);
    return duracion * 0.0343 / 2.0;
  }

  // nivel % = (altura - distancia) / altura * 100
  int nivelPorcentaje() {
    float nivel = (altura - distanciaCm()) / altura * 100.0;
    return constrain((int)nivel, 0, 100);
  }
};

class SensorFuga {
  int pin, umbral;
public:
  SensorFuga(int p, int u) : pin(p), umbral(u) {}
  int valor()    { return analogRead(pin); }
  bool hayFuga() { return valor() > umbral; }
};

class Valvula {
  Servo servo;
  int pin;
  bool abierta = false;
public:
  Valvula(int p) : pin(p) {}
  void iniciar()     { servo.attach(pin); cerrar(); }
  void abrir()       { servo.write(90); abierta = true; }
  void cerrar()      { servo.write(0);  abierta = false; }
  bool estaAbierta() { return abierta; }
};

class Indicador {
  int pinR, pinG, pinB;
  void color(bool r, bool g, bool b) {
    digitalWrite(pinR, r); digitalWrite(pinG, g); digitalWrite(pinB, b);
  }
public:
  Indicador(int r, int g, int b) : pinR(r), pinG(g), pinB(b) {}
  void iniciar() { pinMode(pinR, OUTPUT); pinMode(pinG, OUTPUT); pinMode(pinB, OUTPUT); }
  void mostrar(Estado e) {
    if (e == NORMAL)    color(0, 1, 0);  // verde
    else if (e == BAJO) color(1, 1, 0);  // amarillo
    else                color(1, 0, 0);  // rojo
  }
};

class Alarma {
  int pin;
  bool silenciada = false;
public:
  Alarma(int p) : pin(p) {}
  void iniciar()   { pinMode(pin, OUTPUT); }
  void silenciar() { silenciada = true; noTone(pin); }
  void rearmar()   { silenciada = false; }
  void actualizar(bool activa) {
    if (activa && !silenciada) tone(pin, 1000);
    else noTone(pin);
  }
};

// El boton usa una interrupcion de hardware: el ESP32 avisa en el instante
// en que se presiona (flanco de bajada), aunque el loop este ocupado.
// La rutina de interrupcion va fuera de la clase porque el ESP32 la necesita
// en memoria IRAM.
volatile bool botonPresionado = false;
volatile unsigned long botonUltimaPulsacion = 0;

void IRAM_ATTR alPresionarBoton() {
  unsigned long ahora = millis();
  if (ahora - botonUltimaPulsacion > 200) {  // antirrebote
    botonPresionado = true;
    botonUltimaPulsacion = ahora;
  }
}

class Boton {
  int pin;
public:
  Boton(int p) : pin(p) {}
  void iniciar() {
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), alPresionarBoton, FALLING);
  }
  // Devuelve true una sola vez por cada pulsacion (evento "presionado")
  bool fuePresionado() {
    if (!botonPresionado) return false;
    botonPresionado = false;
    return true;
  }
};

class Pantalla {
  LiquidCrystal_I2C lcd;
public:
  Pantalla() : lcd(0x27, 16, 2) {}
  void iniciar() {
    lcd.init();
    lcd.backlight();
    lcd.print("Tanque IoT");
  }
  void mostrar(int nivel, bool valvulaAbierta, bool fuga, bool wifi) {
    lcd.setCursor(0, 0);
    lcd.printf("Nivel: %3d%%  %s", nivel, wifi ? "W" : " ");
    lcd.setCursor(0, 1);
    if (fuga)                lcd.print("!! FUGA !!      ");
    else if (valvulaAbierta) lcd.print("Valvula: ABIERTA");
    else                     lcd.print("Valvula: CERRADA");
  }
};

// Envia las lecturas al backend Flask. Si no hay WiFi no bloquea nada (RNF-02).
class ClienteServidor {
  const char* ssid;
  const char* clave;
  const char* url;
public:
  ClienteServidor(const char* s, const char* c, const char* u) : ssid(s), clave(c), url(u) {}
  void iniciar()   { WiFi.begin(ssid, clave); }
  bool conectado() { return WiFi.status() == WL_CONNECTED; }
  bool configurado() { return strstr(url, "TU-SERVIDOR") == nullptr; }

  void enviar(int nivel, bool fuga, bool valvulaAbierta, Estado e) {
    char json[128];
    snprintf(json, sizeof(json),
             "{\"nivel\":%d,\"fuga\":%s,\"valvula\":\"%s\",\"estado\":\"%s\"}",
             nivel, fuga ? "true" : "false",
             valvulaAbierta ? "abierta" : "cerrada", nombreEstado(e));
    Serial.print("LECTURA ");
    Serial.println(json);

    if (!conectado() || !configurado()) return;
    HTTPClient http;
    http.setTimeout(2000);
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int codigo = http.POST(json);
    Serial.printf("  -> servidor respondio %d\n", codigo);
    http.end();
  }
};

// ---------------- Controlador (reacciona a eventos) ----------------

class ControladorTanque {
  SensorNivel &sensor;
  SensorFuga  &fuga;
  Valvula     &valvula;
  Indicador   &indicador;
  Alarma      &alarma;
  Boton       &boton;
  Pantalla    &pantalla;
  ClienteServidor &servidor;

  Estado estadoAnterior = NORMAL;
  bool fugaAnterior = false;
  bool wifiAnterior = false;
  unsigned long ultimoEnvio = 0;

  Estado calcularEstado(int nivel) {
    if (nivel < UMBRAL_ABRIR) return CRITICO;
    if (nivel < UMBRAL_BAJO)  return BAJO;
    return NORMAL;
  }

  void evento(const char *nombre) {
    Serial.print("EVENTO: ");
    Serial.println(nombre);
  }

public:
  ControladorTanque(SensorNivel &s, SensorFuga &f, Valvula &v, Indicador &i,
                    Alarma &a, Boton &b, Pantalla &p, ClienteServidor &c)
    : sensor(s), fuga(f), valvula(v), indicador(i), alarma(a), boton(b),
      pantalla(p), servidor(c) {}

  void iniciar() {
    sensor.iniciar(); valvula.iniciar(); indicador.iniciar();
    alarma.iniciar(); boton.iniciar(); pantalla.iniciar();
    servidor.iniciar();
  }

  void ciclo() {
    int nivel = sensor.nivelPorcentaje();
    bool hayFuga = fuga.hayFuga();
    Estado estado = calcularEstado(nivel);

    // Evento: boton presionado, se silencia la alarma (RF-07)
    if (boton.fuePresionado()) {
      alarma.silenciar();
      evento("boton_silenciar");
    }

    // Eventos de la valvula con histeresis: abre bajo 20 % y cierra sobre 90 %
    if (nivel < UMBRAL_ABRIR && !valvula.estaAbierta()) {
      valvula.abrir();
      evento("valvula_abierta");
    } else if (nivel > UMBRAL_CERRAR && valvula.estaAbierta()) {
      valvula.cerrar();
      evento("valvula_cerrada");
    }

    // Evento: cambio de estado del nivel
    if (estado != estadoAnterior) {
      if (estado == NORMAL)    evento("nivel_normal");
      else if (estado == BAJO) evento("nivel_bajo");
      else                     evento("nivel_critico");
      if (estado == NORMAL) alarma.rearmar();
      estadoAnterior = estado;
    }

    // Evento: aparece o desaparece la fuga
    if (hayFuga != fugaAnterior) {
      if (hayFuga) { evento("fuga_detectada"); alarma.rearmar(); }
      else         evento("fuga_resuelta");
      fugaAnterior = hayFuga;
    }

    // Evento: se conecta o se pierde el WiFi
    bool wifi = servidor.conectado();
    if (wifi != wifiAnterior) {
      evento(wifi ? "wifi_conectado" : "wifi_desconectado");
      wifiAnterior = wifi;
    }

    indicador.mostrar(hayFuga ? CRITICO : estado);
    alarma.actualizar(estado == CRITICO || hayFuga);
    pantalla.mostrar(nivel, valvula.estaAbierta(), hayFuga, wifi);

    if (millis() - ultimoEnvio >= PERIODO_ENVIO_MS) {
      servidor.enviar(nivel, hayFuga, valvula.estaAbierta(), estado);
      ultimoEnvio = millis();
    }
  }
};

// ---------------- Objetos globales ----------------
SensorNivel     sensorNivel(5, 18, ALTURA_TANQUE_CM);
SensorFuga      sensorFuga(34, UMBRAL_FUGA);
Valvula         valvula(13);
Indicador       indicador(25, 26, 27);
Alarma          alarma(14);
Boton           boton(4);
Pantalla        pantalla;
ClienteServidor servidor(WIFI_SSID, WIFI_PASSWORD, SERVIDOR_URL);
ControladorTanque controlador(sensorNivel, sensorFuga, valvula, indicador,
                              alarma, boton, pantalla, servidor);

void setup() {
  Serial.begin(115200);
  controlador.iniciar();
}

void loop() {
  controlador.ciclo();
  delay(200);
}
