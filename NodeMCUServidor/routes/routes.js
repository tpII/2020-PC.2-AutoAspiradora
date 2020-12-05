const Dato = require("../controllers/dato.controller.js");
const Grafo = require("../controllers/grafo.controller.js");
const Vertice = require("../controllers/controller.js");
const Estado = require("../controllers/Estado");

const router = require("express").Router();

router.post("/", Dato.create);

router.get("/csv", Dato.csvDatos);

router.post("/grafos", Grafo.create);

router.get("/grafos", Grafo.findOne);

router.post("/vertices", Vertice.create);

router.get("/vertices", Vertice.findAll);

router.post("/dato", Dato.create);

router.put("/estado", Estado.cambiarEstado);

module.exports = router;

