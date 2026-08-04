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

// ---------------- Soil moisture ----------------
#define SOIL_PIN 3
// TODO: replace after calibration (dry = high, wet = low)
const int SOIL_DRY_MV = 2600;
const int SOIL_WET_MV = 1200;

// ---------------- Timing ----------------
const unsigned long TEMP_CONVERSION_MS = 750;
const unsigned long SAMPLE_INTERVAL_MS = 2000;

unsigned long lastSampleStart = 0;
unsigned long tempRequestedAt  = 0;
bool tempPending = false;

// ---------------- Readings ----------------
float temperature = 0;
bool  tempValid   = false;
int   soilRawMv   = 0;
int   soilPercent = 0;

int readSoilMv() {
  long sum = 0;
  for (int i = 0; i < 16; i++) {       // average to suppress ADC noise
    sum += analogReadMilliVolts(SOIL_PIN);
    delayMicroseconds(200);
  }
  return sum / 16;
}

int soilToPercent(int mv) {
  long pct = map(mv, SOIL_DRY_MV, SOIL_WET_MV, 0, 100);
  return constrain(pct, 0, 100);
}

void showStatus(const char *line2) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("RootWatch");
  display.setCursor(0, 12);
  display.println(line2);
  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (1) delay(100);
  }
  display.setTextColor(WHITE);
  display.setTextSize(1);
  showStatus("Initializing...");
  delay(1500);

  analogSetPinAttenuation(SOIL_PIN, ADC_11db);   // full ~0-3.1V range

  uint8_t devices = ds.search(addr, MAX_DEVS);
  Serial.printf("Devices Found: %u\n", devices);
  if (devices == 0) {
    Serial.println("No DS18B20 detected! Check wiring/pull-up resistor.");
    showStatus("No temp sensor!");
    while (1) delay(100);
  }
}

void loop() {
  unsigned long now = millis();

  // --- Start a sampling cycle ---
  if (!tempPending && now - lastSampleStart >= SAMPLE_INTERVAL_MS) {
    lastSampleStart = now;
    ds.request();
    tempRequestedAt = now;
    tempPending = true;

    soilRawMv   = readSoilMv();
    soilPercent = soilToPercent(soilRawMv);
  }

  // --- Collect the temperature once conversion has had time ---
  if (tempPending && now - tempRequestedAt >= TEMP_CONVERSION_MS) {
    tempPending = false;
    tempValid = (ds.getTemp(addr[0], temperature) == 0);

    Serial.printf("Soil raw: %d mV | Soil: %d%% | Temp: ",
                  soilRawMv, soilPercent);
    if (tempValid) Serial.printf("%.1f C\n", temperature);
    else           Serial.println("ERROR");

    display.clearDisplay();
    display.setCursor(0, 0);
    if (tempValid) display.printf("Temp: %.1f C\n", temperature);
    else           display.println("Temp: error");
    display.setCursor(0, 12);
    display.printf("Soil: %d%%", soilPercent);
    display.setCursor(0, 24);
    display.printf("raw %d mV", soilRawMv);
    display.display();
  }

  // loop stays free here for WiFi, buttons, etc.
}