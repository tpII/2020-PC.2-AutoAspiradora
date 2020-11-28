const db = require("../modelos/index.js");
const Vertice = db.vertice;

module.exports = db;
module.exports = Vertice;

// Create and Save a new Vertice
async function create(req, res) {
    if (!req.body.x) {
        res.status(400).send({
          message: "Content can not be empty!"
        });
        return;
      }

    const vertice={
        x: req.body.x,
        y: req.body.y,
        visitado: req.body.visitado
    };

    Vertice.create(vertice)
        .then(data => {
            res.send(data);
        })
        .catch(err => {
            res.status(500).send({
                message:
                    err.message || "Some error occurred while creating the Vertice." //Esta linea no se bien si es asi
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
    create,
}