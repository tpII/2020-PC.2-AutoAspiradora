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
// app.use((req, res, next) => {
//     res.header('Access-Control-Allow-Origin', '*');
//     res.header('Access-Control-Allow-Headers', 'Authorization, X-API-KEY, Origin, X-Requested-With, Content-Type, Accept, Access-Control-Allow-Request-Method');
//     res.header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS, PUT, DELETE');
//     res.header('Allow', 'GET, POST, OPTIONS, PUT, DELETE');
//     next();
// });
app.use(bodyParser.json());       // to support JSON-encoded bodies

app.use(bodyParser.urlencoded({ extended: true })); // to support URL-encoded bodies

app.get('/',(req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
    //res.json({ message: "Welcome" });
});
app.use('/js',express.static(__dirname + '/node_modules/bootstrap/dist/js'));
app.use('/css', express.static(__dirname + '/node_modules/bootstrap/dist/css'));
app.use('/icons', express.static(__dirname + '/node_modules/bootstrap-icons/icons'));
app.use('/imagenes',express.static(__dirname + '/imagenes'));
app.use('/api/robotAspiradora', router);

module.exports = app;