const sequelizeCon = require('../database/config.js');
const { Sequelize, DataTypes, Model } = require('sequelize');
const Vertice = require('./vertice');
const Grafo = require('./grafo');

const Datos = sequelizeCon.define('datos', {
    distancia: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    velocidad: {
        type: DataTypes.INTEGER
    }
});

Datos.belongsTo(Vertice, {as:'origen'});
Datos.belongsTo(Vertice, {as:'destino'});
Datos.belongsTo(Grafo);

module.exports = Datos;