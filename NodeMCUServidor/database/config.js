////////////////////////////////////////
// Conexión a la base de datos MySQL  //
////////////////////////////////////////
const Sequelize = require('sequelize');

// Formato: mysql://usuario:contraseña@localhost:puerto/nombreBD
const sequelizeCon = new Sequelize(`mysql://root:@localhost:3306/aspiradora`,{
    dialect: 'mysql',
    protocol: 'mysql',
});

module.exports = sequelizeCon;