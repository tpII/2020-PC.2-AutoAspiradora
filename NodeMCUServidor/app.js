////////////////////////////////////////////////
// Se configuran los manejadores para la API  //
////////////////////////////////////////////////

const express = require('express');
const app = express();
const bodyParser = require("body-parser");
const path = require('path');
const cors = require('cors');
const router = require('./routes/routes.js');

// Configuracion para evitar errores de CORS
var corsOptions = {
    origin: "http://localhost:3000"
  };

app.use(cors(corsOptions));
// Configuracion para poder leer bodys en formato json
app.use(bodyParser.json());

// Soporte para url-encoded body
app.use(bodyParser.urlencoded({ extended: true })); 

// Al entrar a localhost:3000 devuelve el index.html que tiene la interfaz web
app.get('/',(req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// Devuelve los archivos de la libreria bootstrap
app.use('/js',express.static(__dirname + '/node_modules/bootstrap/dist/js'));
app.use('/css', express.static(__dirname + '/node_modules/bootstrap/dist/css'));
app.use('/icons', express.static(__dirname + '/node_modules/bootstrap-icons/icons'));

// En router estarán los manejadores para  todas las rutas del servidor
app.use('/api/robotAspiradora', router);

module.exports = app;