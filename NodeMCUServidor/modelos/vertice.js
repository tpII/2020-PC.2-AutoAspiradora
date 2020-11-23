const sequelizeCon = require('../database/config.js');
const { Sequelize, DataTypes, Model } = require('sequelize');

const Vertice = sequelizeCon.define('experimento', {
    nombre: {
        type: DataTypes.STRING,
        allowNull: false,
        unique: true
    },
    descripcion: {
        type: DataTypes.STRING
    },
    activo: { // Activo o Finalizado
        type: DataTypes.BOOLEAN,
        allowNull: false
    },
    correoCreador: {
        type: DataTypes.STRING,
        allowNull: false
    }
});

Vertice.hasMany(Vertice);
module.exports = Vertice;