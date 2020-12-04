const db = require("../modelos/index.js");
const Grafo = db.grafo;

module.exports = db;
module.exports = Grafo;

// Create and Save a new Vertice
async function create(req, res) {
    if (!req.body) {
        res.status(400).send({
            message: "El pedido esta vacio"
        });
        return;
    }

    const grafo = {
        nombre: req.body.nombre,
        vertices: req.body.vertices,
        longitudX: req.body.xMax,
        longitudY: req.body.yMax
    };

    Grafo.create(grafo)
        .then(data => {
            res.send(data);
        })
        .catch(err => {
            res.status(500).send({
                message:
                    err.message || "Algun error ocurrio al crear el grafo"
            })
        })
};


// exports.findAll = (req, res) => {

// };

// exports.findOne = (req, res) => {

// };

// exports.update = (req, res) => {

// };

// exports.delete = (req, res) => {

// };

// exports.deleteAll = (req, res) => {

// };

// exports.findAllPublished = (req, res) => {

// };

module.exports = {
    create
}