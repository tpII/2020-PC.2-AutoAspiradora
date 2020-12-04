const sequelizeCon = require('../database/config.js');
const { Sequelize, DataTypes, Model } = require('sequelize');

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