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

uint16_t avgBuffer[3][2];
int avgIndex = 0;
int touchSamples = 0;

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
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("SCREEN TEST", tft.width() / 2, 8, 4);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("Draw with stylus to test. Lines persist until reset.", 8, 30, 2);
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
    bool newTouch = (pen == LOW && lastPen == HIGH);
    lastPen = pen;
    if (pen == LOW) {
        if (newTouch) {
            avgBuffer[0][0] = 0;
            avgBuffer[0][1] = 0;
            avgBuffer[1][0] = 0;
            avgBuffer[1][1] = 0;
            avgBuffer[2][0] = 0;
            avgBuffer[2][1] = 0;
            avgIndex = 0;
            touchSamples = 0;
        }
        RawPoint raw = readRaw();
        Serial.printf("Raw: x=%u y=%u z1=%u z2=%u\n", raw.x, raw.y, raw.z1, raw.z2);
        avgBuffer[avgIndex][0] = raw.x;
        avgBuffer[avgIndex][1] = raw.y;
        avgIndex = (avgIndex + 1) % 3;
        uint32_t sumX = 0, sumY = 0;
        for (int i = 0; i < 3; i++) {
            sumX += avgBuffer[i][0];
            sumY += avgBuffer[i][1];
        }
        uint16_t avgX = sumX / 3;
        uint16_t avgY = sumY / 3;
        touchSamples++;
        bool valid = avgX > 100 && avgX < 3950 && avgY > 100 && avgY < 3950 && raw.z1 > 100;
        if (valid && touchSamples >= 3) {
            int x = map(avgX, TS_MINX, TS_MAXX, 0, tft.width() - 1);
            int y = map(avgY, TS_MINY, TS_MAXY, 0, tft.height() - 1);
            x = constrain(x, 0, tft.width() - 1);
            y = tft.height() - 1 - y;
            if (markerX >= 0 && markerY >= 0 && (abs(x - markerX) > 1 || abs(y - markerY) > 1)) {
                tft.drawLine(markerX, markerY, x, y, TFT_BLACK);
            }
            markerX = x;
            markerY = y;
            tft.fillCircle(x, y, 3, TFT_RED);
            static unsigned long lastPrint = 0;
            if (millis() - lastPrint > 100) {
                Serial.printf("Mapped: x=%d y=%d\n", x, y);
                lastPrint = millis();
            }
        } else if (!valid) {
            Serial.println("Touch detected but invalid (noise/low pressure)");
        }
        delay(15);
    } else {
        touchSamples = 0;
        markerX = -1;
        markerY = -1;
        delay(20);
    }
}
