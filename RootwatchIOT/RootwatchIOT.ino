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
const int SOIL_DRY_MV = 2600;   // <-- your calibrated values
const int SOIL_WET_MV = 1200;

// ---------------- Light (LDR) ----------------
#define LDR_PIN 1
const int LIGHT_DARK_MV   = 150;    // <-- calibrate: covered
const int LIGHT_BRIGHT_MV = 2900;   // <-- calibrate: bright light

// ---------------- Timing ----------------
const unsigned long TEMP_CONVERSION_MS = 750;
const unsigned long SAMPLE_INTERVAL_MS = 2000;

unsigned long lastSampleStart = 0;
unsigned long tempRequestedAt = 0;
bool tempPending = false;

// ---------------- Readings ----------------
float temperature = 0;
bool  tempValid   = false;
int   soilRawMv = 0, soilPercent = 0;
int   lightRawMv = 0, lightPercent = 0;

int readAvgMv(int pin) {
  long sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogReadMilliVolts(pin);
    delayMicroseconds(200);
  }
  return sum / 16;
}

int toPercent(int mv, int atZero, int atHundred) {
  long pct = map(mv, atZero, atHundred, 0, 100);
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

  analogSetPinAttenuation(SOIL_PIN, ADC_11db);
  analogSetPinAttenuation(LDR_PIN,  ADC_11db);

  uint8_t devices = ds.search(addr, MAX_DEVS);
  Serial.printf("Devices Found: %u\n", devices);
  if (devices == 0) {
    Serial.println("No DS18B20 detected!");
    showStatus("No temp sensor!");
    while (1) delay(100);
  }
}

void loop() {
  unsigned long now = millis();

  if (!tempPending && now - lastSampleStart >= SAMPLE_INTERVAL_MS) {
    lastSampleStart = now;
    ds.request();
    tempRequestedAt = now;
    tempPending = true;

    soilRawMv    = readAvgMv(SOIL_PIN);
    soilPercent  = toPercent(soilRawMv, SOIL_DRY_MV, SOIL_WET_MV);

    lightRawMv   = readAvgMv(LDR_PIN);
    lightPercent = toPercent(lightRawMv, LIGHT_DARK_MV, LIGHT_BRIGHT_MV);
  }

  if (tempPending && now - tempRequestedAt >= TEMP_CONVERSION_MS) {
    tempPending = false;
    tempValid = (ds.getTemp(addr[0], temperature) == 0);

    Serial.printf("Temp: %.1f C | Soil: %d%% (%d mV) | Light: %d%% (%d mV)\n",
                  temperature, soilPercent, soilRawMv,
                  lightPercent, lightRawMv);

    display.clearDisplay();
    display.setCursor(0, 0);
    if (tempValid) display.printf("Temp: %.1f C", temperature);
    else           display.print("Temp: error");
    display.setCursor(0, 12);
    display.printf("Soil:  %d%%", soilPercent);
    display.setCursor(0, 24);
    display.printf("Light: %d%%", lightPercent);
    display.display();
  }
}