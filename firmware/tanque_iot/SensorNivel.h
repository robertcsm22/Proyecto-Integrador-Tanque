// Sensor ultrasonico HC-SR04: mide la distancia al agua y la convierte en
// porcentaje de llenado (RF-01).
#pragma once
#include <Arduino.h>

class SensorNivel {
public:
  SensorNivel(int pinTrig, int pinEcho, float alturaTanqueCm, float distanciaLlenoCm);
  void iniciar();
  float distanciaCm();          // mediana de 3 mediciones, en cm
  int porcentaje(float distanciaCm) const;

private:
  int pinTrig;
  int pinEcho;
  float alturaTanqueCm;   // sensor -> fondo (0 %)
  float distanciaLlenoCm; // sensor -> agua con el tanque lleno (100 %)
  float medirUnaVez();
};
