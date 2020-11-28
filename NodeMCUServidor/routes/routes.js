const Vertice = require("../controllers/controller.js");

const router = require("express").Router();

router.post("/", Vertice.create);

    
module.exports = router;

