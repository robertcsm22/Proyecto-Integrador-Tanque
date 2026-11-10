#include "Alarma.h"

const unsigned int FRECUENCIA_HZ = 1000;
const unsigned long ANTIRREBOTE_MS = 200;

// La rutina de interrupcion tiene que estar fuera de la clase y en memoria IRAM
// (requisito del ESP32). Solo levanta una bandera; el trabajo se hace en el loop.
static volatile bool pulsacionPendiente = false;
static volatile unsigned long ultimaPulsacionMs = 0;

static void IRAM_ATTR alPresionarPulsador() {
  unsigned long ahora = millis();
  if (ahora - ultimaPulsacionMs > ANTIRREBOTE_MS) {
    pulsacionPendiente = true;
    ultimaPulsacionMs = ahora;
  }
}

Alarma::Alarma(int pinBuzzer, int pinPulsador)
  : pinBuzzer(pinBuzzer), pinPulsador(pinPulsador) {}

void Alarma::iniciar() {
  pinMode(pinBuzzer, OUTPUT);
  pinMode(pinPulsador, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinPulsador), alPresionarPulsador, FALLING);
}

void Alarma::actualizar(bool condicionDeAlarma) {
  bool debeSonar = condicionDeAlarma && !silenciada;
  if (debeSonar && !sonando) tone(pinBuzzer, FRECUENCIA_HZ);
  if (!debeSonar && sonando) noTone(pinBuzzer);
  sonando = debeSonar;
}

bool Alarma::botonPresionado() {
  if (!pulsacionPendiente) return false;
  pulsacionPendiente = false;
  return true;
}

void Alarma::silenciar() {
  silenciada = true;
  actualizar(false);
}

void Alarma::rearmar() {
  silenciada = false;
}

bool Alarma::estaSonando() const {
  return sonando;
}
