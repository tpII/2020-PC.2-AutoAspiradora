////////////////////////////////////////////////
// Se importan todos los modelos              //
// para que esten dentro de la constante db   //
////////////////////////////////////////////////

const dbConfig = require("../database/config.js");

const Sequelize = require("sequelize");
const sequelizeCon = require("../database/config.js");

const db = {};

db.Sequelize = Sequelize;
db.sequelize = sequelizeCon;

db.vertice = require("./vertice.js");
db.datos = require("./dato.js");
db.grafo = require("./grafo.js");

module.exports = db;