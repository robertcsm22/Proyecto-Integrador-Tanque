// Dashboard: consulta el backend cada 5 s y actualiza la vista sin recargar (RNF-06).

const PERIODO_MS = 5000;
const SIN_DATOS_MS = 30000;  // si la ultima lectura es mas vieja, el ESP32 no esta enviando

const NOMBRES_EVENTO = {
  valvula_abierta: "Válvula abierta",
  valvula_cerrada: "Válvula cerrada",
  nivel_critico: "Nivel crítico",
  nivel_bajo: "Nivel bajo",
  nivel_normal: "Nivel normal",
  fuga_detectada: "Fuga detectada",
  fuga_resuelta: "Fuga resuelta",
};
const ES_ALARMA = new Set(["nivel_critico", "fuga_detectada"]);

const $ = (id) => document.getElementById(id);
const grafico = crearGrafico($("grafico"));

async function pedir(ruta) {
  const respuesta = await fetch(ruta, { cache: "no-store" });
  if (!respuesta.ok) throw new Error(`${ruta}: ${respuesta.status}`);
  return respuesta.json();
}

function mostrarActual(lectura) {
  if (!lectura) return;
  $("nivel").textContent = `${lectura.nivel} %`;
  $("agua").style.height = `${lectura.nivel}%`;

  const estado = $("estado");
  estado.textContent = { normal: "Normal", bajo: "Bajo", critico: "Crítico" }[lectura.estado];
  estado.className = `etiqueta estado-${lectura.estado}`;

  const valvula = $("valvula");
  valvula.textContent = lectura.valvula === "abierta" ? "Abierta" : "Cerrada";
  valvula.className = `valor ${lectura.valvula === "abierta" ? "estado-bajo" : ""}`;

  const fuga = $("fuga");
  fuga.textContent = lectura.fuga ? "Detectada" : "Sin fuga";
  fuga.className = `valor ${lectura.fuga ? "estado-critico" : "estado-normal"}`;
  $("alarma").textContent = lectura.alarma ? "Buzzer sonando" : "Buzzer apagado";
}

function mostrarResumen(r) {
  const pct = (v) => (v === null ? "–" : `${v} %`);
  $("promedio").textContent = pct(r.nivel_promedio);
  $("minmax").textContent = `${pct(r.nivel_minimo)} / ${pct(r.nivel_maximo)}`;
  $("consumo").textContent = `${r.consumo_estimado_litros} L`;
  $("total-alarmas").textContent = r.total_alarmas;
}

function mostrarEventos(eventos) {
  const lista = $("eventos");
  if (eventos.length === 0) {
    lista.innerHTML = '<li class="vacio">Sin eventos todavía</li>';
    return;
  }
  lista.replaceChildren(...eventos.map((e) => {
    const item = document.createElement("li");
    const fecha = document.createElement("span");
    fecha.className = "fecha";
    fecha.textContent = e.fecha.replace("T", " ");
    const texto = document.createElement("span");
    texto.className = ES_ALARMA.has(e.tipo) ? "alarma" : "";
    texto.textContent = `${NOMBRES_EVENTO[e.tipo] || e.tipo} — ${e.detalle}`;
    item.append(fecha, texto);
    return item;
  }));
}

function mostrarConexion(lecturas) {
  const conexion = $("conexion");
  const ultima = lecturas[lecturas.length - 1];
  if (!ultima) {
    conexion.textContent = "Esperando la primera lectura del ESP32";
    conexion.className = "conexion";
    return;
  }
  const edadMs = Date.now() - new Date(ultima.fecha).getTime();
  const reciente = edadMs < SIN_DATOS_MS;
  conexion.textContent = reciente
    ? `Recibiendo datos · última lectura ${ultima.fecha.slice(11, 19)}`
    : `Sin datos nuevos desde ${ultima.fecha.replace("T", " ")}`;
  conexion.className = `conexion ${reciente ? "ok" : "error"}`;
}

async function actualizar() {
  try {
    const [lecturas, eventos, resumen] = await Promise.all([
      pedir("/lecturas?limite=120"),
      pedir("/eventos?limite=30"),
      pedir("/resumen"),
    ]);
    mostrarActual(lecturas[lecturas.length - 1]);
    mostrarResumen(resumen);
    mostrarEventos(eventos);
    actualizarGrafico(grafico, lecturas);
    mostrarConexion(lecturas);
  } catch (error) {
    $("conexion").textContent = "No se puede contactar al servidor";
    $("conexion").className = "conexion error";
  }
}

actualizar();
setInterval(actualizar, PERIODO_MS);
