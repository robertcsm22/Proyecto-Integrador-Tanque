#include "Comunicacion.h"
#include <WiFi.h>
#include <HTTPClient.h>

const int TIMEOUT_HTTP_MS = 2000;  // corto, para no frenar el control local

Comunicacion::Comunicacion(const char* ssid, const char* clave, const char* urlServidor)
  : ssid(ssid), clave(clave), urlServidor(urlServidor) {}

void Comunicacion::iniciar() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);  // si se cae el router, se reconecta solo
  WiFi.begin(ssid, clave);      // no espera: la conexion sigue en segundo plano
}

bool Comunicacion::conectado() const {
  return WiFi.status() == WL_CONNECTED;
}

int Comunicacion::enviar(const Lectura& lectura) {
  char json[200];
  snprintf(json, sizeof(json),
           "{\"distancia_cm\":%.1f,\"nivel\":%d,\"fuga\":%s,\"valvula\":\"%s\","
           "\"estado\":\"%s\",\"alarma\":%s}",
           lectura.distanciaCm, lectura.nivel,
           lectura.fuga ? "true" : "false",
           lectura.valvulaAbierta ? "abierta" : "cerrada",
           nombreEstado(lectura.estado),
           lectura.alarmaSonando ? "true" : "false");

  Serial.print("LECTURA ");
  Serial.println(json);

  if (!conectado()) return -1;

  HTTPClient http;
  http.setTimeout(TIMEOUT_HTTP_MS);
  http.begin(urlServidor);
  http.addHeader("Content-Type", "application/json");
  int codigo = http.POST(json);
  http.end();
  return codigo;
}
