#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(21, 16);

#define TS_MINX 200
#define TS_MAXX 3900
#define TS_MINY 200
#define TS_MAXY 3900

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23);
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  tft.init();
  tft.setRotation(1);
  ts.begin();
  ts.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  int w = tft.width();
  int h = tft.height();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ILI9488 SPI test", w/2, 20, 4);
  int barH = (h - 60) / 6;
  tft.fillRect(0, 40 + 0*barH, w, barH, TFT_RED);
  tft.fillRect(0, 40 + 1*barH, w, barH, TFT_GREEN);
  tft.fillRect(0, 40 + 2*barH, w, barH, TFT_BLUE);
  tft.fillRect(0, 40 + 3*barH, w, barH, TFT_YELLOW);
  tft.fillRect(0, 40 + 4*barH, w, barH, TFT_MAGENTA);
  tft.fillRect(0, 40 + 5*barH, w, barH, TFT_CYAN);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Touch to test", w/2, h - 20, 2);
}

void loop() {
  static uint32_t lastPrint = 0;
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    digitalWrite(21, HIGH);
    int x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width() - 1);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height() - 1);
    if (x < 0) x = 0; if (x >= tft.width()) x = tft.width() - 1;
    if (y < 0) y = 0; if (y >= tft.height()) y = tft.height() - 1;
    tft.fillCircle(x, y, 6, TFT_WHITE);
    tft.drawCircle(x, y, 8, TFT_BLACK);
    if (millis() - lastPrint > 100) {
      Serial.printf("raw x=%d y=%d z=%d -> x=%d y=%d\n", p.x, p.y, p.z, x, y);
      lastPrint = millis();
    }
  } else {
    static uint32_t idleClear = 0;
    if (millis() - idleClear > 1000) {
      idleClear = millis();
      tft.fillRect(0, 0, 120, 20, TFT_BLACK);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Ready", 60, 10, 2);
    }
  }
}

