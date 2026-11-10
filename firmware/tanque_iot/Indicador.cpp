#include "Indicador.h"

Indicador::Indicador(uint8_t direccionLcd, int pinR, int pinG, int pinB)
  : lcd(direccionLcd, 16, 2), pinR(pinR), pinG(pinG), pinB(pinB) {}

void Indicador::iniciar() {
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Tanque IoT");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
}

void Indicador::mostrar(const Lectura& lectura, bool wifiConectado) {
  // Con fuga el LED se pone rojo aunque el nivel este bien.
  colorSegunEstado(lectura.fuga ? CRITICO : lectura.estado);

  // Fila 1: "Nivel:  75%   W"  (W = WiFi conectado)
  lcd.setCursor(0, 0);
  lcd.printf("Nivel: %3d%%  %s ", lectura.nivel, wifiConectado ? "W" : " ");

  // Fila 2: fuga tiene prioridad sobre el estado de la valvula
  lcd.setCursor(0, 1);
  if (lectura.fuga)                lcd.print("!! FUGA !!      ");
  else if (lectura.valvulaAbierta) lcd.print("Valvula: ABIERTA");
  else                             lcd.print("Valvula: CERRADA");
}

void Indicador::color(bool r, bool g, bool b) {
  digitalWrite(pinR, r);
  digitalWrite(pinG, g);
  digitalWrite(pinB, b);
}

void Indicador::colorSegunEstado(EstadoNivel estado) {
  if (estado == NORMAL)    color(0, 1, 0);  // verde
  else if (estado == BAJO) color(1, 1, 0);  // amarillo (rojo + verde)
  else                     color(1, 0, 0);  // rojo
}
