#include "Valvula.h"

const int ANGULO_CERRADA = 0;
const int ANGULO_ABIERTA = 90;

Valvula::Valvula(int pin, int umbralAbrir, int umbralCerrar)
  : pin(pin), umbralAbrir(umbralAbrir), umbralCerrar(umbralCerrar) {}

void Valvula::iniciar() {
  servo.setPeriodHertz(50);
  servo.attach(pin, 500, 2400);
  cerrar();
}

CambioValvula Valvula::controlar(int nivel) {
  if (nivel < umbralAbrir && !abierta) {
    abrir();
    return SE_ABRIO;
  }
  if (nivel > umbralCerrar && abierta) {
    cerrar();
    return SE_CERRO;
  }
  return SIN_CAMBIO;  // entre 20 % y 90 % se mantiene como estaba
}

bool Valvula::estaAbierta() const {
  return abierta;
}

void Valvula::abrir() {
  servo.write(ANGULO_ABIERTA);
  abierta = true;
}

void Valvula::cerrar() {
  servo.write(ANGULO_CERRADA);
  abierta = false;
}
