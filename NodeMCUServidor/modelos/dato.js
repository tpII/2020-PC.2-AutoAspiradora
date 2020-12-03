const sequelizeCon = require('../database/config.js');
const { Sequelize, DataTypes, Model } = require('sequelize');
const Vertice = require('./vertice');
const Grafo = require('./grafo');

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
        type: DataTypes.INTEGER
    },
    velocidad: {
        type: DataTypes.INTEGER
    }
});

Datos.belongsTo(Grafo);

module.exports = Datos;