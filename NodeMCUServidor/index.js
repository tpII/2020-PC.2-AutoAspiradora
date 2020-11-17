// Se levanta un servidor web, utilizando Node.js y la librería Express.js

const express = require('express');
const app = express();
const path = require('path');


app.listen(3000, () => console.log("Servidor funcionando en puerto 3000"));

app.use(express.json());       // to support JSON-encoded bodies
app.use(express.urlencoded({ extended: true })); // to support URL-encoded bodies
app.post('/curl', (req, res) => {
    console.log(req.body);
    res.status(200).send("Informacion con curl recibida");
});

app.get('/',(req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

app.use('/css', express.static(__dirname + '/node_modules/bootstrap/dist/css'));
app.use('/icons', express.static(__dirname + '/node_modules/bootstrap-icons/icons'));
app.use('/imagenes',express.static(__dirname + '/imagenes'));
