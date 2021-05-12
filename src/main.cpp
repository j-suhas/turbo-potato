#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ArduinoJson.h>

#define DHTPIN 4

#define DHTTYPE DHT11  // DHT11 or DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);


const char* ssid = "Your_Wifi_SSID_here";
const char* wifi_password = "Your Wifi Password Here";

const char* mqtt_server = "192.168.43.162";  // IP of the MQTT broker
const char* humidity_topic = "home/data/humidity";
const char* temperature_topic = "home/data/temperature";
const char* json_topic = "home/data/json";
const char* mqtt_username = "pinkPanther"; // MQTT username
const char* mqtt_password = "pinkPanther"; // MQTT password
const char* clientID = "client_data"; // MQTT client ID

unsigned long previousMillis = 0;
unsigned long interval = 30000;

#define lightCheck 32
#define moistCheck 33
#define Light 18
#define Buzz 19
#define Motor 21

float tempC, humidity;
int brightness, moisture;

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); 

void connect_MQTT(){
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);

  Serial.print("WiFi status: ");
  Serial.println(WiFi.status());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}

void setup() {
  pinMode(lightCheck, INPUT);
  pinMode(moistCheck, INPUT);
  pinMode(DHTPIN, INPUT);
  
  pinMode(Light, OUTPUT);
  pinMode(Buzz, OUTPUT);
  pinMode(Motor, OUTPUT);
  digitalWrite(Light, HIGH);
  
  Serial.begin(9600);
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));

  connect_MQTT();  
}


void loop() {
  connect_MQTT();
  Serial.setTimeout(2000);
  
  DynamicJsonDocument doc(256);

  Serial.println("______...______...______");

  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature) || event.temperature > 100) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    tempC =event.temperature;
    doc["field1"] = tempC;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity) || event.relative_humidity > 100) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    humidity=event.relative_humidity;
    doc["field2"] = humidity;
  }

  int moisture = analogRead(moistCheck) ;
  Serial.print("Moisture = ");
  Serial.println(moisture);
  doc["field3"] = moisture;

  int brightness = analogRead(lightCheck);
  Serial.print("Brightness = ");
  Serial.println(brightness);
  doc["field4"] = brightness;
  
  serializeJson(doc, Serial);
  Serial.println();

  brightness > 600 ?
    digitalWrite(Light, HIGH):
    digitalWrite(Light, LOW);
  
  moisture < 50 ?
    digitalWrite(Motor, HIGH):
    digitalWrite(Motor, LOW);
  
  Serial.println(client.state ());
  
  char payload[128];
  size_t payloadSize = serializeJson(doc, payload);

  boolean ret;
  if (client.state () == 0) {
    client.beginPublish(json_topic, payloadSize, ret);
    client.print(payload);
    client.endPublish();
    Serial.println("JSON sent!");
  }
  else {
    Serial.println("JSON failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
    client.beginPublish(json_topic, payloadSize, ret);
    client.print(payload);
    client.endPublish();

    Serial.println("JSON sent!");
   }

  // client.disconnect();  // disconnect from the MQTT broker
  delay(1000*10);       // print new values every 1 Minute
}
