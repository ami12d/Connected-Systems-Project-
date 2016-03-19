/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
document.getElementById("submit").addEventListener("click", setAlarm);
document.getElementById("cancel").addEventListener("click", stopAlarm);
var interval;
var exec = require("cordova/exec");
var counter = 0;
var alarmDate;
var alarmType;
var audio;
var value;
var difftime;

/**
 * This is a global variable called wakeup exposed by cordova
 */
var Wakeup = function(){};

Wakeup.prototype.wakeup = function(success, error, options) {
    exec(success, error, "WakeupPlugin", "wakeup", [options]);
};

Wakeup.prototype.snooze = function(success, error, options) {
    exec(success, error, "WakeupPlugin", "snooze", [options]);
};

module.exports = new Wakeup();
var app = {
    // Application Constructor
    initialize: function() {
        this.bindEvents();
    },
    // Bind Event Listeners
    //
    // Bind any events that are required on startup. Common events are:
    // 'load', 'deviceready', 'offline', and 'online'.
    bindEvents: function() {
        
    },
    // deviceready Event Handler
    //
    // The scope of 'this' is the event. In order to call the 'receivedEvent'
    // function, we must explicitly call 'app.receivedEvent(...);'
    onDeviceReady: function() {
        app.receivedEvent('deviceready');
    },
    // Update DOM on a Received Event
    receivedEvent: function(id) {
        var parentElement = document.getElementById(id);
        var listeningElement = parentElement.querySelector('.listening');
        var receivedElement = parentElement.querySelector('.received');

        listeningElement.setAttribute('style', 'display:none;');
        receivedElement.setAttribute('style', 'display:block;');

        console.log('Received Event: ' + id);
    }
};

app.initialize();

function setAlarm(){
    setTime=document.getElementById("clock").value;
    parsedTime=setTime.split(':');
    if(document.getElementById("r1").checked)
    {
        alarmType = 0;
    }
    if(document.getElementById("r2").checked)
    {
        alarmType = 1;
    }
    if(document.getElementById("r3").checked)
    {
        alarmType = 2;
    }
    alarmDate = new Date();
    alarmDate.setHours(parsedTime[0]);
    alarmDate.setMinutes(parsedTime[1]);
    alarmDate.setSeconds(0);
    alarmDate.setMilliseconds(0);
    navigator.plugins.alarm.set(alarmDate, 
        function(){
            //SUCCESS
            interval = setInterval(checkTrigerred, 250);
            alert("Alarm Set at: " + alarmDate);
        }, 
        function(){
            //FAIL
            alert("Failed");
        });
};

function stopAlarm(){
    window.location="index.html";
};

function checkTrigerred() {
    difference = Math.abs(alarmDate - new Date());
    if(difference < 500)
    {
	difftime = new Date().toISOString().slice(0, 19).replace('T', ' ');
        audio = new Audio('audio/alarm.mp3');
        audio.loop = true;
        audio.play();
        if(alarmType == 2)
        {
            document.getElementById("cancel").style.display = "inline";
        }
        else
        {
            $.post('http://miniproject.eu-gb.mybluemix.net/alarmStatus', {"status":1});
            window.clearInterval(interval);
            getValue();
        }
    }
};
function getValue()
{
    value = $.get('http://miniproject.eu-gb.mybluemix.net/test', callback);
};
function callback()
{
    if(value.responseText == "0")
    {
        alert(flag);
<<<<<<< Updated upstream
	    $.post('http://miniproject.eu-gb.mybluemix.net/diffTime', {"time":difftime});
        window.clearInterval(interval);
        window.location="index.html";
=======
        window.clearInterval(interval);
        window.location="index.html";
>>>>>>> Stashed changes
    }
    setTimeout(getValue, 250);
};
