const direccion = {
    adelante: 0,
    abajo: 1,
    izquierda: 2,
    derecha: 3,
    nada: 4
}

estadoRobot = {
    ventiladorEncendido: 0,
    modoAutomatico: 0,
    mapeando: 0,
    direccionManual: direccion.nada
}

var mapeoActual = {
    nombreHabitacion: "",
    finalizo: 0,
}

async function cambiarEstado(req, res) {
    console.log("LLEGO AL MANEJADOR")
    if (req.body) {
        estadoRobot = req.body.estadoRobot;
        res.sendStatus(200);
    }
    else {
        res.sendStatus(400);
    }
}

async function consultaEstado(req, res) {
    console.log(estadoRobot);
    res.send(estadoRobot.ventiladorEncendido + "" + estadoRobot.modoAutomatico + "" + estadoRobot.mapeando + "" + estadoRobot.direccionManual)
    if (estadoRobot.direccionManual != direccion.nada) {
        estadoRobot.direccionManual = direccion.nada;
    }
}

async function mapearHabitacion(req, res) {
    // Se configura el nombre de la habitacion a mapear y se modifica el estado del robot
    mapeoActual.nombreHabitacion = req.body.nombre;
    mapeoActual.finalizo = 0;
    estadoRobot.mapeando = 1;
    res.sendStatus(200);
}

async function consultaNombreMapeo(req, res) {
    // El robot consulta a esta el nombre de la habitacion a mapear
    console.log(mapeoActual.nombreHabitacion);
    res.send(mapeoActual.nombreHabitacion);
}

async function consultaEstadoMapeo(req, res) {
    // El cliente consulta si termino de mapear
    console.log("CONSULTA EL MAPEO");
    if (mapeoActual.finalizo) {
        res.status(200).send(true);
    }
    else {
        res.status(200).send(false)
    }
}

function resetMapeando() {
    estadoRobot.mapeando = 0;
    mapeoActual.finalizo = 1;
}

module.exports = {
    cambiarEstado,
    consultaEstado,
    mapearHabitacion,
    consultaNombreMapeo,
    consultaEstadoMapeo,
    resetMapeando
}

