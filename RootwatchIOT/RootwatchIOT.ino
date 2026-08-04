#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OneWireESP32.h"

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_SDA 6
#define OLED_SCL 7

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- DS18B20 ----------------
#define ONE_WIRE_PIN 2
#define MAX_DEVS 1

OneWire32 ds(ONE_WIRE_PIN);
uint64_t addr[MAX_DEVS];
float temperature = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // OLED init
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("RootWatch");
  display.setCursor(0, 10);
  display.println("Initializing...");
  display.display();
  delay(1500);

  // DS18B20 init
  uint8_t devices = ds.search(addr, MAX_DEVS);
  Serial.print("Devices Found: ");
  Serial.println(devices);

  if (devices == 0) {
    Serial.println("No DS18B20 detected! Check wiring/pull-up resistor.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("No temp sensor!");
    display.display();
    while (1);
  }

  Serial.print("Sensor Address: 0x");
  Serial.println((unsigned long long)addr[0], HEX);
}

void loop() {
  ds.request();
  delay(1000);

  uint8_t err = ds.getTemp(addr[0], temperature);

  Serial.print("Error code: ");
  Serial.println(err);
  Serial.print("Temperature: ");
  Serial.println(temperature);

  delay(1000);
}

  display.display();
  delay(1000);
}