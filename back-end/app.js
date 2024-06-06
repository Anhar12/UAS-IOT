const express = require('express');
const mqtt = require("mqtt");
const antares = require('antares-http');
const app = express();

const port = 3000;

antares.setAccessKey('ee8d16c4466b58e1:3b8d814324c84c89');

const mqttClient = mqtt.connect('mqtt://broker.emqx.io');

function getAirQuality(ozon, pm25, co, no2) {
  const member = []; 

  if (ozon <= 0.059) {
    member[0] = 1;
  } else if (ozon <= 0.095) {
    member[0] = 0.5;
  } else {
    member[0] = 0;
  }

  if (pm25 <= 12) {
    member[1] = 1;
  } else if (pm25 <= 35) {
    member[1] = 0.5;
  } else {
    member[1] = 0;
  }

  if (co <= 4.4) {
    member[2] = 1;
  } else if (co <= 9.4) {
    member[2] = 0.5;
  } else {
    member[2] = 0;
  }

  if (no2 <= 0.053) {
    member[3] = 1;
  } else if (no2 <= 0.1) {
    member[3] = 0.5;
  } else {
    member[3] = 0;
  }

  let avg = 0.0;
  for (let i = 0; i < member.length; i++) {
    avg += member[i];
  }
  avg = (avg / member.length) * 100;
  if (avg >= 100){
    avg = 99;
  }
  return parseInt(avg);
}

function getAllData() {
  antares.get('AQWM', 'weather_airQuality_nodeCore_teknik')
    .then(function(response) {
      const data = response.content;
      mqttClient.publish('uas_iot/kehidupan/temp', data.Temp.toFixed(2).toString());
      mqttClient.publish('uas_iot/kehidupan/hum', data.Hum.toFixed(2).toString());
      mqttClient.publish('uas_iot/kehidupan/ozon', data.Ozon.toFixed(2).toString());
      mqttClient.publish('uas_iot/kehidupan/pm', data['PM2.5'].toFixed(2).toString());
      mqttClient.publish('uas_iot/kehidupan/co', data.CO.toFixed(2).toString());
      mqttClient.publish('uas_iot/kehidupan/no2', data.NO2.toFixed(2).toString());
      mqttClient.publish('uas_iot/kehidupan/qualityValue', getAirQuality(data.Ozon, data['PM2.5'], data.CO, data.NO2).toString());
      
      let quality;
      if (getAirQuality(data.Ozon, data['PM2.5'], data.CO, data.NO2) >= 67){
        quality = "Good";
      } else if (getAirQuality(data.Ozon, data['PM2.5'], data.CO, data.NO2) >= 33) {
        quality = "Normal";
      } else {
        quality = "Bad";
      }
      mqttClient.publish('uas_iot/kehidupan/quality', quality);
    })
    .catch(function(error) {
      console.error('Error fetching data from Antares:', error);
    });
}

mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
});

mqttClient.on('error', (err) => {
  console.error('MQTT error:', err);
});

setInterval(getAllData, 3000);

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
