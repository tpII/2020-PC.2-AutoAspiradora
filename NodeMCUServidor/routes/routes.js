const Dato = require("../controllers/dato.controller.js");

const router = require("express").Router();

router.post("/", Dato.create);

router.get("/csv", Dato.csvDatos);


module.exports = router;

