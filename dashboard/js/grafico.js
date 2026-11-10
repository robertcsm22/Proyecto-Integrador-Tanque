// Configuracion del grafico historico con Chart.js.
// Dibuja el nivel en el tiempo y las lineas de los umbrales de la valvula.

const UMBRAL_ABRIR = 20;
const UMBRAL_CERRAR = 90;

function crearGrafico(canvas) {
  const lineaUmbral = (valor, etiqueta, color) => ({
    label: etiqueta,
    data: [],
    borderColor: color,
    borderDash: [6, 4],
    borderWidth: 1,
    pointRadius: 0,
    fill: false,
    valorFijo: valor,
  });

  return new Chart(canvas, {
    type: "line",
    data: {
      labels: [],
      datasets: [
        {
          label: "Nivel (%)",
          data: [],
          borderColor: "#2f80d1",
          backgroundColor: "rgba(47, 128, 209, 0.15)",
          fill: true,
          tension: 0.25,
          pointRadius: 0,
          borderWidth: 2,
        },
        lineaUmbral(UMBRAL_CERRAR, "Cierra válvula (90 %)", "#1e9e5a"),
        lineaUmbral(UMBRAL_ABRIR, "Abre válvula (20 %)", "#d23c3c"),
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      interaction: { mode: "index", intersect: false },
      scales: {
        y: { min: 0, max: 100, ticks: { callback: (v) => v + " %" } },
        x: { ticks: { maxTicksLimit: 8, maxRotation: 0 } },
      },
      plugins: { legend: { position: "bottom" } },
    },
  });
}

// Reemplaza los datos del grafico con las lecturas recibidas.
function actualizarGrafico(grafico, lecturas) {
  const etiquetas = lecturas.map((l) => l.fecha.slice(11, 19));  // hh:mm:ss
  grafico.data.labels = etiquetas;
  grafico.data.datasets[0].data = lecturas.map((l) => l.nivel);
  grafico.data.datasets.slice(1).forEach((d) => {
    d.data = etiquetas.map(() => d.valorFijo);
  });
  grafico.update();
}
