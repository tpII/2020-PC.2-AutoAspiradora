////////////////////////////////////////
// Conexión a la base de datos MySQL  //
////////////////////////////////////////
const Sequelize = require('sequelize');

// Formato: mysql://usuario:contraseña@localhost:puerto/nombreBD
const sequelizeCon = new Sequelize(`postgres://postgres:postgres@localhost:5433/asp`,{
    dialect: 'postgres',
    protocol: 'postgres',
});

module.exports = sequelizeCon;