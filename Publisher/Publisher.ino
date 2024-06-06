#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); 

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* topic_temp = "uas_iot/kehidupan/temp";
const char* topic_hum = "uas_iot/kehidupan/hum";
const char* topic_aqi = "uas_iot/kehidupan/quality";
const char* topic_aqiValue = "uas_iot/kehidupan/qualityValue";
const char* topic_device = "uas_iot/kehidupan/device";
// const char* topic_pm = "uas_iot/kehidupan/pm";
// const char* topic_ozon = "uas_iot/kehidupan/ozon";
// const char* topic_co = "uas_iot/kehidupan/co";
// const char* topic_no2 = "uas_iot/kehidupan/no2";

int status = WL_IDLE_STATUS;
String temp = "";
String hum = "";
String aqi = "";
int aqiValue = 0;
bool deviceStatus = false;
bool lastTemp = false;
bool lastHum = false;
bool lastQ = false;
bool lastQV = false;

int ledBar[] = {15, 2, 0, 4, 16, 17, 5, 18, 19, 23};
int ledBarOn = 0;

int ledR = 12;
int ledG = 14;
int ledB = 27;

WiFiClient espClient;
PubSubClient client(espClient);

// fungsi agar untuk generate posisi tengah untuk teks pada lcd
int generatePosx(int length){
  return (20 - length) / 2;
}

void displayLedRGB(int RED, int GREEN, int BLUE){
  analogWrite(ledR, RED);
  analogWrite(ledG, GREEN);
  analogWrite(ledB, BLUE);
}

// connect/reconnect to wifi
void InitWiFi(){
  // cek status wifi
  if (WiFi.status() != WL_CONNECTED){ // if not connected
    Serial.print("Connecting to AP . ");
    // connecting to wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    lcd.clear();
    lcd.setCursor(generatePosx(7), 0);
    lcd.print("UAS IOT");
    lcd.setCursor(generatePosx(19), 2);
    lcd.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED){
      Serial.print(". ");
      delay(500);
    }

    Serial.println("Connected to AP");

    lcd.clear();
    lcd.setCursor(generatePosx(7), 0);
    lcd.print("UAS IOT");
    lcd.setCursor(generatePosx(15), 2);
    lcd.print("WiFi  Connected");
    delay(1000);
    lcd.clear(); 
  }
}

// connect/reconnect to mqtt broker
void reconnect() {
  // while client not coneccted to mqtt broker
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection . . . ");
    lcd.clear();
    lcd.setCursor(generatePosx(7), 0);
    lcd.print("UAS IOT");
    lcd.setCursor(generatePosx(19), 2);
    lcd.print("Connecting to MQTT");

    // connecting to mqqt as a subscriber
    if (client.connect("mqtt-subscriber")) {
      Serial.println("connected");
      // subscribe to this topic
      client.subscribe(topic_device);
      client.subscribe(topic_aqi);
      client.subscribe(topic_aqiValue);
      client.subscribe(topic_temp);
      client.subscribe(topic_hum);
      // client.subscribe(topic_ozon);
      // client.subscribe(topic_pm);
      // client.subscribe(topic_co);
      // client.subscribe(topic_no2);
      lcd.clear();
      lcd.setCursor(generatePosx(7), 0);
      lcd.print("UAS IOT");
      lcd.setCursor(generatePosx(15), 2);
      lcd.print("MQTT  Connected");
      delay(1000);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
    lcd.clear();
  }
}

// fungsi yang digunakan untuk memproses jika terdapat pesan baru pada mqtt
void callback(char* topic, byte* payload, unsigned int length) {

  String payloadString;
  for (int i = 0; i < length; i++) {
    payloadString += (char)payload[i];
  }
  Serial.println(payloadString);

  double payloadDouble = payloadString.toDouble();

  if (String(topic) == topic_device){
    if (payloadString == "1"){
      deviceStatus = true;
      lastTemp = true;
      lastHum = true;
      lastQ = true;
      lastQV = true;
    } else {
      deviceStatus = false;
    }
    lcd.clear();
  }

  if (String(topic) == topic_temp){
    if (payloadString != temp){
      temp = payloadString;
      lastTemp = true;
    }
  }
  
  if (String(topic) == topic_hum){
    if (payloadString != hum){
      hum = payloadString;
      lastHum = true;
    }
  }
  
  if (String(topic) == topic_aqiValue){
    if (payloadString.toInt() != aqiValue){
      aqiValue = payloadString.toInt();
      lastQV = true;
    }
  }

  if (String(topic) == topic_aqi){
    if (payloadString != aqi){
      lastQ = true;
      aqi = payloadString;
      if (aqi == "very Bad"){
        ledBarOn = 2;
      } else if (aqi == "Bad"){
        ledBarOn = 4;
      } else if (aqi == "Normal"){
        ledBarOn = 6;
      } else if (aqi == "Good"){
        ledBarOn = 8;
      } else if (aqi == "Very Good"){
        ledBarOn = 10;
      }
    }
  }
}

// persiapan awal
void setup() {
  // inisiasi serial monitor
  Serial.begin(115200);
  // inisiasi led bar 
  for (int i = 0 ; i < 10 ; i++){
    pinMode(ledBar[i], OUTPUT);
  }
  // inisiasi ledRGB
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  // inisiasi lcd i2c
  lcd.init();
  lcd.backlight();
  // inisiasi wifi
  lcd.clear();
  lcd.setCursor(generatePosx(7), 0);
  lcd.print("UAS IOT");
  lcd.setCursor(generatePosx(11), 1);
  lcd.print("Air Quality");
  lcd.setCursor(generatePosx(17), 2);
  lcd.print("Monitoring System");
  lcd.setCursor(generatePosx(7), 3);
  lcd.print("B  2024");
  delay(2000);

  InitWiFi();
  // inisiasi broker mqtt
  client.setServer(mqtt_server, mqtt_port);
  // inisiasi sebagai subscriber
  client.setCallback(callback);

  lcd.clear();
}

void loop() {
  InitWiFi();

  if (!client.connected()){
    reconnect();
  } 

  if (deviceStatus){
    if (lastTemp){
      lcd.setCursor(generatePosx(13), 2);
      lcd.print("Temp : " + temp);
      lcd.write(byte(223));
      lcd.print("C");
      lastTemp = false;
    }

    if (lastHum){
      lcd.setCursor(generatePosx(11), 3);
      lcd.print("Hum : " + hum + "%");
      lastHum = false;
    }

    if (lastQV){
      lcd.setCursor(generatePosx(7), 0);
      lcd.print("AQI  " + String(aqiValue));
      lastQV = false;
    }

    if (lastQ){
      lcd.setCursor(generatePosx(aqi.length()), 1);
      lcd.print(aqi);
      lastQ = false;
    }

    for (int i = 0 ; i < ledBarOn ; i++){
      digitalWrite(ledBar[i], HIGH);
    }

    for (int i = ledBarOn-1 ; i < 10 ; i++){
      digitalWrite(ledBar[i], LOW);
    }

    if (temp != ""){
      double tempD = temp.toDouble();
      if (tempD < 30){
        displayLedRGB(0, 0, 255);
      } else if (tempD <= 33){
        displayLedRGB(0, 255, 0);
      } else if (tempD <= 36){
        displayLedRGB(255, 255, 0);
      } else if (tempD <= 40){
        displayLedRGB(255, 165, 0);
      } else {
        displayLedRGB(255, 0, 0);
      }
    }
  } else {
    lcd.setCursor(generatePosx(11), 1);
    lcd.print("Devices Off");
  }

  client.loop();
  // delay(1000);
}
