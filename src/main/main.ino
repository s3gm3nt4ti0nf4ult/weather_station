#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "secrets.h"

ESP8266WebServer server(80); // port 80
Adafruit_BME280 bme;
IPAddress ws_ip(192, 168,0, 77);
IPAddress gateway(192,168,0,1);
IPAddress dns(8,8,8,8);
IPAddress subnet(255,255,255,0);

void connect_to_wifi() {
  WiFi.disconnect();
  Serial.println("[DEBUG] Connecting to ");
  Serial.println(ssid);
  WiFi.config(ws_ip, dns, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print("#");
  }
  Serial.println("[DEBUG] WiFi Connected!");
  Serial.println("[DEBUG] IP address: " );
  Serial.println(WiFi.localIP());
}
void setup() {
  Serial.begin(115200); // inicjalizacja połączenia portem szeregowym
  bme.begin(0x76); // adres czujnika BME i2c
  bme.setSampling(
        Adafruit_BME280::MODE_FORCED,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::SAMPLING_X1,
        Adafruit_BME280::FILTER_OFF);
  connect_to_wifi();
  
  server.on("/", handle_request);
  server.onNotFound(handle_not_found);
  server.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[DEBUG] Need to reconnect!");
    connect_to_wifi();
  }
  server.handleClient();
}

void handle_request() {
  Serial.println("Handling weather request!");
  char buff[152];
  float temp, pres, hum;
  temp = bme.readTemperature();
  pres = bme.readPressure() / 100.0F;
  hum = bme.readHumidity();
  snprintf(buff, 152, "<h2>Temperature:</h2> %.2f °C</br><h2>Atmospheric pressure:</h2> %.0f hPa</br><h2>Humidity:</h2> %.2f", temp, pres, hum);
  Serial.println(buff);
  
  String content = "<DOCTYPE html>\n";
  content += "<html>\n";
  content += "<head>\n";
  content += "<title>Pogoda</title>\n";
  content += "<meta charset=\"UTF-8\">\n";
  content += "</head>";
  content += "<body>\n";
  content += buff;
  content += " %</br>\n";
  content += "</body>\n";
  content += "</html>\n";
  server.send(200, "text/html", content);
}

void handle_not_found() {
  Serial.println("Handling not found");
  server.send(404, "text/plain", "Not found");
}
