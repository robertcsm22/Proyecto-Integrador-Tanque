// Comunicacion con el backend: WiFi + HTTP POST con JSON (RF-08).
// Si no hay red, el dispositivo sigue funcionando localmente (RNF-02).
#pragma once
#include <Arduino.h>
#include "config.h"

class Comunicacion {
public:
  Comunicacion(const char* ssid, const char* clave, const char* urlServidor);
  void iniciar();
  bool conectado() const;
  int enviar(const Lectura& lectura);  // codigo HTTP, o negativo si no se pudo

private:
  const char* ssid;
  const char* clave;
  const char* urlServidor;
};
