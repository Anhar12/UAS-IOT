const express = require('express');
const mqtt = require("mqtt");
const antares = require('antares-http');

const app = express();
const port = 3000;

antares.setAccessKey('ee8d16c4466b58e1:3b8d814324c84c89');

const mqttClient = mqtt.connect('mqtt://broker.emqx.io');

function sendSuhuToMQTT() {
  antares.get('AQWM', 'weather_airQuality_nodeCore_teknik')
    .then(function(response) {
      const data = response.content;
      mqttClient.publish('uasiot/kematian/suhu', data.Temp.toString());
      console.log("Berhasil kirim suhu ke uasiot/kematian/suhu \nSuhu:", data.Temp);
    })
    .catch(function(error) {
      console.error('Error fetching data from Antares:', error);
    });
}

function sendHumToMQTT() {
  antares.get('AQWM', 'weather_airQuality_nodeCore_teknik')
    .then(function(response) {
      const data = response.content;
      mqttClient.publish('uasiot/kematian/hum', data.Hum.toString());
      console.log("Berhasil kirim suhu ke uasiot/kematian/hum \nKelembapan:", data.Hum);
    })
    .catch(function(error) {
      console.error('Error fetching data from Antares:', error);
    });
}

setInterval(sendSuhuToMQTT, 1000);
setInterval(sendHumToMQTT, 1000);

mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
});

mqttClient.on('error', (err) => {
  console.error('MQTT error:', err);
});

app.get('/suhu', (req, res) => {
  antares.get('AQWM', 'weather_airQuality_nodeCore_teknik')
    .then(function(response) {
      res.json(response.content.Temp);
    })
    .catch(function(error) {
      res.status(500).json({ error: 'Error fetching data from Antares' });
    });
});

app.get('/data', (req, res) => {
  antares.get('AQWM', 'weather_airQuality_nodeCore_teknik')
    .then(function(response) {
      res.json(response.content);
    })
    .catch(function(error) {
      res.status(500).json({ error: 'Error fetching data from Antares' });
    });
});

app.listen(port, () => {
  console.log(`API server running on http://localhost:${port}`);
});