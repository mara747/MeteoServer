'use strict';

var mongoose = require('mongoose'),
  Sensor = mongoose.model('Sensors');

exports.list_all_sensors = function(req, res) {
  Sensor.find({}, function(err, sensor) {
    if (err)
      res.send(err);
    res.json(sensor);
  });
};

exports.create_a_sensor = function(req, res) {
  var new_sensor = new Sensor(req.body);
  new_sensor.save(function(err, sensor) {
    if (err)
      res.send(err);
    res.json(sensor);
  });
};

exports.read_a_sensor = function(req, res) {
  Sensor.findById(req.params.sensorId, function(err, sensor) {
    if (err)
      res.send(err);
    res.json(sensor);
  });
};

exports.update_a_sensor = function(req, res) {
  Sensor.findOneAndUpdate({_id: req.params.sensorId}, req.body, {new: true}, function(err, sensor) {
    if (err)
      res.send(err);
    res.json(Sensor);
  });
};

exports.delete_a_sensor = function(req, res) {
  Sensor.remove({
    _id: req.params.sensorId
  }, function(err, sensor) {
    if (err)
      res.send(err);
    res.json({ message: 'Sensor successfully deleted' });
  });
};
