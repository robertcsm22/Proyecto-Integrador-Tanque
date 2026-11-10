#include "SensorNivel.h"

SensorNivel::SensorNivel(int pinTrig, int pinEcho, float alturaTanqueCm, float distanciaLlenoCm)
  : pinTrig(pinTrig), pinEcho(pinEcho), alturaTanqueCm(alturaTanqueCm),
    distanciaLlenoCm(distanciaLlenoCm) {}

void SensorNivel::iniciar() {
  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);
  digitalWrite(pinTrig, LOW);
}

float SensorNivel::medirUnaVez() {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);
  long duracionUs = pulseIn(pinEcho, HIGH, 30000);  // 30 ms = unos 5 m
  if (duracionUs == 0) return -1;                    // sin eco
  return duracionUs * 0.0343 / 2.0;                  // velocidad del sonido, ida y vuelta
}

// La mediana de 3 mediciones descarta un valor suelto erroneo
// (por ejemplo, cuando el agua se mueve). Ayuda a cumplir RNF-01 (error < 1 cm).
float SensorNivel::distanciaCm() {
  float a = medirUnaVez(); delay(10);
  float b = medirUnaVez(); delay(10);
  float c = medirUnaVez();
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}

// nivel % = (altura - distancia) / altura * 100
int SensorNivel::porcentaje(float distanciaCm) const {
  if (distanciaCm < 0) return 0;  // sin eco: se toma como tanque vacio
  // El sensor no mide bien a menos de 2 cm, por eso el 100 % se fija a distanciaLlenoCm.
  float nivel = (alturaTanqueCm - distanciaCm) / (alturaTanqueCm - distanciaLlenoCm) * 100.0;
  return constrain((int)(nivel + 0.5), 0, 100);
}
