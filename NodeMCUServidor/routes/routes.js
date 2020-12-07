//////////////////////////////////////////////////
// Se configuran los manejadores para cada ruta //
//////////////////////////////////////////////////
const Dato = require("../controllers/dato.controller.js");
const Grafo = require("../controllers/grafo.controller.js");
const Vertice = require("../controllers/vertice.controller.js");
const Estado = require("../controllers/estado.controller");
const router = require("express").Router();

// Ruta para crear datos de recorrido
//router.post("/", Dato.create);

// Ruta para descargar el archivo csv con los datos -> la simulación realiza este pedido
router.get("/csv", Dato.csvDatos);

// Ruta para almacenar información sobre el grafo -> la simulación realiza este pedido
router.post("/grafos", Grafo.create);

// Ruta para recuperar un grafo -> la interfaz web envia este pedido
router.get("/grafos", Grafo.findOne);

// Ruta para recuperar informacion de todos los grafos -> la interfaz web envia este pedido
router.get("/grafos/lista", Grafo.findAll);

// Ruta para almacenar datos de un vertice -> la simulación realiza este pedido
router.post("/vertices", Vertice.create);

// Ruta para recuperar todos los vertices -> la interfaz web envia este pedido
router.get("/vertices", Vertice.findAll);

// Ruta para crear datos de recorrido -> la simulación realiza este pedido
router.post("/dato", Dato.create);

// Ruta para modificar el estado del robot aspiradora -> la interfaz web envia este pedido
router.put("/estado", Estado.cambiarEstado);

// Ruta para consultar el estado actual del robot -> la simulación realiza este pedido
router.get("/consultaEstado", Estado.consultaEstado);

// Ruta para consultar el nombre de la habitacion a mapear -> la simulación realiza este pedido
router.get("/consultaNombre", Estado.consultaNombreMapeo);

// Ruta para indicar que debe iniciar el mapeo -> la interfaz web envia este pedido
router.put("/mapearHabitacion", Estado.mapearHabitacion);

// Ruta para consultar si ya finalizó el mapeo -> la interfaz web envia este pedido
router.get("/consultaEstadoMapeo", Estado.consultaEstadoMapeo);

module.exports = router;

