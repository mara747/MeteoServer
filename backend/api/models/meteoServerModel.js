'use strict';
var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var SensorSchema = new Schema({
    name: {
        type: String,
        required: 'Kindly enter the name of the Sensor'
    },
    units: {
        type: String,
        required: 'Kindly enter the units of the Sensor'
    },
    value: {
        type: Number,
        default: 0.0
    },
    last_update: {
        type: Date,
        default: null
    }
});

module.exports = mongoose.model('Sensors', SensorSchema);
