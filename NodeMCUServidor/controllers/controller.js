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

    const vertice = {
        x: req.body.x,
        y: req.body.y,
        estadoGrafo: req.body.estado
        
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


async function csvVertices(req, res) {
    const fastcsv = require("fast-csv");
    const fs = require("fs");


    var nombreHabitacion = "Habitacion1"
    const ws = fs.createWriteStream(nombreHabitacion + ".csv");

    const vertices = db.vertice.findAll({
        attributes: ['x', 'y', 'visitado']
    })
        .then(data => {
            fastcsv
                .write(JSON.parse(JSON.stringify(data)), { headers: true })
                .on("finish", function () {
                    // res.send("<a href='Habitacion1.csv' download='Habitacion1.csv' id='link-descarga'></a><script>document.getElementById('link-descarga').click();</script>");

                    res.setHeader("Content-Type", "text/csv");
                    res.setHeader("Content-Disposition", "attachment; filename=tutorials.csv");

                    res.download("./Habitacion1.csv");

                })
                .pipe(ws);

        });

};

async function csvDatos(req, res) {
    const fastcsv = require("fast-csv");
    const fs = require("fs");

    var nombreHabitacion = "Habitacion1"
    const ws = fs.createWriteStream(nombreHabitacion + ".csv");

    const datos = db.datos.findAll({
    })
        .then(data => {
            fastcsv
                .write(JSON.parse(JSON.stringify(data)), { headers: true })
                .on("finish", function () {
                    // res.send("<a href='Habitacion1.csv' download='Habitacion1.csv' id='link-descarga'></a><script>document.getElementById('link-descarga').click();</script>");

                    res.setHeader("Content-Type", "text/csv");
                    res.setHeader("Content-Disposition", "attachment; filename=tutorials.csv");

                    res.download("./Habitacion1.csv");

                })
                .pipe(ws);

                data[0].id;

        });

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
    csvVertices,
    csvDatos
}