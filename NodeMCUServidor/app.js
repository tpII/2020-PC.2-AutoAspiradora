const express = require('express');
const app = express();
const bodyParser = require("body-parser");
const path = require('path');
const cors = require('cors');
const router = require('./routes/routes.js');

var corsOptions = {
    origin: "http://localhost:3000"
  };

app.use(cors(corsOptions));
app.use(bodyParser.json());       // to support JSON-encoded bodies

app.use(bodyParser.urlencoded({ extended: true })); // to support URL-encoded bodies
app.post('/curl', (req, res) => {
    console.log(req.body);
    res.status(200).send("Informacion con curl recibida");
});

app.get('/',(req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
    //res.json({ message: "Welcome" });
});
app.use('/css', express.static(__dirname + '/node_modules/bootstrap/dist/css'));
app.use('/icons', express.static(__dirname + '/node_modules/bootstrap-icons/icons'));
app.use('/imagenes',express.static(__dirname + '/imagenes'));
app.use('/api/robotAspiradora', router);

module.exports = app;