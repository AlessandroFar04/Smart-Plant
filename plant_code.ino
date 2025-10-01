// secret information for Blynk cloud communication

#define BLYNK_AUTH_TOKEN ""
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <HCSR04.h>
#include <DHT.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// constant definitions, building objects and internal timers

const int LDR = 34;
UltraSonicDistanceSensor distanceSensor(19, 21);
DHT dht(25, DHT22);

const float empty = 29.10;     // Empty water tank measurement
const float full = 19.84;      // Full water tank measurement

const int maxLuce = 850;
const int minLuce = 0;

BlynkTimer timer;
WidgetRTC rtc;        // to sinchronize local date when sending debug messages


// connection setup and timer definitions

void setup() {
  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, "YOUR SSID", "YOUR PSSW", "blynk.cloud", 80);
  

  timer.setInterval(360000L, leggiDHT22);                     // 6 minutes
  timer.setInterval(420000L, leggiLuce);                      // 7 minutes

  timer.setInterval(540000L, leggiUmiditaTerreno);            // 9 minutes
  timer.setInterval(1080000L, leggiSerbatoio);                // 18 minutes
  
  timer.setInterval(86400000L, innaffia);                     // 24 hours

}

BLYNK_CONNECTED() {
  rtc.begin();
}

void innaffia(){
  if(leggiUmiditaTerreno() < 41){
    
    char buf[64];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Accensione pompa...",
             day(), month(), hour(), minute());

    // sending to widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);
    digitalWrite(12, HIGH);
    delay(5000);                
    digitalWrite(12, LOW);
  }
}

void leggiDHT22() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    
    char buf[64];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Errore DHT22",
             day(), month(), hour(), minute());

    // sending to widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);
    return;
  }
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
}

int leggiUmiditaTerreno(){

  // min-max measurements taken during tests

  int raw = analogRead(32);
  raw = map(raw, 980, 2510, 0, 100);
  raw = constrain(raw, 0, 100);
  raw = (raw - 100) * (-1);
  
  Blynk.virtualWrite(V2, raw);

  if(raw < 36){
    
    char buf[100];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Terreno troppo asciutto! Controlla la pompa!",
             day(), month(), hour(), minute());

    // sending to widget Terminal (V5)
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

    // sending to widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);
    return;
  }
  
  livello = ((empty - distanza) / (empty - full)) * 100;
  livello = constrain(livello, 0, 100);
  Blynk.virtualWrite(V4, livello);

  if(livello < 25){
      
    char buf[70];
    snprintf(buf, sizeof(buf),
             "[%02d/%02d %02d:%02d] Livello serbatoio basso!",
             day(), month(), hour(), minute());

    // sending to widget Terminal (V5)
    Blynk.virtualWrite(V5, buf);
    Blynk.logEvent("serbatoio_basso", "Livello dell'acqua basso, rabbocca!");
  }
  
}

// CALLBACK FUNCTIONS

BLYNK_WRITE(V5) {
  String cmd = param.asStr();   // received from widget terminal (V5)

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
