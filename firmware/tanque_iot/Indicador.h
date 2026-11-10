// Salidas visuales locales: LCD 16x2 I2C (RF-02) y LED RGB (RF-03).
#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

class Indicador {
public:
  Indicador(uint8_t direccionLcd, int pinR, int pinG, int pinB);
  void iniciar();
  void mostrar(const Lectura& lectura, bool wifiConectado);

private:
  LiquidCrystal_I2C lcd;
  int pinR, pinG, pinB;
  void color(bool r, bool g, bool b);
  void colorSegunEstado(EstadoNivel estado);
};
