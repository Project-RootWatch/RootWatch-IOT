# RootWatch — IoT Firmware

ESP32-C6 firmware for the RootWatch smart soil, irrigation, and plant
health monitoring system.

## Role in the system

Runs on the ESP32-C6 in the field. Reads soil moisture, temperature,
light level, and leaf color sensors on a regular interval, shows current
status on an onboard OLED, and sends readings to **RootWatch-BackEnd**
over WiFi (HTTP POST). Also listens for irrigation trigger commands from
the backend and drives the relay/solenoid valve, with a safety auto-close
timer so a lost connection or bug can't leave the water running.

## Status

Placeholder — firmware code lands in the next build step.

## Hardware

- ESP32-C6 dev board
- Waterproof DS18B20-style temperature sensor
- I2C OLED display (SSD1306-style)
- Capacitive soil moisture sensor
- LDR light sensor (10kΩ resistor voltage divider)
- TCS3200 color sensor (leaf color tracking)
- 5V relay module + 12V solenoid valve (1/2" NC), powered by a separate
  12V supply — not the ESP32's own power rail

## Tech stack

- Arduino framework (C++) targeting ESP32-C6
