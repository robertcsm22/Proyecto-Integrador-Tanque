// Alarma sonora: buzzer (RF-04) y pulsador para silenciarla (RF-07).
// El pulsador usa una interrupcion de hardware: el ESP32 registra la pulsacion
// en el instante en que ocurre, aunque el loop este ocupado midiendo.
#pragma once
#include <Arduino.h>

class Alarma {
public:
  Alarma(int pinBuzzer, int pinPulsador);
  void iniciar();
  void actualizar(bool condicionDeAlarma);  // suena si hay condicion y no esta silenciada
  bool botonPresionado();                   // true una vez por cada pulsacion
  void silenciar();
  void rearmar();                           // vuelve a sonar ante la proxima alarma
  bool estaSonando() const;

private:
  int pinBuzzer;
  int pinPulsador;
  bool silenciada = false;
  bool sonando = false;
};
