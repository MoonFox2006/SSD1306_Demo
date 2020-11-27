#include <Arduino.h>
#include <Wire.h>
#include "SSD1306Console.h"

static void halt(const __FlashStringHelper *msg) {
  Serial.println(msg);
  Serial.flush();
#if defined(ESP32)
  esp_deep_sleep_start();
#elif defined(ESP8266)
  ESP.deepSleep(0);
#else
  for(;;);
#endif
}

#if defined(ESP32) // Heltec Wireless Stick, built-in display 64x32
SSD1306Console<SSD1306<SSD1306_64x32, 0, 16, Wire1>> console;
#elif defined(ESP8266) // Wemos D1 mini, 128x32 display
SSD1306Console<SSD1306<SSD1306_128x32>> console;
#else // UNO, 128x64 @ 0x3C display
SSD1306Console<SSD1306<SSD1306_128x64, 0x3C>> console;
#endif

void setup() {
  Serial.begin(115200);
  Serial.println();

#ifdef ESP32
  if (! console.init(4, 15)) // Heltec Wireless Stick only!
#else
  if (! console.init())
#endif
    halt(F("I2C init fail!"));
  if (! console.begin())
    halt(F("OledConsole fail!"));
//  console.flip(true);

  for (uint8_t c = ' '; c < 127; ++c) {
    console.print((char)c);
    if (console.col() >= console.columns())
      delay(1000);
  }
  console.println();
  delay(1000);
  for (uint16_t c = 0x0410; c < 0x0450; ++c) {
    char str[3];

    str[0] = 0B11000000 | (c >> 6);
    str[1] = 0B10000000 | (c & 0B00111111);
    str[2] = '\0';
    console.print(str);
    if (console.col() >= console.columns())
      delay(1000);
  }
  delay(1000);
  console.print(F("\fСвязь с WiFi "));
  {
    static const char ROTATOR[] PROGMEM = { '-', '\\', '|', '/' };

    uint8_t rotator = 0;

    for (uint8_t i = 0; i < 10; ++i) {
      console.print((char)pgm_read_byte(&ROTATOR[rotator]));
      console.print('\b');
      if (++rotator >= sizeof(ROTATOR))
        rotator = 0;
      delay(250);
    }
  }
  console.println(F("\aOK\a"));
  {
    static const uint8_t CIRCLE_XBM[] PROGMEM = {
      0xC0, 0x03, 0x38, 0x1C, 0x04, 0x20, 0x02, 0x50,
      0x02, 0x48, 0x02, 0x44, 0x01, 0x82, 0x01, 0x81,
      0x81, 0x80, 0x41, 0x80, 0x22, 0x40, 0x12, 0x40,
      0x0A, 0x40, 0x04, 0x20, 0x38, 0x1C, 0xC0, 0x03
    };

    console.drawXBM_P(console.width() - 16, 0, 16, 16, CIRCLE_XBM);
  }
  console.negative(true);
  console.printStr_P((console.width() - console.charWidth() * 8) / 2, 16, PSTR(" *ДЕМО* "));
  console.negative(false);
  delay(1000);
  console.clear();
  for (uint8_t i = 0; i < 10; ++i) {
    console.print(F("Строка\t#"));
    console.println(i + 1);
    delay(500);
  }
  for (int8_t i = 5; i > 0; --i) {
    console.print(F("\rДо сна осталось "));
    console.print(i);
    delay(1000);
  }
  console.println(F("\rЛожимся в сон... "));
  for (uint8_t i = 0; i < 5; ++i) {
    console.invert((i & 0x01) == 0);
    delay(250);
  }
  console.invert(false);
  for (uint8_t c = 127; c > 0; c >>= 1) {
    console.contrast(c);
    delay(250);
  }
  console.power(false);
#if defined(ESP32)
  esp_deep_sleep_start();
#elif defined(ESP8266)
  ESP.deepSleep(0);
#else
  for(;;);
#endif
}

void loop() {}
