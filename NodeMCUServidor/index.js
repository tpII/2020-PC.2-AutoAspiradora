// Se levanta un servidor web, utilizando Node.js y la librería Express.js

const express = require('express');
const app = express();
const path = require('path');

app.listen(3000, () => console.log("Servidor funcionando en puerto 3000"));


app.get('/',(req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
})

app.use('/css', express.static(__dirname + '/node_modules/bootstrap/dist/css'));
app.use('/icons', express.static(__dirname + '/node_modules/bootstrap-icons/icons'));
app.use('/imagenes',express.static(__dirname + '/imagenes'));
