var express = require('express'),
    app = express(),
    port = process.env.PORT || 3000,
    mongoose = require('mongoose'),
    Sensor = require('./api/models/meteoServerModel'), //created model loading here
    bodyParser = require('body-parser');

// mongoose instance connection url connection
mongoose.Promise = global.Promise;
mongoose.connect('mongodb://localhost/meteodb');

app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

var routes = require('./api/routes/meteoServerRoutes'); //importing route
routes(app); //register the route

app.listen(port);

console.log('MeteoServer RESTful API started on: '+ port);
