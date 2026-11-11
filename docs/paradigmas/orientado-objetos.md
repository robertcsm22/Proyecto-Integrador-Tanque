# Paradigma orientado a objetos (firmware)

## Dónde se aplica

En el firmware del ESP32 ([firmware/tanque_iot/](../../firmware/tanque_iot/)). Cada
componente físico es una clase con sus datos privados y una interfaz pública pequeña:

| Clase | Encapsula | Interfaz pública |
|-------|-----------|------------------|
| `SensorNivel` | Pines Trig/Echo, altura del tanque, cálculo del eco | `iniciar()`, `distanciaCm()`, `porcentaje()` |
| `SensorFuga` | Pin analógico y umbral | `iniciar()`, `valor()`, `hayFuga()` |
| `Valvula` | Objeto `Servo`, umbrales, si está abierta | `iniciar()`, `controlar(nivel)`, `estaAbierta()` |
| `Indicador` | Objeto `LiquidCrystal_I2C` y pines del LED RGB | `iniciar()`, `mostrar(lectura, wifi)` |
| `Alarma` | Buzzer, pulsador, si está silenciada o sonando | `actualizar()`, `botonPresionado()`, `silenciar()`, `rearmar()` |
| `Comunicacion` | Red WiFi y URL del servidor | `iniciar()`, `conectado()`, `enviar(lectura)` |

## Evidencia en el código

**Encapsulamiento.** Nadie fuera de `Valvula` puede mover el servo directamente:
`abrir()` y `cerrar()` son privados. Solo se puede pedir que *decida* según el nivel,
y la regla de histéresis queda protegida dentro de la clase:

```cpp
// Valvula.h
class Valvula {
public:
  CambioValvula controlar(int nivel);  // decide segun el nivel y avisa si cambio
  bool estaAbierta() const;
private:
  Servo servo;
  bool abierta = false;
  void abrir();
  void cerrar();
};
```

```cpp
// Valvula.cpp
CambioValvula Valvula::controlar(int nivel) {
  if (nivel < umbralAbrir && !abierta)  { abrir();  return SE_ABRIO; }
  if (nivel > umbralCerrar && abierta)  { cerrar(); return SE_CERRO; }
  return SIN_CAMBIO;  // entre 20 % y 90 % se mantiene como estaba
}
```

**Abstracción.** `tanque_iot.ino` no sabe qué es un pulso de eco ni una dirección I2C.
Usa los objetos por lo que hacen:

```cpp
lectura.distanciaCm = sensorNivel.distanciaCm();
lectura.nivel       = sensorNivel.porcentaje(lectura.distanciaCm);
lectura.fuga        = sensorFuga.hayFuga();
CambioValvula cambio = valvula.controlar(lectura.nivel);
indicador.mostrar(lectura, wifi);
```

**Composición.** `Valvula` contiene un `Servo` e `Indicador` contiene un
`LiquidCrystal_I2C`: las clases propias se construyen sobre objetos de librerías.

**Constructores con parámetros.** Los pines y umbrales llegan por el constructor desde
`config.h`, así la misma clase sirve con otra conexión sin tocar su código:

```cpp
Valvula valvula(PIN_SERVO, UMBRAL_ABRIR, UMBRAL_CERRAR);
```

## Justificación

- **Un componente físico = un objeto.** El modelo del código coincide con la maqueta:
  si falla el buzzer, se revisa `Alarma`; si cambia el sensor de agua, solo se toca `SensorFuga`.
- **Se puede probar por partes.** Cada clase se puede montar y probar sola (por ejemplo,
  `SensorNivel` con el monitor serie) antes de conectar el resto.
- **Trabajo en equipo.** Cada integrante puede trabajar en su archivo `.h/.cpp` sin pisar
  el de otro, lo que también hace los commits más claros.
- **Evita el "loop gigante".** Sin clases, `loop()` tendría cientos de líneas mezclando
  pines, cálculos y reglas. Con clases, `loop()` se lee como una lista de pasos.
