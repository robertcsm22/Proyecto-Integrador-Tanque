# Dashboard (HTML · JavaScript · Chart.js)

Muestra el nivel actual, el estado de la válvula, la fuga, un resumen, el gráfico
histórico y la lista de alarmas (RF-10). Se actualiza solo cada 5 s sin recargar
la página (RNF-06).

| Archivo | Contenido |
|---------|-----------|
| `index.html` | Estructura de la página |
| `css/estilos.css` | Estilos (modo claro y oscuro) |
| `js/app.js` | `fetch` periódico a `/lecturas`, `/eventos` y `/resumen`; actualiza la vista |
| `js/grafico.js` | Gráfico de Chart.js con las líneas de los umbrales 20 % y 90 % |

No se abre como archivo suelto: lo sirve Flask. Con el backend corriendo, entrar a
`http://localhost:5000` (o `http://<IP-de-la-laptop>:5000` desde otro dispositivo).

Chart.js se carga desde un CDN, así que la laptop necesita internet la primera vez
que abre la página.

El indicador de arriba a la derecha dice "Recibiendo datos" mientras lleguen lecturas
y "Sin datos nuevos" si pasan más de 30 s sin lecturas (por ejemplo, si se cae el WiFi).
