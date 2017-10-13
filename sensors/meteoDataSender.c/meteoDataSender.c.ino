#include <Streaming.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_SHT31.h>

Adafruit_BMP085 sensorBMP180;
Adafruit_SHT31 sensorSHT30;
HTTPClient httpClient;

float temperatureBMP180 = 0.0f, temperatureSHT30 = 0.0f, pressureBMP180 = 0.0f;
uint8_t humiditySHT30 = 0;
uint16_t altitudeBMP180 = 0;

const char wifiSSID[] = "humbucak";
const char wifiPass[] = "kareltoneni";

String server = "104.40.218.233";

String temperatureID = "59e124f1ed2d30079fd5f726";
String pressureID = "59e12529ed2d30079fd5f727";
String humidityID = "59e12560ed2d30079fd5f728";
String altitudeID = "59e1259ded2d30079fd5f729";

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
    Serial << WiFi.status() << endl;
    delay(500);
  }

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
  httpClient.begin(server, 3000, String("/sensors/")+aID);
  String temperatureJSON = "{\"value\":"+String(aValue)+String("}");
  httpClient.addHeader("Content-Type","application/json");
  int httpCode = httpClient.sendRequest("PUT",temperatureJSON);
  if(httpCode > 0) {
    Serial << "[HTTP] PUT... httpCode:" << httpCode << endl;
    if(httpCode == HTTP_CODE_OK) {
      String payload = httpClient.getString();
      Serial << payload << endl;
    }
  } else {
      Serial << "[HTTP] PUT... failed, error: " << httpClient.errorToString(httpCode) << endl;
  }
  httpClient.end();
  }

