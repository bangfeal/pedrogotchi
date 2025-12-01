#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "TAMA.h"
#include "TAMA_NOCHE.h"

#define TFT_MOSI 17
#define TFT_SCLK 18
#define TFT_DC   16
#define TFT_CS   15
#define TFT_RST  4

#define RED_BUTTON 40
#define BLUE_BUTTON 41
#define WHITE_BUTTON 42

#define PHOTO_RESISTOR 9
#define BUZZER 35
#define BUZZ_FREQ 3500

#define MAX_OPT 8

volatile bool redPress = false;
volatile bool bluePress = false;
volatile bool whitePress = false;

void IRAM_ATTR redISR()   { redPress = true; }
void IRAM_ATTR blueISR()  { bluePress = true; }
void IRAM_ATTR whiteISR() { whitePress = true; }

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

int happiness = 100;
int hunger = 100;
int cold = 50;

unsigned long lastDecay = 0;
const unsigned long decayInterval = 5000;
const int decayAmount = 5;

int status = 0;
int choice = 1;

uint16_t backgroundColor = ST77XX_BLACK;
uint16_t textColor = ST77XX_WHITE;

const char* ssid = "placeholder";
const char* password = "placeholder";
const char* weatherURL = "http://api.openweathermap.org/data/2.5/weather?q=Madrid&appid=placeholder&units=metric";

void incChoice(int &c) {
  c++;
  if (c > MAX_OPT) c = 1;
}

void beepBoop(bool longBeep) {
  int duration = longBeep ? 120 : 50;
  tone(BUZZER, BUZZ_FREQ);
  delay(duration);
  noTone(BUZZER);
}

struct Rect { int x, y, w, h; };
Rect choiceRects[MAX_OPT] = {
  {0, 0, 60, 60},
  {60, 0, 60, 60}, 
  {120, 0, 60, 60}, 
  {180, 0, 60, 60}, 
  {0, 180, 60, 60}, 
  {60, 180, 60, 60}, 
  {120, 180, 60, 60}, 
  {180, 180, 60, 60}, 
};

void drawIcon(int idx) {
  if (backgroundColor == ST77XX_WHITE ) tft.drawRGBBitmap(0, 0, TAMA, 240, 240);
  else tft.drawRGBBitmap(0, 0, TAMA_NOCHE, 240, 240);

  if(idx >= 1 && idx <= MAX_OPT) {
    Rect r = choiceRects[idx-1];
    tft.drawRect(r.x, r.y, r.w, r.h, ST77XX_YELLOW); 
  }
}

void drawStats() {
  tft.fillScreen(backgroundColor);
  tft.setTextColor(textColor);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.print("Felicidad:");
  tft.fillRect(120, 10, happiness, 10, ST77XX_GREEN);

  tft.setCursor(10, 30);
  tft.print("Hambre:");
  tft.fillRect(120, 30, hunger, 10, ST77XX_RED);

  tft.setCursor(10, 50);
  tft.print("Frio:");
  tft.fillRect(120, 50, cold, 10, ST77XX_BLUE);
}

void handleFeeding() {
  switch(choice) {
    case 1: 
      hunger = min(100, hunger + 20);
      happiness = min(100, happiness + 10);
      break;
    case 2:
      hunger = min(100, hunger + 10);
      happiness = min(100, happiness + 20);
      break;
    case 3: 
      happiness = min(100, happiness + 15);
      cold = max(0, cold - 10);
      break;
  }
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connectando a WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado");
}

float fetchTemperature() {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(weatherURL);
    int httpCode = http.GET();
    if(httpCode == 200) {
      String payload = http.getString();
      int idx = payload.indexOf("\"temp\":");
      if(idx != -1) {
        String t = payload.substring(idx+7, payload.indexOf(",", idx));
        Serial.println(t);
        return t.toFloat();
      }
    }
    http.end();
  }
  return 20.0;
}

unsigned long lastWeatherFetch = 0;
const unsigned long weatherInterval = 600000; 

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  pinMode(PHOTO_RESISTOR, ANALOG);
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(WHITE_BUTTON, INPUT_PULLUP);

  attachInterrupt(RED_BUTTON, &redISR, FALLING);
  attachInterrupt(BLUE_BUTTON, &blueISR, FALLING);
  attachInterrupt(WHITE_BUTTON, &whiteISR, FALLING);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 240);
  tft.setRotation(0);

  drawIcon(choice);
  connectWiFi();
}

void loop() {
  if(redPress) {
    beepBoop(false);
    if(status == 0) incChoice(choice);
    redPress = false;
  }

  if(bluePress) {
    beepBoop(false);
    if(status == 0) status = choice;
    else if(status == 1) handleFeeding();
    bluePress = false;
  }

  if(whitePress) {
    beepBoop(false);
    status = 0;
    whitePress = false;
  }

  backgroundColor = (analogRead(PHOTO_RESISTOR) > 1500) ? ST77XX_WHITE : ST77XX_BLACK;
  textColor = (backgroundColor == ST77XX_WHITE) ? ST77XX_BLACK : ST77XX_WHITE;

  if(status == 0) drawIcon(choice);
  else if(status == 6) {
    drawStats();
    delay(100);
  }
  else drawIcon(status);

  if(millis() - lastDecay > decayInterval) {
    happiness = max(0, happiness - decayAmount);
    hunger = max(0, hunger - decayAmount);
    lastDecay = millis();
  }

  if(millis() - lastWeatherFetch > weatherInterval) {
    float temp = fetchTemperature();
    if(temp < 10) cold = min(100, cold + 5);
    else if(temp > 20) cold = max(0, cold - 5);
    lastWeatherFetch = millis();
  }
}
