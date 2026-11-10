/*
 * Monitor IoT de tanque de agua - firmware del ESP32
 * Proyecto Integrador - Paradigmas de Programacion
 *
 * Paradigmas:
 *  - Orientado a objetos: cada componente fisico es una clase con su estado
 *    encapsulado (SensorNivel, SensorFuga, Valvula, Indicador, Alarma,
 *    Comunicacion). Este archivo solo los crea y los coordina.
 *  - Orientado a eventos: el loop detecta cambios (nivel, fuga, boton, WiFi)
 *    y llama a una funcion "alEvento..." solo cuando ocurren.
 *
 * Antes de compilar: copiar secrets.example.h como secrets.h y poner la red
 * WiFi y la IP de la laptop donde corre el backend.
 */

#include "config.h"
#include "secrets.h"
#include "SensorNivel.h"
#include "SensorFuga.h"
#include "Valvula.h"
#include "Indicador.h"
#include "Alarma.h"
#include "Comunicacion.h"

// ---------------- Objetos del sistema ----------------
SensorNivel  sensorNivel(PIN_TRIG, PIN_ECHO, ALTURA_TANQUE_CM, DISTANCIA_LLENO_CM);
SensorFuga   sensorFuga(PIN_AGUA, UMBRAL_FUGA, MARGEN_FUGA, FUGA_CON_VALOR_BAJO);
Valvula      valvula(PIN_SERVO, UMBRAL_ABRIR, UMBRAL_CERRAR);
Indicador    indicador(LCD_DIRECCION, PIN_LED_R, PIN_LED_G, PIN_LED_B);
Alarma       alarma(PIN_BUZZER, PIN_PULSADOR);
Comunicacion comunicacion(WIFI_SSID, WIFI_PASSWORD, SERVER_URL);

// Estado anterior, para detectar cambios (eventos)
EstadoNivel estadoAnterior = NORMAL;
bool fugaAnterior = false;
bool wifiAnterior = false;
unsigned long ultimoEnvioMs = 0;
unsigned long ultimoCicloMs = 0;

// ---------------- Funciones auxiliares ----------------
// Con margen: para cambiar de estado el nivel debe pasar el umbral por
// MARGEN_ESTADO puntos. Evita que el ruido del sensor dispare eventos seguidos.
EstadoNivel calcularEstado(int nivel, EstadoNivel anterior) {
  int bajo    = UMBRAL_BAJO  + (anterior == NORMAL  ? -MARGEN_ESTADO : MARGEN_ESTADO);
  int critico = UMBRAL_ABRIR + (anterior == CRITICO ?  MARGEN_ESTADO : -MARGEN_ESTADO);
  if (nivel < critico) return CRITICO;
  if (nivel < bajo)    return BAJO;
  return NORMAL;
}

void registrarEvento(const char* nombre) {
  Serial.print("EVENTO: ");
  Serial.println(nombre);
}

// ---------------- Manejadores de eventos ----------------
void alEventoBoton() {
  if (!alarma.estaSonando()) return;  // solo cuenta si habia algo que silenciar
  alarma.silenciar();
  registrarEvento("alarma_silenciada");
}

void alEventoValvula(CambioValvula cambio) {
  registrarEvento(cambio == SE_ABRIO ? "valvula_abierta" : "valvula_cerrada");
}

void alEventoCambioNivel(EstadoNivel nuevo) {
  if (nuevo == NORMAL)    registrarEvento("nivel_normal");
  else if (nuevo == BAJO) registrarEvento("nivel_bajo");
  else                    registrarEvento("nivel_critico");
  if (nuevo == NORMAL) alarma.rearmar();  // se resolvio: la proxima alarma vuelve a sonar
}

void alEventoFuga(bool hayFuga) {
  registrarEvento(hayFuga ? "fuga_detectada" : "fuga_resuelta");
  if (hayFuga) alarma.rearmar();  // una fuga nueva siempre suena
}

void alEventoWifi(bool conectado) {
  registrarEvento(conectado ? "wifi_conectado" : "wifi_desconectado");
}

void alEventoEnvio(const Lectura& lectura) {
  // Valor crudo del sensor de agua: sirve para calibrar UMBRAL_FUGA (prueba P04)
  Serial.printf("  sensor de agua (crudo 0-4095): %d\n", sensorFuga.valor());
  int codigo = comunicacion.enviar(lectura);
  if (codigo > 0) Serial.printf("  servidor respondio %d\n", codigo);
  else if (comunicacion.conectado()) Serial.println("  servidor no respondio");
}

// ---------------- Programa principal ----------------
void setup() {
  Serial.begin(115200);
  sensorNivel.iniciar();
  sensorFuga.iniciar();
  valvula.iniciar();
  indicador.iniciar();
  alarma.iniciar();
  comunicacion.iniciar();
  registrarEvento("dispositivo_iniciado");
}

void loop() {
  // El boton se atiende siempre, sin esperar al siguiente ciclo
  if (alarma.botonPresionado()) alEventoBoton();

  unsigned long ahora = millis();
  if (ahora - ultimoCicloMs < PERIODO_CICLO_MS) return;
  ultimoCicloMs = ahora;

  // 1. Leer sensores
  Lectura lectura;
  lectura.distanciaCm = sensorNivel.distanciaCm();
  lectura.nivel = sensorNivel.porcentaje(lectura.distanciaCm);
  lectura.fuga = sensorFuga.hayFuga();
  lectura.estado = calcularEstado(lectura.nivel, estadoAnterior);

  // 2. Detectar eventos y reaccionar
  CambioValvula cambio = valvula.controlar(lectura.nivel);
  if (cambio != SIN_CAMBIO) alEventoValvula(cambio);

  if (lectura.estado != estadoAnterior) {
    alEventoCambioNivel(lectura.estado);
    estadoAnterior = lectura.estado;
  }
  if (lectura.fuga != fugaAnterior) {
    alEventoFuga(lectura.fuga);
    fugaAnterior = lectura.fuga;
  }
  bool wifi = comunicacion.conectado();
  if (wifi != wifiAnterior) {
    alEventoWifi(wifi);
    wifiAnterior = wifi;
  }

  // 3. Actualizar salidas locales (funcionan con o sin WiFi, RNF-02)
  lectura.valvulaAbierta = valvula.estaAbierta();
  alarma.actualizar(lectura.estado == CRITICO || lectura.fuga);
  lectura.alarmaSonando = alarma.estaSonando();
  indicador.mostrar(lectura, wifi);

  // 4. Evento periodico: enviar la lectura al backend (RF-08)
  if (ahora - ultimoEnvioMs >= PERIODO_ENVIO_MS) {
    alEventoEnvio(lectura);
    ultimoEnvioMs = ahora;
  }
}
