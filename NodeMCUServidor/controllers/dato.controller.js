const db = require("../modelos/index.js");
const Dato = db.datos;

module.exports = db;
module.exports = Dato;

// Create and Save a new Vertice
async function create(req, res) {
    if (!req.body.xorigen) {
        res.status(400).send({
            message: "Content can not be empty!"
        });
        return;
    }

    const dato = {
        xorigen: req.body.xorigen,
        yorigen: req.body.yorigen,
        xdestino: req.body.xdestino,
        ydestino: req.body.ydestino,
        distancia: req.body.distancia,
        velocidad: req.body.velocidad
    };

    Dato.create(dato)
        .then(data => {
            res.send(data);
        })
        .catch(err => {
            res.status(500).send({
                message:
                    err.message || "Some error occurred while creating the Dato." //Esta linea no se bien si es asi
            })
        })
};

async function csvDatos(req, res) {
    const CsvParser = require("json2csv").Parser;
    const datos = db.datos.findAll({
    })
        .then(data => {
            let datos = [];

            data.forEach((obj) => {
                const { xorigen, yorigen, xdestino, ydestino, distancia, velocidad } = obj;
                datos.push({ xorigen, yorigen, xdestino, ydestino, distancia, velocidad });
            });

            const csvFields = ["xorigen", "yorigen", "xdestino", "ydestino", "distancia", "velocidad"];
            const csvParser = new CsvParser({ csvFields });
            const csvData = csvParser.parse(datos);

            res.setHeader("Content-Type", "text/csv");
            res.setHeader("Content-Disposition", "attachment; filename=datos.csv");

            res.status(200).end(csvData);

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
    csvDatos
}