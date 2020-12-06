const direccion = {
    adelante: 0,
    abajo: 1,
    izquierda: 2,
    derecha: 3,
    nada: 4
}

var estadoRobot = {
    ventiladorEncendido: 0,
    modoAutomatico: 0,
    mapeando: 0,
    direccionManual: direccion.adelante
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
    if(estadoRobot.direccionManual != direccion.nada){
        estadoRobot.direccionManual = direccion.nada;
    }
}

async function mapearHabitacion(req, res){
    // Se configura el nombre de la habitacion a mapear y se modifica el estado del robot
    mapeoActual.nombreHabitacion = req.body.nombre;
    estadoRobot.mapeando = 1;
    res.sendStatus(200);
}

async function consultaNombreMapeo(req, res){
    // El robot consulta a esta el nombre de la habitacion a mapear
    console.log(mapeoActual.nombreHabitacion);
    res.send(mapeoActual.nombreHabitacion);
}

async function consultaEstadoMapeo(req, res){
    // El cliente consulta si termino de mapear
    if(mapeoActual.finalizo){
        res.status(200).send("Finalizo");
    }
    else{
        res.status(200).send("Procesando")
    }
}

module.exports = {
    cambiarEstado,
    consultaEstado,
    mapearHabitacion,
    consultaNombreMapeo,
    consultaEstadoMapeo
}