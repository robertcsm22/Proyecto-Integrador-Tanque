#include "SensorFuga.h"

SensorFuga::SensorFuga(int pin, int umbral, int margen, bool fugaConValorBajo)
  : pin(pin), umbral(umbral), margen(margen), fugaConValorBajo(fugaConValorBajo) {}

void SensorFuga::iniciar() {
  pinMode(pin, INPUT);
}

int SensorFuga::valor() const {
  return analogRead(pin);
}

bool SensorFuga::hayFuga() {
  int lectura = valor();
  // Sin fuga se compara con el umbral; con fuga, con el umbral corrido por el margen.
  int limite = fuga ? umbral + (fugaConValorBajo ? margen : -margen) : umbral;
  fuga = fugaConValorBajo ? lectura < limite : lectura > limite;
  return fuga;
}
