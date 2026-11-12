// Configuracion general del dispositivo: pines, umbrales y tipos compartidos.
// Cambiar aqui los valores al calibrar con el tanque real.
#pragma once
#include <Arduino.h>

// ---------------- Pines (ESP32 DevKit) ----------------
const int PIN_TRIG     = 5;   // HC-SR04 Trig
const int PIN_ECHO     = 18;  // HC-SR04 Echo (con divisor 1 kOhm / 2 kOhm, RNF-03)
const int PIN_AGUA     = 34;  // sensor de agua (entrada analogica)
const int PIN_SDA      = 21;  // LCD I2C
const int PIN_SCL      = 22;
const int PIN_LED_R    = 25;  // LED RGB catodo comun, 220 Ohm por color
const int PIN_LED_G    = 26;
const int PIN_LED_B    = 27;
const int PIN_BUZZER   = 14;
const int PIN_SERVO    = 13;  // alimentado con fuente aparte y GND comun (RNF-04)
const int PIN_PULSADOR = 4;   // a GND, con INPUT_PULLUP

// ---------------- Tanque y umbrales ----------------
const float ALTURA_TANQUE_CM = 16.8;  // distancia del sensor al fondo (medido en la maqueta = 0 %)
const float DISTANCIA_LLENO_CM = 3.0; // distancia del sensor al agua con el tanque lleno (medido en la maqueta = 100 %)
const int   UMBRAL_ABRIR     = 20;    // %: abrir valvula por debajo (RF-06)
const int   UMBRAL_CERRAR    = 90;    // %: cerrar valvula por encima (RF-06)
const int   UMBRAL_BAJO      = 50;    // %: verde sobre 50, amarillo 20-50, rojo bajo 20 (RF-03)
const int   UMBRAL_FUGA      = 3700;  // 0-4095. Calibrado: seco 4095, mojado ~3300 (prueba P04)
const int   MARGEN_ESTADO    = 3;     // %: margen para cambiar de estado (evita eventos por ruido)
const int   MARGEN_FUGA      = 250;   // para dar la fuga por resuelta la lectura debe subir a 3950
// Modulo de humedad de suelo con LM393 (salida AO): da valores ALTOS en seco y BAJOS
// mojado, asi que hay fuga cuando la lectura baja del umbral -> true.
// Con el sensor de agua rojo del kit (sube al mojarse) poner false.
const bool  FUGA_CON_VALOR_BAJO = true;

// ---------------- Tiempos ----------------
const unsigned long PERIODO_CICLO_MS = 200;   // cada cuanto se leen los sensores
const unsigned long PERIODO_ENVIO_MS = 5000;  // RF-08: enviar cada 5-10 s

// ---------------- LCD ----------------
const uint8_t LCD_DIRECCION = 0x27;  // si no muestra nada, probar 0x3F

// ---------------- Tipos compartidos ----------------
enum EstadoNivel { NORMAL, BAJO, CRITICO };

// Una lectura completa del dispositivo; es lo que se envia al backend.
struct Lectura {
  float distanciaCm;
  int nivel;            // %
  bool fuga;
  bool valvulaAbierta;
  EstadoNivel estado;
  bool alarmaSonando;
};

inline const char* nombreEstado(EstadoNivel e) {
  if (e == NORMAL) return "normal";
  if (e == BAJO)   return "bajo";
  return "critico";
}
