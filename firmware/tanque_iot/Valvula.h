// Valvula de llenado simulada con un microservo (RF-06).
// Aplica histeresis: abre por debajo de un umbral y cierra por encima de otro,
// asi no abre y cierra sin parar cuando el nivel ronda un solo valor.
#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

enum CambioValvula { SIN_CAMBIO, SE_ABRIO, SE_CERRO };

class Valvula {
public:
  Valvula(int pin, int umbralAbrir, int umbralCerrar);
  void iniciar();
  CambioValvula controlar(int nivel);  // decide segun el nivel y avisa si cambio
  bool estaAbierta() const;

private:
  Servo servo;
  int pin;
  int umbralAbrir;
  int umbralCerrar;
  bool abierta = false;
  void abrir();
  void cerrar();
};
