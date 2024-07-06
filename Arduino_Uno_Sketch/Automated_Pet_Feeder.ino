#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "HX711.h"
#include <Stepper.h>
#include <TimeLib.h>

#define STEPPER_PIN_1 8
#define STEPPER_PIN_2 9
#define STEPPER_PIN_3 10
#define STEPPER_PIN_4 11
const int stepsPerRevolution = 2048;
Stepper mojStepMotor(stepsPerRevolution, 8, 10, 9, 11);
int step_number = 0;

SoftwareSerial espSerial(7, 6);
String jsonData;

HX711 vagaHrana;
HX711 vagaVoda;
const int DT_VHrana = A0;
const int SCK_VHrana = A1;
const int DT_VVoda = A2;
const int SCK_VVoda = A3;

const int LEDB = 5;
int ledBState = LOW;
const int LEDR = 4;
const int taster = 13;
int tasterState = 0;

unsigned long poslednjeMerenjeVode = 0;
const unsigned long intervalMerenjaVode = 30 * 1000; // 2 minuta u milisekundama
const float maxTezinaVode = 200.0; // Maksimalna težina vode u gramima, prilagodi prema stvarnim vrednostima
const float granicaVode = maxTezinaVode / 3.0;
const int relejPin = 2;
bool relejUkljucen = false;

//Funkcija za pracenje odgovora sa ESP8622
boolean echoFind(String keyword) {
  byte current_char = 0;
  byte keyword_length = keyword.length();
  long deadline = millis() + 5000;

  while (millis() < deadline) {
    if (espSerial.available()) {
      char ch = espSerial.read();
      Serial.write(ch);

      if (ch == keyword[current_char]) {
        if (++current_char == keyword_length) {
          Serial.println();
          return true;
        }
      } else {
        current_char = 0;
      }
    }
  }
  return false;
}

//Funkcija za slanje AT komande na ESP8622
boolean SendCommand(String cmd, String ack) {
  espSerial.println(cmd);
  return echoFind(ack);
}

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  setTime(0,0,0,1,1,2024);
  if (SendCommand("AT+RST", "ready")) {
    delay(5000);
    Serial.println("ESP8266 reset successfully");
    blinkBlue(3,500);
  } else {
    Serial.println("Failed to reset ESP8266");
  }
  SendCommand(" =1", "OK");
  delay(500);

  pinMode(relejPin, OUTPUT);
  digitalWrite(relejPin, HIGH);

  pinMode(STEPPER_PIN_1, OUTPUT);
  pinMode(STEPPER_PIN_2, OUTPUT);
  pinMode(STEPPER_PIN_3, OUTPUT);
  pinMode(STEPPER_PIN_4, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(taster, INPUT);
  
  if (SendCommand("AT+CWJAP=\"Redmi Note 9\",\"ilija1999\"", "OK")) {
    delay(500);
    Serial.println("Connected to WiFi");
    blinkBlue(5,200);
    SendCommand("AT+CIFSR", "OK");
    delay(500);
    SendCommand("AT+CIPMUX=1", "OK");
    delay(500);
    SendCommand("AT+CIPSERVER=1,80", "OK");
    delay(500);

    vagaHrana.begin(DT_VHrana, SCK_VHrana);
    delay(500);
    vagaVoda.begin(DT_VVoda, SCK_VVoda);
    delay(500);
    if((vagaHrana.is_ready())&&(vagaVoda.is_ready())){
      delay(1000);
      vagaHrana.tare();
      delay(3000);
      vagaVoda.tare();
      delay(3000);
      Serial.println("Vage su spremne!");
      ledBState = HIGH;
      digitalWrite(LEDB,ledBState);
    }
    else{
      digitalWrite(LEDR, HIGH);
      Serial.println("Vage nisu spremne!");
    }
    
  } else {
    Serial.println("Failed to connect to WiFi");
    blinkRed(5,500);
  }
}

bool motorRotiran = false;
unsigned long motorRotationStartTime = 0;
bool podaciPrimljeni = false;
bool prviObrok;
int kolicina;
int vrednostDugmeta;
unsigned long vremePoslednjegObroka = 0;

void checkEsp() {
  if (espSerial.available() > 0) {
    String data = espSerial.readString();
    Serial.println("Received data:");
    Serial.println(data);

    if (data.indexOf("{") != -1 && data.lastIndexOf("}") != -1) {
      int startIndex = data.indexOf("{");
      int endIndex = data.lastIndexOf("}");
      jsonData = data.substring(startIndex, endIndex + 1);
      
      Serial.println("Linija sa JSON podacima: ");
      Serial.println(jsonData);

      processJsonData(jsonData, kolicina, vrednostDugmeta);
      
      Serial.print("Količina: ");
      Serial.println(String(kolicina));

      Serial.print("Vrednost dugmeta: ");
      Serial.println(vrednostDugmeta);

      podaciPrimljeni = true;
      prviObrok = false;
    }
  }
}

void proveriVodu() {
  unsigned long trenutnoVreme = millis();
  if (trenutnoVreme - poslednjeMerenjeVode >= intervalMerenjaVode) {
    poslednjeMerenjeVode = trenutnoVreme;
    float tezinaVode = getWeight(&vagaVoda);
    tasterState = digitalRead(taster);
    delay(500);
    Serial.println("Težina vode: " + String(tezinaVode) + "g");
    if(tezinaVode < granicaVode && tasterState == LOW){
      digitalWrite(LEDR,LOW);
      while (1) {
        tezinaVode = getWeight(&vagaVoda);
        delay(500);
        Serial.println(String(tezinaVode) + "g");
        if (tezinaVode < granicaVode && !relejUkljucen) {
          digitalWrite(relejPin, LOW); // Uključi relej
          relejUkljucen = true;
        }
        if (tezinaVode >= maxTezinaVode && relejUkljucen) {
          digitalWrite(relejPin, HIGH); // Isključi relej
          relejUkljucen = false;
          Serial.println("Relej isključen, pumpa staje...");
          break;
        }
      }
    }
    else{
      digitalWrite(LEDR, HIGH);
    }
  }
}
void servirajPrviObrok(){
  // Merenje težine sa senzora i rotacija motora samo ako su podaci primljeni
  if (podaciPrimljeni && !prviObrok) {
     while (1) {
        float tezina_VH = getWeight(&vagaHrana);
        Serial.println(String(tezina_VH) + "g");
        if (tezina_VH < kolicina && !motorRotiran) {
          rotirajMotor();
        }
        if (tezina_VH >= kolicina) {
          motorRotiran = false;
          prviObrok = true;
          vremePoslednjegObroka = now();
          delay(1000);
          break;
        }
     }
  }
}
int interval;
void servirajNaredniObrok(){
  if (prviObrok) {
    unsigned long trenutnoVreme = now();
    unsigned long vremeOdPoslednjegObroka = trenutnoVreme - vremePoslednjegObroka;

    // Postavljanje intervala na osnovu vrednostiDugmeta
    switch (vrednostDugmeta) {
      case 2:
        interval = 2 * 60; // 2 minuta u sekundama
        break;
      case 8:
        interval = 8 * 3600; // 8 sati u sekundama
        break;
      case 12:
        interval = 12 * 3600; // 12 sati u sekundama
        break;
      case 24:
        interval = 24 * 3600; // 24 sata u sekundama
        break;
      default:
        interval = 8 * 3600;
        break;
    }
    if (vremeOdPoslednjegObroka >= interval) {
      Serial.println("Vreme za sledeći obrok!");
      float tezina_VH = getWeight(&vagaHrana);
      if(tezina_VH <= kolicina)
      {
        while (1) {
          tezina_VH = getWeight(&vagaHrana);
          Serial.println(String(tezina_VH) + "g");
          if (tezina_VH < kolicina && !motorRotiran) {
            rotirajMotor();
          }
          if (tezina_VH >= kolicina) {
            motorRotiran = false;
            vremePoslednjegObroka = now();
            delay(1000);
            break;
          }
        }
      }else{
        vremePoslednjegObroka = now();
      }
    }
  }
}
void loop() {
  tasterState = digitalRead(taster);
  if (tasterState == HIGH) {
    digitalWrite(LEDR, HIGH);
  } else {
    digitalWrite(LEDR, LOW);
  }
  proveriVodu(); //Provera i dopuna vode
  servirajPrviObrok(); //Serviranje prvog obroka nakon prinjama informacija
  servirajNaredniObrok(); //Izracunavanje vremena za sledeci obrok i serviranje
  checkEsp(); //Osluskivanje da li su podaci sa aplikacije stigli
}

// Funkcija za tumacenje JSON fajla koji stigne sa beka na ESP8622 i izvlacenje vrednosti svojstva kolicina, vrednostDugmeta i selectedTime iz JSON-a
void processJsonData(const String& jsonData, int& kolicina, int& vrednostDugmeta) {
  // Dinamičko alociraje memorije za JSON dokument.
  const size_t capacity = JSON_OBJECT_SIZE(2) + 50;
  DynamicJsonDocument doc(capacity);

  // Konvertovanje u JSON
  DeserializationError error = deserializeJson(doc, jsonData);

  // Provera moguce greske
  if (error) {
    Serial.print("Greška prilikom tumačenja JSON podataka: ");
    Serial.println(error.c_str());
    return false;
  }

  // JSON podaci su uspešno tumačeni
  Serial.println("JSON podaci uspešno tumačeni.");

  // Izvlacenje svojstva
  kolicina = doc["k"].as<int>();
  vrednostDugmeta = doc["v"].as<int>();
}


// Funkcija iscitavanje sa vage
float getWeight(HX711* scale) {
  // Čitanje i konvertovanje podataka iz vagi koristeći kalibraciju
  return scale->get_units(10) / 1000.0;
}

//Funkcija za rotaciju motora
void rotirajMotor() {
  mojStepMotor.setSpeed(15);
  mojStepMotor.step(stepsPerRevolution);
}

void blinkBlue(int count, int duration) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LEDB, HIGH);
    delay(duration);
    digitalWrite(LEDB, LOW);
    delay(duration);
  }
}

void blinkRed(int count, int duration) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LEDR, HIGH);
    delay(duration);
    digitalWrite(LEDR, LOW);
    delay(duration);
  }
}
