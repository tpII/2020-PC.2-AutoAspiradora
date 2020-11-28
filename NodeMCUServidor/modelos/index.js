const dbConfig = require("../database/config.js");

const Sequelize = require("sequelize");
const sequelizeCon = require("../database/config.js");

const db = {};

db.Sequelize = Sequelize;
db.sequelize = sequelizeCon;

db.vertice = require("./vertice.js");

module.exports = db;





// db.vertice.findAll().then(
//     (vertice) => {
//         vertice.forEach(element => {
//             console.log("x:" + element.x + ";y=" + element.y)
//         });
        
//     }
// )

// db.vertice.findByPk(1).then(
//     (data) => {
//         console.log(data);
//         data.addAdyacentes(db.vertice.findByPk(2))
//     });

async function test(){
    // await db.sequelize.sync();

    // const vertice1 = await db.vertice.create({
    //     x:0,
    //     y:1,
    //     visitado:0
    // });
    // const vertice2 = await db.vertice.create({
    //     x:1,
    //     y:1,
    //     visitado:0
    // });
    
    //  v1 = await db.vertice.findByPk(1);
    //  v2 = await db.vertice.findByPk(2);
    //  console.log(v2);
    // await v1.addAdyacente(v2);

}

test();