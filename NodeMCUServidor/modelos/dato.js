const sequelizeCon = require('../database/config.js');
const { Sequelize, DataTypes, Model } = require('sequelize');
const Vertice = require('./vertice');
const Grafo = require('./grafo');

////////////////////////////////////////////////
// Se define el modelo de dato de recorrido  //
// para Sequelize                            //
///////////////////////////////////////////////
const Datos = sequelizeCon.define('datos', {
    xorigen: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    yorigen: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    xdestino: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    ydestino: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    distancia: {
        type: DataTypes.FLOAT
    },
    velocidad: {
        type: DataTypes.FLOAT
    },
    nombreGrafo: {
        type: DataTypes.STRING
    }
});

module.exports = Datos;