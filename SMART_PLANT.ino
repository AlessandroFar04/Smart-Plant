#define BLYNK_AUTH_TOKEN "8O-w6CUUPYsijpyTujfliEGM0OyrcZPl"
#define BLYNK_TEMPLATE_ID "TMPL4jbib8RiK"
#define BLYNK_TEMPLATE_NAME "SMART PLANT"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HCSR04.h>
#include <DHT.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// definizione costanti, costruzione oggetti e timers

const int LDR = 34;
UltraSonicDistanceSensor distanceSensor(19, 21);
DHT dht(25, DHT22);

const float empty = 29.10;     // serbatoio VUOTO
const float full = 19.84;      // serbatoio PIENO

const int maxLuce = 850;
const int minLuce = 0;

BlynkTimer timer;
WidgetRTC rtc;        // per avere ora da Blynk


// connessione al cloud blynk e ripartizione delle fasce temporali

void setup() {
  // pilotaggio dht22 ON al setup
  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);
  delay(100);
  dht.begin();


  WiFi.begin("YOURSSID", "SSIDPASSWORD");

  unsigned long startAttempt = millis();
  const unsigned long timeout = 10000; // 10 secondi

  // Attesa connessione WiFi con timeout
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttempt > timeout) {
      softwareReset();
    }
    delay(100);
  }

  // Configurazione Blynk (NON bloccante)
  Blynk.config(
    BLYNK_AUTH_TOKEN,
    "blynk.cloud",
    80
  );

  startAttempt = millis();

  // Attesa connessione Blynk con timeout
  while (!Blynk.connected()) {
    Blynk.run();

    if (millis() - startAttempt > timeout) {
      softwareReset();
    }

    delay(100);
  }

  
  // setting di tutti i timer di attivazione delle funzioni

  timer.setInterval(360000L, leggiDHT22);                     // 6 minuti
  timer.setInterval(420000L, leggiLuce);                      // 7 minuti

  timer.setInterval(540000L, leggiUmiditaTerreno);            // 9 minuti
  timer.setInterval(1080000L, leggiSerbatoio);                // 18 minuti
  
  timer.setInterval(86400000L, innaffia);                     // 24 ore

}

BLYNK_CONNECTED() {
  rtc.begin(); // sincronizza orario da server Blynk
}

void softwareReset(){
  ESP.restart();
}

void innaffia(){
  if(leggiUmiditaTerreno() < 41){
    // creo stringa con timestamp + codice errore
    char buf[64];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Accensione pompa...",
             day(), month(), hour(), minute());

    // invio al widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);
    digitalWrite(12, HIGH);
    delay(5000);                // visto quanta poca acqua gli arriva, più o meno equivale a 3/4 di bicchiere d'acqua
    digitalWrite(12, LOW);
  }
}

void leggiDHT22() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    // creo stringa con timestamp + codice errore
    char buf[64];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Errore DHT22, reset...",
             day(), month(), hour(), minute());

    // invio al widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);

    // avvio procedura di reset dell'alimentazione
    digitalWrite(23, LOW);
    delay(1000);
    digitalWrite(23, HIGH);

    return;
  }
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
}

int leggiUmiditaTerreno(){

  // valore massimo rilevato empiricamente da moisture sensor: 2500 e minimo 980

  int raw = analogRead(32);
  
  // SENSORE NON FUNZIONANTE
  raw = map(raw, 0, 250, 0, 100);
  raw = constrain(raw, 0, 100);

  // SENSORE FUNZIONANTE  
  /*raw = map(raw, 980, 2510, 0, 100);
  raw = constrain(raw, 0, 100);
  raw = (raw - 100) * (-1);*/
  
  Blynk.virtualWrite(V2, raw);

  if(raw < 36){
    // creo stringa con timestamp + codice errore
    char buf[100];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Terreno troppo asciutto! Controlla la pompa!",
             day(), month(), hour(), minute());

    // invio al widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);
    Blynk.logEvent("terreno_asciutto", "terra troppo secca, controlla che la pompa funzioni correttamente");
  }
  
  return raw;
}

void leggiLuce(){
  int luce = analogRead(LDR);

  float livello = ((float)(minLuce - luce) / (minLuce - maxLuce)) * 100;
  livello = constrain(livello, 0, 100);
  Blynk.virtualWrite(V3, livello);
}

void leggiSerbatoio(){

  float livello;

  float distanza = distanceSensor.measureDistanceCm();

  if(distanza < 0){
    char buf[70];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Lettura livello serbatoio errata!",
             day(), month(), hour(), minute());

    // invio al widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);
    return;
  }
  // calcolo il rapporto e poi la percentuale
  livello = ((empty - distanza) / (empty - full)) * 100;
  livello = constrain(livello, 0, 100);
  Blynk.virtualWrite(V4, livello);

  if(livello < 25){
      // creo stringa con timestamp + codice errore
    char buf[70];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Livello serbatoio basso!",
             day(), month(), hour(), minute());

    // invio al widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);
    Blynk.logEvent("serbatoio_basso", "Livello dell'acqua basso, rabbocca!");
  }
  
}

// FUNZIONI DI CALLBACK

BLYNK_WRITE(V5) {
  String cmd = param.asStr();   // testo ricevuto dal terminale

  if (cmd.equalsIgnoreCase("DIST")) {
    float d = distanceSensor.measureDistanceCm();

    if (d < 0) {
      Blynk.virtualWrite(V5, "Errore: nessun echo / timeout\n");
    } else {
      char buf[40];
      snprintf(buf, sizeof(buf), "Distanza: %.2f cm\n", d);
      Blynk.virtualWrite(V5, buf);
    }
    return;
  }

  if(cmd.equalsIgnoreCase("LUCE")){
    Blynk.virtualWrite(V5, analogRead(LDR));
    return;
  }

  if(cmd.equalsIgnoreCase("TERRA")){
    Blynk.virtualWrite(V5, analogRead(32));
    return;
  }

  if(cmd.equalsIgnoreCase("DHT")){
    char buf[40];
      snprintf(buf, sizeof(buf), "Temp: %.2f | hum: %.2f", dht.readTemperature(), dht.readHumidity());
      
      if(isnan(dht.readTemperature()) || isnan(dht.readHumidity())) {
        strcat(buf, " | reset...");
        digitalWrite(23, LOW);
        delay(1000);
        digitalWrite(23, HIGH);
      }
      Blynk.virtualWrite(V5, buf);
      return;
  }

  char buf[40];
  snprintf(buf, sizeof(buf), "Comando non riconosciuto.");
  Blynk.virtualWrite(V5, buf);
}


void loop() {
  Blynk.run();
  timer.run();

}
