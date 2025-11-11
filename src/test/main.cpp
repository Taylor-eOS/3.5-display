#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(21);

#define TS_MINX 200
#define TS_MAXX 3900
#define TS_MINY 200
#define TS_MAXY 3900
#define TFT_CS_PIN 15

int currentColor = TFT_WHITE;
int brushSize = 3;

struct Button {
  int x, y, w, h;
  uint16_t color;
  const char* label;
};

Button buttons[] = {
  {10, 10, 60, 40, TFT_RED, "Red"},
  {80, 10, 60, 40, TFT_GREEN, "Green"},
  {150, 10, 60, 40, TFT_BLUE, "Blue"},
  {220, 10, 60, 40, TFT_YELLOW, "Yellow"},
  {290, 10, 60, 40, TFT_MAGENTA, "Mag"},
  {360, 10, 60, 40, TFT_CYAN, "Cyan"},
  {430, 10, 40, 40, TFT_WHITE, "Clr"}
};

void drawUI() {
  tft.fillScreen(TFT_BLACK);
  for (int i = 0; i < 7; i++) {
    tft.fillRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, buttons[i].color);
    tft.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, TFT_WHITE);
    tft.setTextColor(i < 6 ? TFT_BLACK : TFT_BLACK, buttons[i].color);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(buttons[i].label, buttons[i].x + buttons[i].w/2, buttons[i].y + buttons[i].h/2, 2);
  }
  tft.drawRect(buttons[6].x, buttons[6].y, buttons[6].w, buttons[6].h, currentColor);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Touch to draw", 240, 70, 2);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(16, INPUT_PULLUP);
  pinMode(TFT_CS_PIN, OUTPUT);
  digitalWrite(TFT_CS_PIN, HIGH);
  SPI.begin(18, 19, 23, -1);
  tft.init();
  tft.setRotation(1);
  ts.begin();
  ts.setRotation(1);
  drawUI();
  Serial.println("Touch drawing ready");
}

void loop() {
  int irq = digitalRead(16);
  if (irq == LOW) {
    digitalWrite(TFT_CS_PIN, HIGH);
    delay(1);
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      Serial.printf("Raw: x=%d y=%d z=%d\n", p.x, p.y, p.z);
      if (p.z > 200 && p.z < 4000 && p.x > 100 && p.y > 100) {
        int x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width() - 1);
        int y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height() - 1);
        x = constrain(x, 0, tft.width() - 1);
        y = constrain(y, 0, tft.height() - 1);
        Serial.printf("Mapped: x=%d y=%d\n", x, y);
        bool buttonPressed = false;
        for (int i = 0; i < 7; i++) {
          if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].w &&
              y >= buttons[i].y && y <= buttons[i].y + buttons[i].h) {
            buttonPressed = true;
            if (i < 6) {
              currentColor = buttons[i].color;
              Serial.printf("Color changed to %s\n", buttons[i].label);
            } else {
              drawUI();
              Serial.println("Screen cleared");
            }
            delay(200);
            break;
          }
        }
        if (!buttonPressed && y > 90) {
          tft.fillCircle(x, y, brushSize, currentColor);
          Serial.printf("Drew at x=%d y=%d\n", x, y);
        }
      } else {
        Serial.println("Touch rejected (invalid z or x/y)");
      }
    }
    delay(10);
  }
}
