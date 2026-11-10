/*
 * Monitor de tanque de agua - version para simulacion en Tinkercad
 * Placa: Arduino Uno (Tinkercad no tiene ESP32)
 *
 * Paradigmas aplicados:
 *  - Orientado a objetos: cada componente es una clase con su estado encapsulado.
 *  - Orientado a eventos: el loop detecta cambios de estado (eventos) y solo
 *    reacciona cuando ocurren (nivel bajo, nivel lleno, fuga, boton).
 *
 * Conexiones:
 *  Ultrasonico (4 pines) Trig: D7, Echo: D6, VCC: 5V, GND: GND
 *  Sensor de humedad de suelo (simula el sensor de agua / fuga) SIG: A1
 *  LCD 16x2 I2C (MCP23008, direccion 0x20) SDA: SDA, SCL: SCL
 *  LED RGB catodo comun  R: D11, G: D10, B: D9, catodo con 220 ohm a GND
 *  Buzzer (piezo)        positivo: D8
 *  Micro servo (valvula) senal: D3
 *  Pulsador              entre D2 y GND (INPUT_PULLUP)
 *
 * Notas para Tinkercad:
 *  - tone() no compila junto con la libreria Servo, asi que el pitido del
 *    buzzer se genera a mano (ver Alarma::pitido).
 *  - Los estados son constantes int porque el preprocesador de Tinkercad no
 *    acepta un enum como parametro de un metodo.
 *  - Los textos van con F() para guardarlos en flash y ahorrar RAM.
 */

#include <Adafruit_LiquidCrystal.h>
#include <Servo.h>

// ---------------- Configuracion ----------------
const float ALTURA_TANQUE_CM = 100.0;  // distancia del sensor al fondo
const int   UMBRAL_ABRIR     = 20;     // %: abrir valvula por debajo
const int   UMBRAL_CERRAR    = 90;     // %: cerrar valvula por encima
const int   UMBRAL_BAJO      = 50;     // %: verde sobre 50, amarillo 20-50, rojo bajo 20 (RF-03)
const int   UMBRAL_FUGA      = 300;    // valor analogico que se considera fuga
const unsigned long PERIODO_ENVIO_MS = 5000;  // RF-08: cada 5-10 s

// Estados del tanque
const int NORMAL = 0, BAJO = 1, CRITICO = 2;

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
    float d = distanciaCm();
    float nivel = (altura - d) / altura * 100.0;
    return constrain((int)nivel, 0, 100);
  }
};

class SensorFuga {
  int pin, umbral;
public:
  SensorFuga(int p, int u) : pin(p), umbral(u) {}
  int valor()       { return analogRead(pin); }
  bool hayFuga()    { return valor() > umbral; }
};

class Valvula {
  Servo servo;
  int pin;
  bool abierta = false;
public:
  Valvula(int p) : pin(p) {}
  void iniciar()      { servo.attach(pin); cerrar(); }
  void abrir()        { servo.write(90); abierta = true; }
  void cerrar()       { servo.write(0);  abierta = false; }
  bool estaAbierta()  { return abierta; }
};

class Indicador {
  int pinR, pinG, pinB;
  void color(bool r, bool g, bool b) {
    digitalWrite(pinR, r); digitalWrite(pinG, g); digitalWrite(pinB, b);
  }
public:
  Indicador(int r, int g, int b) : pinR(r), pinG(g), pinB(b) {}
  void iniciar() { pinMode(pinR, OUTPUT); pinMode(pinG, OUTPUT); pinMode(pinB, OUTPUT); }
  void mostrar(int e) {
    if (e == NORMAL)      color(0, 1, 0);  // verde
    else if (e == BAJO)   color(1, 1, 0);  // amarillo
    else                  color(1, 0, 0);  // rojo
  }
};

class Alarma {
  int pin;
  bool silenciada = false;
  // Pitido de 1 kHz durante 100 ms generado a mano: en Tinkercad tone() no
  // compila junto con la libreria Servo.
  void pitido() {
    for (int i = 0; i < 100; i++) {
      digitalWrite(pin, HIGH); delayMicroseconds(500);
      digitalWrite(pin, LOW);  delayMicroseconds(500);
    }
  }
public:
  Alarma(int p) : pin(p) {}
  void iniciar()      { pinMode(pin, OUTPUT); }
  void silenciar()    { silenciada = true; }
  void rearmar()      { silenciada = false; }
  void actualizar(bool activa) {
    if (activa && !silenciada) pitido();
  }
};

class Boton {
  int pin;
  bool anterior = HIGH;
public:
  Boton(int p) : pin(p) {}
  void iniciar() { pinMode(pin, INPUT_PULLUP); }
  // Devuelve true solo en el flanco de bajada (evento "presionado")
  bool fuePresionado() {
    bool actual = digitalRead(pin);
    bool evento = (anterior == HIGH && actual == LOW);
    anterior = actual;
    if (evento) delay(30);  // antirrebote simple
    return evento;
  }
};

class Pantalla {
  Adafruit_LiquidCrystal lcd;
public:
  Pantalla() : lcd(0) {}
  void iniciar() {
    lcd.begin(16, 2);
    lcd.setBacklight(1);
    lcd.print(F("Tanque IoT"));
  }
  void mostrar(int nivel, bool valvulaAbierta, bool fuga) {
    lcd.setCursor(0, 0);
    lcd.print(F("Nivel: "));
    lcd.print(nivel);
    lcd.print(F("%      "));
    lcd.setCursor(0, 1);
    if (fuga)                lcd.print(F("!! FUGA !!      "));
    else if (valvulaAbierta) lcd.print(F("Valvula: ABIERTA"));
    else                     lcd.print(F("Valvula: CERRADA"));
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

  int estadoAnterior = NORMAL;
  bool fugaAnterior = false;
  unsigned long ultimoEnvio = 0;

  int calcularEstado(int nivel) {
    if (nivel < UMBRAL_ABRIR) return CRITICO;
    if (nivel < UMBRAL_BAJO)  return BAJO;
    return NORMAL;
  }

  // En el ESP32 esto seria un HTTP POST a Flask; aqui se simula por Serial.
  void enviarLectura(int nivel, bool hayFuga, int e) {
    Serial.print(F("LECTURA nivel="));
    Serial.print(nivel);
    Serial.print(hayFuga ? F(" fuga=si") : F(" fuga=no"));
    Serial.print(valvula.estaAbierta() ? F(" valvula=abierta") : F(" valvula=cerrada"));
    if (e == NORMAL)    Serial.println(F(" estado=normal"));
    else if (e == BAJO) Serial.println(F(" estado=bajo"));
    else                Serial.println(F(" estado=critico"));
  }

  void evento(const __FlashStringHelper *nombre) {
    Serial.print(F("EVENTO: "));
    Serial.println(nombre);
  }

public:
  ControladorTanque(SensorNivel &s, SensorFuga &f, Valvula &v, Indicador &i,
                    Alarma &a, Boton &b, Pantalla &p)
    : sensor(s), fuga(f), valvula(v), indicador(i), alarma(a), boton(b), pantalla(p) {}

  void iniciar() {
    sensor.iniciar(); valvula.iniciar(); indicador.iniciar();
    alarma.iniciar(); boton.iniciar(); pantalla.iniciar();
  }

  void ciclo() {
    int nivel = sensor.nivelPorcentaje();
    bool hayFuga = fuga.hayFuga();
    int estado = calcularEstado(nivel);

    // Evento: boton presionado, se silencia la alarma
    if (boton.fuePresionado()) {
      alarma.silenciar();
      evento(F("boton_silenciar"));
    }

    // Eventos de la valvula con histeresis: abre bajo 20 y cierra sobre 90
    if (nivel < UMBRAL_ABRIR && !valvula.estaAbierta()) {
      valvula.abrir();
      evento(F("valvula_abierta"));
    } else if (nivel > UMBRAL_CERRAR && valvula.estaAbierta()) {
      valvula.cerrar();
      evento(F("valvula_cerrada"));
    }

    // Evento: cambio de estado del nivel
    if (estado != estadoAnterior) {
      if (estado == NORMAL)    evento(F("nivel_normal"));
      else if (estado == BAJO) evento(F("nivel_bajo"));
      else                     evento(F("nivel_critico"));
      if (estado == NORMAL) alarma.rearmar();
      estadoAnterior = estado;
    }

    // Evento: aparece o desaparece la fuga
    if (hayFuga != fugaAnterior) {
      if (hayFuga) { evento(F("fuga_detectada")); alarma.rearmar(); }
      else         evento(F("fuga_resuelta"));
      fugaAnterior = hayFuga;
    }

    indicador.mostrar(hayFuga ? CRITICO : estado);
    alarma.actualizar(estado == CRITICO || hayFuga);
    pantalla.mostrar(nivel, valvula.estaAbierta(), hayFuga);

    if (millis() - ultimoEnvio >= PERIODO_ENVIO_MS) {
      enviarLectura(nivel, hayFuga, estado);
      ultimoEnvio = millis();
    }
  }
};

// ---------------- Objetos globales ----------------
SensorNivel sensorNivel(7, 6, ALTURA_TANQUE_CM);
SensorFuga  sensorFuga(A1, UMBRAL_FUGA);
Valvula     valvula(3);
Indicador   indicador(11, 10, 9);
Alarma      alarma(8);
Boton       boton(2);
Pantalla    pantalla;
ControladorTanque controlador(sensorNivel, sensorFuga, valvula, indicador,
                              alarma, boton, pantalla);

void setup() {
  Serial.begin(9600);
  controlador.iniciar();
}

void loop() {
  controlador.ciclo();
  delay(200);
}
