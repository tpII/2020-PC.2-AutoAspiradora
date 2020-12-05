// Se levanta un servidor web, utilizando Node.js y la librería Express.js
const { writeToPath } = require('@fast-csv/format');
const app = require('./app');
const path = require('path');

var estadoRobot = {
    ventiladorEncendido: false,
    modoAutomatico: false,
    mapeando: false,
    direccionManual: 'Adelante'
}

app.listen(3000, () => console.log("Servidor funcionando en puerto 3000"));
const db = require("./modelos");
db.sequelize.sync({force:true});
//db.sequelize.sync();


module.exports = estadoRobot;

// db

// const fs = require("fs");


// var nombreHabitacion = "Habitacion1"
// const ws = fs.createWriteStream(nombreHabitacion + ".csv");

// const vertices = db.vertice.findAll({
//     attributes: ['x', 'y', 'visitado']
// })
//     .then(data => {
//         fastcsv
//             .write(JSON.parse(JSON.stringify(data)), { headers: true })
//             .on("finish", function () {

//             })
//             .pipe(ws);
//     });

// const datos = db.datos.findAll({
// })
//     .then(data => {
//         let datos = [];

//         data.forEach((obj) => {
//             const { xorigen, yorigen, xdestino, ydestino, distancia, velocidad } = obj;
//             datos.push({ xorigen, yorigen, xdestino, ydestino, distancia, velocidad });
//         });

//         const csvFields = ["xorigen", "yorigen", "xdestino", "ydestino", "distancia", "velocidad"];
//         const csvParser = new CsvParser({ csvFields });
//         const csvData = csvParser.parse(datos);

//         res.setHeader("Content-Type", "text/csv");
//         res.setHeader("Content-Disposition", "attachment; filename=datos.csv");

//         res.status(200).end(csvData);

//     });

