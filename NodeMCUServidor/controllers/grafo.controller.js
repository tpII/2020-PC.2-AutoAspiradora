const db = require("../modelos/index.js");
var estado= require("./estado.controller.js");
const Grafo = db.grafo;

module.exports = db;
module.exports = Grafo;


//Se crea y se guarda un nuevo grafo
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

    estado.resetMapeando();
    Grafo.create(grafo)
        .then(data => {
            res.send();
        })
        .catch(err => {
            res.status(500).send({
                message:
                    err.message || "Algun error ocurrio al crear el grafo"
            })
        })
};

//Se busca un grafo en especifico segun el nombre y se lo devuelve
async function findOne(req, res){
    const nombreHabitacion = req.query.nombreGrafo;

    db.grafo.findOne({
        where: {
            nombre: nombreHabitacion
        }
    })
    .then(data =>{
        res.send(data);
    }).catch(err => {
        res.status(500).send({
            message:
                err.message || "Algun error ocurrio al recuperar el grafo"
        })
    })
}
//Se buscan todos los grafos y se los devuelve
async function findAll(req, res){
    db.grafo.findAll({
        attributes: ['nombre']
    })
    .then(data => {
        res.send(data);
    }).catch(err => {
        res.status(500).send({
            message:
                err.message || "Algun error ocurrio al recuperar los grafos"
        })
    })
};
module.exports = {
    create,
    findOne,
    findAll
}