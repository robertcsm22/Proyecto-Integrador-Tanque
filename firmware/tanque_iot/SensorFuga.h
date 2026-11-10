// Sensor de agua en la base del tanque: detecta fuga o desborde (RF-05).
// Funciona con dos tipos de sensor: los que suben el valor al mojarse y los que
// lo bajan (como el modulo de humedad de suelo con LM393).
// Tiene margen (histeresis): la fuga empieza al cruzar el umbral y solo se da por
// resuelta cuando la lectura se aleja "margen" puntos. Asi no parpadea cerca del limite.
#pragma once
#include <Arduino.h>

class SensorFuga {
public:
  SensorFuga(int pin, int umbral, int margen, bool fugaConValorBajo);
  void iniciar();
  int valor() const;     // lectura cruda 0-4095 (sirve para calibrar)
  bool hayFuga();        // actualiza y devuelve el estado de fuga

private:
  int pin;
  int umbral;
  int margen;
  bool fugaConValorBajo;
  bool fuga = false;
};
