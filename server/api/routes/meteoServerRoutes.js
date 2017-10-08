'use strict';
module.exports = function (app) {
    var meteoServer = require('../controllers/meteoServerController');

    // todoList Routes
    app.route('/sensors')
        .get(meteoServer.list_all_sensors)
        .post(meteoServer.create_a_sensor);

    app.route('/sensors/:sensorId')
        .get(meteoServer.read_a_sensor)
        .put(meteoServer.update_a_sensor)
        .delete(meteoServer.delete_a_sensor);
};
