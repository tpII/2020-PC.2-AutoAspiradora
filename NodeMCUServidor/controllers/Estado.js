var estadoRobot = require("../server");

async function cambiarEstado(req, res) {
    console.log("LLEGO AL MANEJADOR")
    if (req.body) {
        estadoRobot = req.body.estadoRobot;
        res.sendStatus(200);
    }
    else{
        res.sendStatus(400);
    }
}

module.exports = {
    cambiarEstado,
}