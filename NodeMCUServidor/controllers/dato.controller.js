const db = require("../modelos/index.js");
const Dato = db.datos;

module.exports = db;
module.exports = Dato;

//Se crea y se guarda un nuevo vertice
async function create(req, res) {
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
        velocidad: req.body.vel_max,
        nombreGrafo: req.body.nombreGrafo
    };

    Dato.create(dato)
        .then(data => {
            res.send(data);
        })
        .catch(err => {
            res.status(500).send({
                message:
                    err.message || "Some error occurred while creating the Dato."
            })
        })
};

//Se genera el archivo csv con los datos del mapa 
async function csvDatos(req, res) {
    const CsvParser = require("json2csv").Parser;
    const nombreHabitacion = req.query.nombreGrafo;

    const datos = db.datos.findAll({
        where: {
          nombreGrafo: nombreHabitacion
        }
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
            res.setHeader("Content-Disposition", "attachment; filename=datos-" + nombreHabitacion + ".csv");

            res.status(200).end(csvData);

        })
        .catch(err => {
            res.status(500).send({
                message:
                    err.message || "No existe ninguna habitacion con ese nombre."
            })
        });

};

module.exports = {
    create,
    csvDatos
}