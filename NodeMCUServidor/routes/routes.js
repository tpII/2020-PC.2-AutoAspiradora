const Dato = require("../controllers/dato.controller.js");
const Grafo = require("../controllers/grafo.controller.js");
const Vertice = require("../controllers/controller.js");

const router = require("express").Router();

router.post("/", Dato.create);

router.get("/csv", Dato.csvDatos);

<<<<<<< HEAD
=======
router.post("/grafos", Grafo.create);

router.post("/vertices", Vertice.create);

router.post("/dato", Dato.create);
>>>>>>> 638e83375f8f1e6f16ab1d0bf1c8aca1a7e927d7

module.exports = router;

