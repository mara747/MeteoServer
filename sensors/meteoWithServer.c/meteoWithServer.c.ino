/*
  https://www.zive.cz/clanky/pojdme-programovat-elektroniku-postavime-si-titernou-wi-fi-meteostanici-s-lepsim-teplomerem-nez-netatmo/sc-3-a-188184/default.aspx
  
  Namisto obvykleho vypisu do seriove linky pomoci Serial.print()
  pouziji knihovnu Streaming a format Serial << promenna << promenna atd.
  Streaming stahnete z webu http://arduiniana.org/libraries/streaming/
*/
#include <Streaming.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_SHT31.h>

/*
  Objekty tlakomeru, teplomeru a weboveho serveru,
  ktery se spusti na standardnim portu 80
*/
Adafruit_BMP085 tlakomer;
Adafruit_SHT31 teplomer = Adafruit_SHT31();
ESP8266WebServer server(80);

// Promenne meteorologickych udaju
float teplota0 = 0.0f, teplota1 = 0.0f, tlak = 0.0f;
uint8_t vlhkost = 0;
uint16_t vyska = 0;

// Prihlasovaci udaje k Wi-Fi
const char ssid[] = "humbucak";
const char heslo[] = "kareltoneni";

/*
  Nemenny HTML kod stranky
  ulozeny ve flashove pameti cipu
*/
PROGMEM const char html_hlavicka[] = "<!DOCTYPE html><html><head><title>Meteostanice</title><meta http-equiv=\"refresh\" content=\"70\"><style>html,body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;margin:0;padding:0;display:flex;justify-content:center;align-items:center;width:100%;height:100%;overflow:hidden;background-color:black;}div{font-size:10vw;color:grey;resize:none;overflow:auto;}.value{color:white;font-weight:bolder;}</style></head><body><div>";
PROGMEM const char html_paticka[] = "</div></body>";

/*
  Pomocna promenna casovace, ktery kazdou
  minutu aktualizuje udaje ze senzoru
*/
uint64_t posledniObnova = 0;

// Funkce setup se zpracuje hned po spusteni
void setup() {
  // Nastartuj seriovoou linku na rychlosti 115 200 bps
  Serial.begin(115200);
  // Dvakrat odradkuj seriovou linku a napis uvitani
  Serial << endl << endl;
  Serial << "===============================" << endl;
  Serial << "=== M E T E O S T A N I C E ===" << endl;
  Serial << "===============================" << endl << endl;

  /*
    Pokud se nepodarilo nastartovat tlakomer
    nebo teplomer (I2C adresa 0x45 je napsana na jeho desticce),
    vypis do seriove linky chybove hlaseni a zastav dalsi zpracovavani
  */
  if (!tlakomer.begin() || !teplomer.begin(0x45)) {
    Serial << "Tlakomer nebo teplomer neodpovida. Zkontroluj zapojeni!" << endl;
    while (1) {}
  }

  /*
    Precti hodnoty ze sensoru a vypis
    je do seriove linky
  */
  ziskejHodnoty();

  // Jmeno zarizeni v siti
  WiFi.hostname("meteostanice");
  // Rezim Wi-Fi (sta = station)
  WiFi.mode(WIFI_STA);
  // Zahajeni pripojovani
  WiFi.begin(ssid, heslo);
  Serial << endl << "Pripojuji se k Wi-Fi siti " << ssid << " ";
  // Dokud se nepripojim, vypisuj do seriove linky tecky
  while (WiFi.status() != WL_CONNECTED) {
    Serial << ".";
    delay(500);
  }

  // Vypis do seriove linky pridelenou IP adresu
  Serial << endl << "Meteostanice ma IP adresu " << WiFi.localIP() << endl;

  /*
    Nastaveni weboveho serveru. Zareaguje, pokud zachyti pozadavek GET /,
    tedy pozadavek na korenovy adresar. Stejne tak bych mohl nastavit
    reakce na fiktivny stranky, treba server.on("/index.html" a tak dale.
  */
  server.on("/", []() {
    // Ziskej parametr URL jmenem api, tedy /?api=...
    String api = server.arg("api");
    // Preved jej na mala pismena
    api.toLowerCase();
    /*
      Pokud URL parametr obsahuje text 'json',
      posli klientovi HTTP kod 200 a JSON s hodnotami ze senzoru
      a vypis do seriove linky IP adresu klienta
    */
    if (api == "json") {
      server.send(200, "application/json", "{\"tlak\":" + String(tlak, 2) + ",\"teplota\":" + String(teplota1, 2) + ",\"vlhkost\":" + String(vlhkost) + "}");
      Serial << "HTTP GET: Klient si stahl JSON data" << endl << endl;
    }
    /*
      Pokud URL parametr obsahuje neco jineho, nebo
      neexistuje, posli klientovi HTTP kod 200 a HTML stranku
    */
    else {
      server.send(200, "text/html", String(html_hlavicka) + "Teplota: <span class=\"value\">" + String(teplota1, 2) + " &#x00B0;C</span><br/>Tlak: <span class=\"value\">" + String(tlak, 2) + " hPa</span><br/>Vlhkost: <span class=\"value\">" + String(vlhkost) + " %</span>" + String(html_paticka));
      Serial << "HTTP GET: Klient si stahl HTML stranku" << endl << endl;
    }
  });

  // Nastavili jsme webovy server a ted ho uz muzeme spustit
  server.begin();
  Serial << "Webovy sever je spusteny a ceka!" << endl;
}

// Funkce loop se po spusteni opakuje stale dokola
void loop() {
  // Zpracuj pozadavky weboveho serveru
  server.handleClient();
  /*
    Kazdych sedesat sekund precti hodnoty ze senzoru.
    Nesmim pouzit delay(60000), protoze by se na 60s
    zastavil cely program, a kdyby v tuto chvili nekdo
    navstivil server, ten by na pozadavek zareagoval az
    po minute.

    Funkci ziskejHodnoty bych take mohl zavolat az pri
    pozadavku na webovy server, takto ale mohu prubezne zaznamenavat
    treba rekordni hodnoty aj.
  */
  if ((millis() - posledniObnova) > 5000) {
    ziskejHodnoty();
    posledniObnova = millis();
  }
}

// Funkce pro precteni hodnot ze senzoru do promennych
void ziskejHodnoty() {
  // Prepocet tlaku v Pa na hPa
  tlak = tlakomer.readPressure() / 100.0f;
  // Vypocet velmi hrube nadmorske vysky podle tlaku
  vyska = tlakomer.readAltitude();
  // Teplota z tlakomeru
  teplota0 = tlakomer.readTemperature();
  // Presnejsi teplota z teplomeru
  teplota1 = teplomer.readTemperature();
  // Vlhkost vzduchu
  vlhkost = teplomer.readHumidity();

  // Vypsani cerstvych hodnot do seriove linky
  Serial << "System bezi: " << millis() << " ms" << endl;
  Serial << "Volna pamet heap: " << ESP.getFreeHeap() << " B" << endl << endl;

  Serial << "Udaje z tlakomeru BMP180" << endl;
  Serial << "Atmosfericky tlak: " << tlak << "hPa" << endl;
  Serial << "Teplota vzduchu: " << teplota0 << " C" << endl;
  Serial << "Nadmorska vyska: " << vyska << " m n.m." << endl << endl;

  Serial << "Udaje z teplomeru a vlhkomeru SHT30" << endl;
  Serial << "Teplota vzduchu: " << teplota1 << " C" << endl;
  Serial << "Relativni vlhkost vzduchu: " << vlhkost << " %" << endl << endl << endl;
}
