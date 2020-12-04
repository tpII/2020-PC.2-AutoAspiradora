const db = require("../modelos/index.js");
const Dato = db.datos;

module.exports = db;
module.exports = Dato;

// Create and Save a new Vertice
async function create(req, res) {
    console.log("llego un dato");
    console.log(req.body);
    if (!req.body) {
        res.status(400).send({
            message: "Content can not be empty!"
        });
        return;
    }

    const dato = {
        xorigen: req.body.origen_x,
        yorigen: req.body.origen_y,
        xdestino: req.body.fin_x,
        ydestino: req.body.fin_y,
        distancia: req.body.distancia,
        velocidad: req.body.vel_max
    };

    Dato.create(dato)
        .then(data => {
            console.log("Se crea dato 2");
            console.log(data);
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