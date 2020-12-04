const Dato = require("../controllers/dato.controller.js");
const Grafo = require("../controllers/grafo.controller.js");
const Vertice = require("../controllers/controller.js");

const router = require("express").Router();

router.post("/", Dato.create);

router.get("/csv", Dato.csvDatos);

router.post("/grafos", Grafo.create);

router.post("/vertices", Vertice.create);

router.post("/dato", Dato.create);

module.exports = router;

