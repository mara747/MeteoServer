#include <Streaming.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_SHT31.h>

//#define LOG_DEBUG

Adafruit_BMP085 sensorBMP180;
Adafruit_SHT31 sensorSHT30;
HTTPClient httpClient;

float temperatureBMP180 = 0.0f, temperatureSHT30 = 0.0f, pressureBMP180 = 0.0f;
uint8_t humiditySHT30 = 0;
uint16_t altitudeBMP180 = 0;

const char wifiSSID[] = "humbucak";
const char wifiPass[] = "kareltoneni";

String temperatureID = "59dbd72017513c040894d21a";
String pressureID = "59dbd7d917513c040894d21b";
String humidityID = "59dbd81717513c040894d21c";
String altitudeID = "59dbd86517513c040894d21d";

template< typename T > void sendDataToServer(String aID, T aValue);

void setup() {
  Serial.begin(115200);
  Serial << endl << "meteo sender start" << endl;

  if (!sensorBMP180.begin() || !sensorSHT30.begin(0x45)) {
    Serial << "Tlakomer nebo teplomer neodpovida. Zkontroluj zapojeni!" << endl;
    while (1) {}
  }

  getValues();

  WiFi.hostname("meteoDataSender");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPass);
  Serial << endl << "Wi-Fi connecting " << wifiSSID << " ";
  while (WiFi.status() != WL_CONNECTED) {
    Serial << ".";
    delay(500);
  }

#ifdef LOG_DEBUG
  Serial << endl << "Connected. IP: " << WiFi.localIP() << endl;
#endif 

  sendDataToServer(temperatureID,temperatureSHT30);
  sendDataToServer(pressureID,pressureBMP180);
  sendDataToServer(humidityID,humiditySHT30);
  sendDataToServer(altitudeID,altitudeBMP180);

  int sleepSeconds = 60;
  Serial << endl << "Going to sleep for "+String(sleepSeconds)+" seconds" << endl;
  ESP.deepSleep(sleepSeconds * 1000000);
  Serial << endl << "Weaking up!" << endl;
}

void loop() {
  Serial << endl << "Entered the loop()" << endl;
  while(1){};
}

void getValues() {
  pressureBMP180 = sensorBMP180.readPressure() / 100.0f;
  altitudeBMP180 = sensorBMP180.readAltitude();
  temperatureBMP180 = sensorBMP180.readTemperature();
  temperatureSHT30 = sensorSHT30.readTemperature();
  humiditySHT30 = sensorSHT30.readHumidity();

  Serial << "BMP180 - temp:" << temperatureBMP180 << " press:" << pressureBMP180 << " alt:" << altitudeBMP180 << endl;
  Serial << "SHT30 - temp:" << temperatureSHT30 << " humidity:" << humiditySHT30 << endl;
}


template< typename T > void sendDataToServer(String aID, T aValue){
  httpClient.begin("13.95.93.26", 3000, String("/sensors/")+aID);

#ifdef LOG_DEBUG
  Serial << "[HTTP] PUT..." << endl;
#endif
  String temperatureJSON = "{\"value\":"+String(aValue)+String("}");
#ifdef LOG_DEBUG
  Serial << "temperatureJSON=" << temperatureJSON << endl;
#endif
  httpClient.addHeader("Content-Type","application/json");
  int httpCode = httpClient.sendRequest("PUT",temperatureJSON);
  if(httpCode > 0) {
#ifdef LOG_DEBUG
      Serial << "[HTTP] PUT... code: " + String(httpCode,2) << endl;
#endif      
      // file found at server
      if(httpCode == HTTP_CODE_OK) {
          String payload = httpClient.getString();
#ifdef LOG_DEBUG          
          Serial << payload << endl;
#endif          
      }
  } else {
      Serial << "[HTTP] PUT... failed, error: " << httpClient.errorToString(httpCode) << endl;
  }
  httpClient.end();
  }

