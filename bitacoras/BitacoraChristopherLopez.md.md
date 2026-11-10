# Bitácora de desarrollo del Dashboard

**Proyecto:** Monitor IoT de tanque de agua  
**Responsable:** Christopher Marchena Telles

## Semana 1 — 17/10/2026 al 24/10/2026

**Trabajo realizado**
- Revisé qué información debía mostrar el dashboard para que fuera útil al supervisar el tanque.
- Organicé la página en secciones: nivel de agua, estado de la válvula, detección de fugas, resumen, gráfico y eventos.
- Preparé la estructura inicial en HTML, identificando los espacios que luego serían actualizados con los datos del sistema.

**Avance**
- Se definió la distribución general de la interfaz y los elementos principales que tendría la pantalla.

**Dificultades y solución**
- Al inicio no estaba claro cómo presentar varios datos sin recargar demasiado la página. Se optó por utilizar tarjetas para los indicadores y secciones separadas para el histórico y las alarmas.

**Aprendizaje**
- La estructura HTML permite organizar la información y dejar identificados los elementos que JavaScript actualizará posteriormente.

**Siguiente paso**
- Dar estilo a la interfaz y comenzar a trabajar con la información dinámica.

---

## Semana 2 — 25/10/2026 al 31/10/2026

**Trabajo realizado**
- Desarrollé los estilos CSS para las tarjetas, paneles, indicadores y gráfico.
- Apliqué una distribución adaptable para que la página pueda visualizarse en pantallas de distintos tamaños.
- Incorporé colores para diferenciar los estados normal, bajo y crítico, además de soporte para la apariencia clara u oscura del dispositivo.

**Avance**
- La interfaz adquirió una presentación más ordenada y legible.
- Se agregó una representación visual del tanque cuyo nivel puede cambiar según el porcentaje recibido.

**Dificultades y solución**
- Fue necesario acomodar los elementos para que no se desordenaran en pantallas pequeñas. Para resolverlo se utilizaron cuadrículas CSS y reglas de adaptación para móviles.

**Decisiones**
- Mantener una apariencia sencilla, priorizando que los indicadores importantes se puedan reconocer rápidamente.

**Aprendizaje**
- Uso de CSS Grid, variables de estilo y media queries para adaptar una página a diferentes tamaños de pantalla.

**Siguiente paso**
- Conectar la interfaz con el backend y mostrar los datos recibidos.

---

## Semana 3 — 01/11/2026 al 07/11/2026

**Trabajo realizado**
- Programé la comunicación entre el dashboard y el backend mediante solicitudes `fetch` a las rutas de lecturas, eventos y resumen.
- Configuré la actualización automática de la información cada cinco segundos, sin necesidad de recargar la página.
- Implementé la presentación del nivel actual, estado de la válvula, detección de fugas, buzzer y estadísticas generales.
- Agregué el listado de eventos y alarmas, diferenciando visualmente los eventos críticos.
- Incorporé un gráfico histórico con Chart.js, incluyendo las líneas de referencia del 20 % y 90 %.
- Añadí un indicador para mostrar si se están recibiendo datos o si la última lectura lleva más de 30 segundos sin actualizarse.

**Avance**
- El dashboard quedó preparado para consultar los datos almacenados por el backend y representar el estado del tanque en una sola pantalla.
- Se organizaron los archivos por responsabilidad: estructura HTML, estilos CSS, actualización de datos y configuración del gráfico JavaScript.

**Dificultades y solución**
- La interfaz depende de que el backend esté funcionando y entregue la información con el formato esperado. Se organizaron las consultas y el manejo de errores para mostrar un aviso cuando no sea posible contactar al servidor.
- También se tuvo que contemplar el caso en que todavía no existan lecturas o eventos, para evitar que la pantalla quede mostrando información confusa.

**Decisiones**
- Utilizar Chart.js para el histórico y consultar periódicamente las rutas del backend.
- Mantener separados el código que actualiza los indicadores y el que administra el gráfico.

**Pruebas y aprendizajes**
- Se revisó el comportamiento esperado de la actualización periódica, los estados visuales y la presentación de datos vacíos o de conexión fallida.
- Aprendí a consumir información desde una API con `fetch`, trabajar con respuestas JSON y actualizar elementos del HTML mediante JavaScript.

**Próximos pasos**
- Probar el dashboard junto con el backend y verificar su funcionamiento con lecturas reales enviadas por el ESP32.
