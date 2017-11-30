'use strict';

import * as express from "express"
import * as http from "http"


var renderer = require('bindings')('renderer');

var resFolder = "../../vulkan-rendering-engine/vulkan-rendering-engine/res/";
renderer.init(resFolder);

var width = 640;
var height = 480;
renderer.setResolution(width,height);

var app = express();
var server = http.createServer(app);
app.use(express.static('./html/'));

//Here we are configuring express to use body-parser as middle-ware.
var bodyParser = require("body-parser");
app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());

var fs = require('fs');
//var jsonFile = resFolder + "/scenes/bloom_test.json";
//var json = fs.readFileSync(jsonFile, 'utf8');

app.post('/render',function(request,res){
    console.log("Server: RECEIVED POST REQUEST");

    var json = JSON.stringify(request.body);
    //console.log(json);
    renderer.renderAsync(json, function(err, pixelBuffer) {
        if (err) {
            console.log(err);
        }
        else {
            res.send(pixelBuffer);
        }
    });
});

// Sends the pixel-data as a NodeJS Buffer
app.get('/render', function(req, res) {
    console.log("Server: RECEIVED GET REQUEST");
    var fileName = req.query.fp;
    var filePath = resFolder + "/scenes/" + fileName;

    if (fs.existsSync(filePath)) {
        var json = fs.readFileSync(filePath);

        renderer.renderAsync(json, function(err, pixelBuffer) {
            if (err) {
                console.log(err);
            }
            else {
                res.send(pixelBuffer);
            }
        });
    } else{
        console.log("RenderRequest failed: FILE #" + filePath + " does not exist!");
    }
});

// Sends the render-size in JSON format
app.get('/renderSize', function(req, res) {
    res.send({ width: width, height: height });
});

server.listen("8081"); 

// This needs to be called when the server should be shutdown
//renderer.shutdown();