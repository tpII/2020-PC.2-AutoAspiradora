////////////////////////////////////////
// Conexión a la base de datos MySQL  //
////////////////////////////////////////
const Sequelize = require('sequelize');

const sequelizeCon = new Sequelize(`mysql://usuario:contraseña@localhost:3306/aspiradoraBD`,{
    dialect: 'mysql',
    protocol: 'mysql',
});

module.exports = sequelizeCon;