#include <Arduino.h>
#include <SPI.h>

const int TFT_CS_PIN = 15;
const int TS_CS_PIN = 21;
const int TS_PEN_IRQ = 16;
const int SCK_PIN = 18;
const int MISO_PIN = 19;
const int MOSI_PIN = 23;

void setup() {
    Serial.begin(115200);
    pinMode(TS_PEN_IRQ, INPUT_PULLUP);
    pinMode(TFT_CS_PIN, OUTPUT);
    pinMode(TS_CS_PIN, OUTPUT);
    digitalWrite(TFT_CS_PIN, HIGH);
    digitalWrite(TS_CS_PIN, HIGH);
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, -1);
    delay(200);
    Serial.println("SPI probe ready");
}

void loop() {
    int pen = digitalRead(TS_PEN_IRQ);
    Serial.printf("PENIRQ=%d  ", pen);
    digitalWrite(TFT_CS_PIN, HIGH);
    delayMicroseconds(200);
    if (pen == LOW) {
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(TS_CS_PIN, LOW);
        uint8_t cmd = 0x90;
        uint8_t r1 = SPI.transfer(cmd);
        delayMicroseconds(2);
        uint8_t hi = SPI.transfer(0x00);
        uint8_t lo = SPI.transfer(0x00);
        digitalWrite(TS_CS_PIN, HIGH);
        SPI.endTransaction();
        uint16_t raw = (((uint16_t)hi << 8) | lo) >> 3;
        Serial.printf("SPI bytes: cmd_resp=0x%02X hi=0x%02X lo=0x%02X raw=%u\n", r1, hi, lo, raw);
    } else {
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        digitalWrite(TS_CS_PIN, LOW);
        uint8_t probe = SPI.transfer(0x00);
        digitalWrite(TS_CS_PIN, HIGH);
        SPI.endTransaction();
        Serial.printf("idle MISO=0x%02X\n", probe);
    }
    delay(300);
}

