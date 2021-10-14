#include <M5Stack.h>
// Incluimos librería
#include <DHT.h>

#include "WiFi.h"
#include "AsyncUDP.h"
#include <TimeLib.h>
#include <ArduinoJson.h>
const char * ssid = "TP-LINK_6CAE";
const char * password = "41422915";
AsyncUDP udp;
StaticJsonDocument<200> jsonBuffer; //tamaño maximo de los datos

// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 26
#define pinLEDTemperature 23
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11

// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

const int pinMagnetic = 2;
const int pinLEDMagnetic = 5;

int preview = LOW;

void setup() {

  // Inicializamos comunicación serie
  //Serial.begin(115200);

  // Comenzamos el sensor DHT
  dht.begin();

  M5.begin();  //Init M5Core.  初始化 M5Core
  M5.Power.begin(); //Init Power module.  初始化电源模块
  /* Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project */

  //configurar pin como entrada con resistencia pull-up interna
  pinMode(pinMagnetic, INPUT_PULLUP);
  pinMode(pinLEDMagnetic, OUTPUT);
  pinMode(pinLEDTemperature, OUTPUT);

  setTime (9, 15, 0, 7, 10, 2018); //hora minuto segundo dia mes año
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //Serial.println("WiFi Failed");
    while (1) {
      delay(1000);
    }
  }
  if (udp.listen(1234)) {
    //Serial.print("UDP Listening on IP: ");
    //Serial.println(WiFi.localIP());
    udp.onPacket([](AsyncUDPPacket packet) {
      //Serial.write(packet.data(), packet.length());
      //Serial.println();
    });
  }

}

void loop() {
  //Codigo para sensor de temperatura


  delay(2000);
  //digitalWrº
  ite(pinLEDTemperature, HIGH);
  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
  // Leemos la temperatura en grados Fahreheit
  float f = dht.readTemperature(true);


  /*if (isnan(h) || isnan(t) || isnan(f)) {
    digitalWrite(pinLEDTemperature, HIGH);
    //Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
    }*/

  if (t > 30) {
    digitalWrite(pinLEDTemperature, HIGH);
    M5.Speaker.tone(900, 1000);
    //Serial.println("Error obteniendo los datos del sensor DHT11");
    //return;
  } else {
    digitalWrite(pinLEDTemperature, LOW);
    M5.Speaker.end();
  }



  float hif = dht.computeHeatIndex(f, h);
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(t, h, false);

  /*Serial.print("Humedad: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t");
    Serial.print("Índice de calor: ");
    Serial.print(hic);
    Serial.print(" *C ");
    Serial.print(hif);
    Serial.println(" *F");*/
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println(t);
  jsonBuffer["Temperatura"] = t;

  int value = digitalRead(pinMagnetic);

  if (value == LOW && value != preview) {

    //M5.Lcd.setCursor(0,0);
    M5.Lcd.print("Cerrado");
    jsonBuffer["Puerta"] = "Cerrada";
    digitalWrite(pinLEDMagnetic, LOW);
    M5.Lcd.clear();


  } else if (value == HIGH && value != preview) {

    //M5.Lcd.setCursor(0,0);
    M5.Lcd.print("Abierto");
    jsonBuffer["Puerta"] = "Abierta";
    digitalWrite(pinLEDMagnetic, HIGH);
    M5.Lcd.clear();

  }
  preview = value;
  //delay(1000);
  char texto[200];
  serializeJson(jsonBuffer, texto); //paso del objeto “jsonbuffer" a texto para
  //transmitirlo
  udp.broadcastTo(texto, 1234); //se envía por el puerto 1234 el JSON
  delay(2000);
}
