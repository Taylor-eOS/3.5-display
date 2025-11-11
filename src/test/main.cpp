#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();
SPIClass TouchSPI(HSPI);

const int TFT_CS_PIN = 15;
const int TFT_DC_PIN = 2;
const int TFT_RST_PIN = 4;
const int TS_CS_PIN = 21;
const int TS_PEN_IRQ = 16;

#define TS_MINX 200
#define TS_MAXX 3900
#define TS_MINY 200
#define TS_MAXY 3900

bool bgWhite = true;
int lastPen = HIGH;
int markerX = -1;
int markerY = -1;

uint16_t touchRead12bit(uint8_t cmd) {
    TouchSPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(TS_CS_PIN, LOW);
    TouchSPI.transfer(cmd);
    delayMicroseconds(2);
    uint8_t hi = TouchSPI.transfer(0x00);
    uint8_t lo = TouchSPI.transfer(0x00);
    digitalWrite(TS_CS_PIN, HIGH);
    TouchSPI.endTransaction();
    return ((((uint16_t)hi << 8) | lo) >> 3) & 0x0FFF;
}

struct RawPoint {
    uint16_t x;
    uint16_t y;
    uint16_t z1;
    uint16_t z2;
};

RawPoint readRaw() {
    RawPoint p;
    p.x = touchRead12bit(0x90);
    p.y = touchRead12bit(0xD0);
    p.z1 = touchRead12bit(0xB0);
    p.z2 = touchRead12bit(0xC0);
    return p;
}

void drawBackground() {
    if (bgWhite)
        tft.fillScreen(TFT_WHITE);
    else
        tft.fillScreen(TFT_LIGHTGREY);
    tft.setTextColor(TFT_BLACK, bgWhite ? TFT_WHITE : TFT_LIGHTGREY);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("SCREEN TEST", tft.width() / 2, 8, 4);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_BLACK, bgWhite ? TFT_WHITE : TFT_LIGHTGREY);
    tft.drawString("Touch to toggle background. Valid touches show marker and coords.", 8, 30, 2);
}

void drawMarker(int x, int y) {
    if (x < 0 || y < 0) return;
    tft.fillCircle(x, y, 6, TFT_RED);
    tft.drawLine(x - 12, y, x + 12, y, TFT_BLACK);
    tft.drawLine(x, y - 12, x, y + 12, TFT_BLACK);
    tft.setTextColor(TFT_BLACK, bgWhite ? TFT_WHITE : TFT_LIGHTGREY);
    tft.setTextDatum(TL_DATUM);
    char buf[32];
    sprintf(buf, "X:%d Y:%d", x, y);
    tft.drawString(buf, 8, tft.height() - 24, 2);
}

void setup() {
    Serial.begin(115200);
    pinMode(TS_PEN_IRQ, INPUT_PULLUP);
    pinMode(TFT_CS_PIN, OUTPUT);
    pinMode(TS_CS_PIN, OUTPUT);
    pinMode(TFT_RST_PIN, OUTPUT);
    pinMode(TFT_DC_PIN, OUTPUT);
    digitalWrite(TFT_CS_PIN, HIGH);
    digitalWrite(TS_CS_PIN, HIGH);
    digitalWrite(TFT_RST_PIN, HIGH);
    TouchSPI.begin(14, 12, 13, -1);
    SPI.begin(18, 19, 23, -1);
    tft.init();
    tft.setRotation(1);
    drawBackground();
}

void loop() {
    int pen = digitalRead(TS_PEN_IRQ);
    if (pen == LOW && lastPen == HIGH) {
        bgWhite = !bgWhite;
        drawBackground();
        markerX = -1;
        markerY = -1;
    }
    lastPen = pen;
    if (pen == LOW) {
        RawPoint p = readRaw();
        Serial.printf("Raw: x=%u y=%u z1=%u z2=%u\n", p.x, p.y, p.z1, p.z2);
        bool valid = p.x > 50 && p.x < 4096 && p.y > 50 && p.y < 4096;
        if (valid) {
            int x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width() - 1);
            int y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height() - 1);
            x = constrain(x, 0, tft.width() - 1);
            y = tft.height() - 1 - y;  // invert Y
            markerX = x;
            markerY = y;
            drawBackground();
            drawMarker(markerX, markerY);
            Serial.printf("Mapped: x=%d y=%d\n", markerX, markerY);
        } else {
            Serial.println("Touch detected but coordinates invalid");
        }
        delay(80);
    } else {
        delay(30);
    }
}

