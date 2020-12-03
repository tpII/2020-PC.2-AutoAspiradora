const sequelizeCon = require('../database/config.js');
const { Sequelize, DataTypes, Model } = require('sequelize');
const Grafo = require('./grafo');

const Vertice = sequelizeCon.define('vertice', {
    x: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    y: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    visitado:{
        type: DataTypes.INTEGER
    }
});

// Vertice.hasMany(Vertice, {
//     as:'adyacentes',
//     foreignKey:"adyacenteID"
// });

// Vertice.belongsTo(Vertice);

Vertice.belongsToMany(Vertice, {as:'adyacentes', through: 'Adyacentes' });

Vertice.belongsTo(Grafo);
// Vertice.hasOne(Vertice, {
//     as:'siguiente'
// })
module.exports = Vertice;