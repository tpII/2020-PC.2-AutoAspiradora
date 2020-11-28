// Se levanta un servidor web, utilizando Node.js y la librería Express.js
const app = require('./app');

app.listen(3000, () => console.log("Servidor funcionando en puerto 3000"));
const db = require("./modelos");
db.sequelize.sync();