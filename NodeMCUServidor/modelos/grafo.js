const sequelizeCon = require('../database/config.js');
const { Sequelize, DataTypes, Model } = require('sequelize');

////////////////////////////////////////////////
// Se define el modelo de Grafo de recorrido  //
// para Sequelize                             //
////////////////////////////////////////////////
const Grafo = sequelizeCon.define('grafo', {
    nombre: {
        type: DataTypes.STRING,
        allowNull: false
    },
    longitudX: {
        type: DataTypes.INTEGER
    },
    longitudY:{
        type: DataTypes.INTEGER
    },
    vertices: {
        type: DataTypes.INTEGER
    }
});

module.exports = Grafo;