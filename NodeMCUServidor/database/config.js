////////////////////////////////////////
// Conexión a la base de datos MySQL  //
////////////////////////////////////////
const Sequelize = require('sequelize');

const sequelizeCon = new Sequelize(`mysql://root:@localhost:3306/aspiradoraBD`,{
    dialect: 'mysql',
    protocol: 'mysql',
});

module.exports = sequelizeCon;

// module.exports = {
//     HOST: "localhost",
//     USER: "root",
//     PASSWORD: "",
//     DB: "aspiradoraBD",
//     dialect: "mysql",
//     pool: {
//       max: 5,
//       min: 0,
//       acquire: 30000,
//       idle: 10000
//     }
//   };