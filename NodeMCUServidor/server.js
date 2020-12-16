///////////////////////////////////////////////////////////
// Archivo principal del servidor                        //
// En este se levanta un servidor web en el puerto 3000  //
// utilizando Node.js y la librería Express.js           //
///////////////////////////////////////////////////////////
const path = require('path');
// app tiene todos los manejadores de las rutas de la API
const app = require('./app');


app.listen(3000, () => console.log("Servidor funcionando en puerto 3000"));
const db = require("./modelos");
db.sequelize.sync({force:true}); // CREA TODAS LAS TABLAS DE NUEVO.
//db.sequelize.sync(); // Solo sincroniza la BD.

