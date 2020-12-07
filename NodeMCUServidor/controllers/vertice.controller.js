const db = require("../modelos/index.js");
const Vertice = db.vertice;

module.exports = db;
module.exports = Vertice;

//Se guarda y crea un nuevo vertice
async function create(req, res) {
    if (!req.body.x) {
        res.status(400).send({
            message: "Content can not be empty!"
        });
        return;
    }

    const vertice = {
        x: req.body.x,
        y: req.body.y,
        estado: req.body.estado,
        nombreGrafo: req.body.nombreGrafo
        
    };

    Vertice.create(vertice)
        .then(data => {
            res.send();
        })
        .catch(err => {
            res.status(500).send({
                message:
                    err.message || "Some error occurred while creating the Vertice." //Esta linea no se bien si es asi
            })
        })
};

//Se buscan todos los vertices y se los devuelve
async function findAll(req, res){
    const nombreGrafo = req.query.nombreGrafo;
    const vertices = db.vertice.findAll({where: {
        nombreGrafo: nombreGrafo
    }})
    .then(data => {
        res.send(data);
    })
    .catch(err => {
        res.status(500).send({
          message:
            err.message || "Hubo un error recuperando los vertices."
        });
      });
}
module.exports = {
    create,
    findAll
}