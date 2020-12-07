const sequelizeCon = require('../database/config.js');
const { Sequelize, DataTypes, Model } = require('sequelize');
const Grafo = require('./grafo');

///////////////////////////////////////
// Se define el modelo de Vertice    //
// para Sequelize                    //
///////////////////////////////////////
const Vertice = sequelizeCon.define('vertice', {
    x: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    y: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    estado:{
        type: DataTypes.INTEGER
    },
    nombreGrafo:{
        type: DataTypes.STRING
    }
});

module.exports = Vertice;